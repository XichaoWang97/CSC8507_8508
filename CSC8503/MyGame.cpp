#include "MyGame.h"
#include "Level.h"
#include "MetalObject.h"
#include "GameWorld.h"
#include "PhysicsSystem.h"
#include "PhysicsObject.h"
#include "LevelRegistry.h"

#include "Window.h"
#include "Debug.h"
#include "KeyboardMouseController.h"

#include "GameTechRendererInterface.h"
#include "Ray.h"

using namespace NCL;
using namespace NCL::Maths;
using namespace NCL::CSC8503;

MyGame::MyGame(GameWorld& inWorld, GameTechRendererInterface& inRenderer, PhysicsSystem& inPhysics)
    : world(inWorld)
    , renderer(inRenderer)
    , physics(inPhysics) {

    physics.UseGravity(true);

    // controller for camera
    controller = new KeyboardMouseController(*Window::GetWindow()->GetKeyboard(),
        *Window::GetWindow()->GetMouse());
    world.GetMainCamera().SetController(*controller);

    // lighting (keep simple)
    world.SetSunPosition({ -200.0f, 60.0f, -200.0f });
    world.SetSunColour({ 0.8f, 0.8f, 0.5f });

    controller->MapAxis(3, "XLook");
    controller->MapAxis(4, "YLook");

    InitCamera();
    InitWorld();
}

MyGame::~MyGame() {
    delete controller;
    controller = nullptr;
}

void MyGame::UpdateGame(float dt) {
    // Update camera controller (free look), then we apply follow camera
    world.GetMainCamera().UpdateCamera(dt);

    if (!player) return;

    SetCameraToPlayer(player);

    player->Update(dt);

    ApplyPullPush(player);

    Debug::Print("LMB: Pull | RMB: Push | F: lock on object", Vector2(5, 5), Vector4(1, 1, 1, 1));

    // update physics/world
    physics.Update(dt);
    world.UpdateWorld(dt);
}

void MyGame::InitCamera() {
    world.GetMainCamera().SetNearPlane(0.1f);
    world.GetMainCamera().SetFarPlane(500.0f);
    world.GetMainCamera().SetPitch(-15.0f);
    world.GetMainCamera().SetYaw(315.0f);
    world.GetMainCamera().SetPosition(Vector3(-30, 20, 30));
}

void MyGame::InitWorld() {
    world.ClearAndErase();
    physics.Clear();
    metalObjects.clear();
    player = nullptr;

    // Build via Level (so teammates can own Level.cpp/h without touching MyGame)
    currentLevel = LevelRegistry::Get().Create(0);
    if (!currentLevel) {
        return;
    }

    LevelContext ctx;
    ctx.world = &world;
    ctx.physics = &physics;
    ctx.renderer = &renderer;
    ctx.metalObjects = &metalObjects;
    ctx.playerOut = &player;

    currentLevel->SetContext(ctx);
    currentLevel->Build();

    // Let Player do pre-lock selection on our metal list
    if (player) {
        player->SetMetalObjects(&metalObjects);
    }
}

// GameMechanic: 3rd person follow camera logic
void MyGame::SetCameraToPlayer(Player* p) {
    Vector3 playerPos = p->GetTransform().GetPosition();

    float yaw = world.GetMainCamera().GetYaw();
    float pitch = world.GetMainCamera().GetPitch();

    // restrict pitch
    if (pitch > 30.0f)  pitch = 30.0f;
    if (pitch < -60.0f) pitch = -60.0f;

    Quaternion cameraRot = Quaternion::EulerAnglesToQuaternion(pitch, yaw, 0);
    Vector3 cameraBackward = cameraRot * Vector3(0, 0, 1);

    Vector3 offset = Vector3(0, 5, 0);
    float maxDist = 15.0f;
    float currentDist = maxDist;

    Vector3 rayOrigin = playerPos + offset;
    Ray ray(rayOrigin, cameraBackward);
    RayCollision collision;

    if (world.Raycast(ray, collision, true, p)) {
        if (collision.rayDistance < maxDist) {
            currentDist = collision.rayDistance - 0.5f;
            if (currentDist < 0.5f) currentDist = 0.5f;
        }
    }

    Vector3 camPos = rayOrigin + (cameraBackward * currentDist);
    world.GetMainCamera().SetPosition(camPos);
}

// GameMechanic: Pull/push interaction logic
void MyGame::ApplyPullPush(Player* p) {
    if (!p) return;

    const bool pulling = p->IsPullHeld();
    const bool pushing = p->IsPushHeld();

    if (!pulling && !pushing) {
        return;
    }

    if (metalObjects.empty()) return;

    Vector3 origin = p->GetMagnetOrigin();

    // Use player's pre-lock / hard-lock target instead of re-selecting here
    MetalObject* bestObj = p->GetLockedTarget();
    PhysicsObject* playerPhys = p->GetPhysicsObject();
    PhysicsObject* objPhys;
    Vector3 objPos;

    if (bestObj) {
        objPhys = bestObj->GetPhysicsObject();
        if (!objPhys) return;
        objPos = bestObj->GetTransform().GetPosition();
    }
    else {
        // No locked target: cast a ray straight forward from the player
        Vector3 rayOrigin = origin;             // origin = p->GetMagnetOrigin()
        Vector3 rayDir = p->GetAimForward();    // In Player.cpp forward is defined as +Z

        Vector::Normalise(rayDir);

        const Vector4 rayColor = pulling ? Vector4(0.2f, 1.0f, 0.2f, 0.35f)
            : Vector4(1.0f, 0.2f, 0.2f, 0.35f);

        // 1.Draw the full ray each frame (so you can confirm the ray is emitted)
        Debug::DrawLine(rayOrigin, rayOrigin + rayDir * p->GetLockRadius(), rayColor);

        Ray ray(rayOrigin, rayDir);
        RayCollision hit;

        // 2.Perform the actual raycast (otherwise hit is uninitialized)
        if (!world.Raycast(ray, hit, true, p)) {
            return;
        }

        GameObject* hitGO = static_cast<GameObject*>(hit.node);
        if (!hitGO) return;

        // 3.Only allow metallic objects (currently filtered by name)
        if (hitGO->GetName().find("MetalObject") == std::string::npos) return;

        objPhys = hitGO->GetPhysicsObject();
        if (!objPhys) return;

        // 4.Hit point (ray intersection point)
        objPos = rayOrigin + rayDir * hit.rayDistance;

        // 5.Draw a clearer line to the hit point
        const Vector4 hitCol = pulling ? Vector4(0.2f, 1.0f, 0.2f, 1.0f)
            : Vector4(1.0f, 0.2f, 0.2f, 1.0f);
        Debug::DrawLine(rayOrigin, objPos, hitCol);
    }

    //objPos = bestObj->GetTransform().GetPosition();
    Vector3 toObj = objPos - origin;
    float dist = Vector::Length(toObj);
    if (dist <= 0.0001f) return;
    if (dist > p->GetLockRadius()) return;
    Vector3 dirToObj = toObj / dist;

    // Pull => bring together (object towards player, player towards object)
    // Push => separate (object away from player, player away from object)
    Vector3 F = dirToObj * interactForce;
    if (pulling && !pushing) {
        // Object pulled toward player: -F
        objPhys->AddForce(-F);
        playerPhys->AddForce(F);
        Debug::DrawLine(origin, objPos, Vector4(0.2f, 1.0f, 0.2f, 1.0f)); // green
    }
    else if (pushing && !pulling) {
        objPhys->AddForce(F);
        playerPhys->AddForce(-F);
        Debug::DrawLine(origin, objPos, Vector4(1.0f, 0.2f, 0.2f, 1.0f)); // red
    }
    else {
        // Both pressed: do nothing (or choose one). Here: prefer Pull.
        objPhys->AddForce(-F);
        playerPhys->AddForce(F);
        Debug::DrawLine(origin, objPos, Vector4(0.2f, 1.0f, 0.2f, 1.0f));
    }
}