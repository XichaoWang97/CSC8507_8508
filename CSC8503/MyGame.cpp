#include "MyGame.h"

#include "GameWorld.h"
#include "PhysicsSystem.h"
#include "PhysicsObject.h"
#include "RenderObject.h"

#include "Window.h"
#include "Debug.h"
#include "KeyboardMouseController.h"

#include "GameTechRendererInterface.h"
#include "Ray.h"

using namespace NCL;
using namespace NCL::Maths;
using namespace NCL::CSC8503;

static Vector4 ColourForPolarity(Polarity p) {
    switch (p) {
    case Polarity::South: return Vector4(1.0f, 0.55f, 0.0f, 1.0f); // orange
    case Polarity::North: return Vector4(0.2f, 0.5f, 1.0f, 1.0f);  // blue
    default:              return Vector4(0.9f, 0.9f, 0.9f, 1.0f);
    }
}

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

    // assets (minimum)
    cubeMesh = renderer.LoadMesh("cube.msh");
    playerMesh = renderer.LoadMesh("ORIGAMI_Chat.msh");

    defaultTex = renderer.LoadTexture("Default.png");
    notexMaterial.type = MaterialType::Opaque;
    notexMaterial.diffuseTex = defaultTex;

    InitCamera();
    InitWorld();
}

MyGame::~MyGame() {
    delete cubeMesh;
    delete playerMesh;
    delete defaultTex;
    delete controller;
}

void MyGame::UpdateGame(float dt) {
    // Update camera controller (free look), then we apply follow camera
    world.GetMainCamera().UpdateCamera(dt);

    if (!player) return;

    // Keep your 3rd person camera-follow logic
    SetCameraToPlayer(player);

    // Update player movement & polarity pulse input
    player->Update(dt);

    // Apply magnet interaction (only when pulse active)
    ApplyMagnetPulse(player, dt);

    // debug hint
    Debug::Print("LMB: South (Orange) | RMB: North (Blue)", Vector2(5, 5), Vector4(1, 1, 1, 1));

    // update physics/world (kept same pattern as your original MyGame)
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

    // Floor
    AddFloorToWorld(Vector3(0, -1, 0));

    // Player
    player = AddPlayerToWorld(Vector3(0, 3, 0), 2.0f, 0.5f);

    // Example: place multiple magnetic cubes
    {
        auto* c1 = AddMagneticCubeToWorld(Vector3(0, 2, -12), Vector3(1, 1, 1), 1.0f, Polarity::North);
        magneticCubes.push_back({ c1, Polarity::North });

        auto* c2 = AddMagneticCubeToWorld(Vector3(6, 2, -10), Vector3(1, 1, 1), 1.0f, Polarity::South);
        magneticCubes.push_back({ c2, Polarity::South });

        auto* c3 = AddMagneticCubeToWorld(Vector3(-6, 2, -10), Vector3(1, 1, 1), 0.0f, Polarity::North);
        magneticCubes.push_back({ c3, Polarity::North });

		auto* c4 = AddOBBCubeToWorld(Vector3(10, 2, -6), Vector3(1, 1, 1), 1.0f, Polarity::South);
    }
}

GameObject* MyGame::AddFloorToWorld(const Vector3& position) {
    GameObject* floor = new GameObject("Floor");

    Vector3 floorHalfSize = Vector3(200, 1, 200);
    AABBVolume* volume = new AABBVolume(floorHalfSize);
    floor->SetBoundingVolume(volume);

    floor->GetTransform()
        .SetScale(floorHalfSize * 2.0f)
        .SetPosition(position);

    floor->SetRenderObject(new RenderObject(floor->GetTransform(), cubeMesh, notexMaterial));
    floor->GetRenderObject()->SetColour(Vector4(0.25f, 0.25f, 0.25f, 1.0f));

    floor->SetPhysicsObject(new PhysicsObject(floor->GetTransform(), floor->GetBoundingVolume()));
    floor->GetPhysicsObject()->SetInverseMass(0.0f);
    floor->GetPhysicsObject()->InitCubeInertia();

    world.AddGameObject(floor);
    return floor;
}

Player* MyGame::AddPlayerToWorld(const Vector3& position, float radius, float inverseMass) {

    Player* p = new Player(&world);

    SphereVolume* volume = new SphereVolume(radius * 0.5f);
    p->SetBoundingVolume(volume);

    p->GetTransform()
        .SetScale(Vector3(radius, radius, radius))
        .SetPosition(position);

    p->SetRenderObject(new RenderObject(p->GetTransform(), playerMesh, notexMaterial));
    p->GetRenderObject()->SetColour(Vector4(0.9f, 0.9f, 0.9f, 1.0f));

    PhysicsObject* po = new PhysicsObject(p->GetTransform(), p->GetBoundingVolume());
    po->SetInverseMass(inverseMass);
    po->InitSphereInertia();
    po->SetElasticity(0.0f);

    p->SetPhysicsObject(po);

    world.AddGameObject(p);
    return p;
}

GameObject* MyGame::AddMagneticCubeToWorld(const Vector3& position,
    const Vector3& halfDims,
    float inverseMass,
    Polarity polarity) {
    GameObject* cube = new GameObject("MagneticCube");

    AABBVolume* volume = new AABBVolume(halfDims);
    cube->SetBoundingVolume(volume);

    cube->GetTransform()
        .SetPosition(position)
        .SetScale(halfDims * 2.0f);

    cube->SetRenderObject(new RenderObject(cube->GetTransform(), cubeMesh, notexMaterial));
    cube->GetRenderObject()->SetColour(ColourForPolarity(polarity));

    cube->SetPhysicsObject(new PhysicsObject(cube->GetTransform(), cube->GetBoundingVolume()));
    cube->GetPhysicsObject()->SetInverseMass(inverseMass);
    cube->GetPhysicsObject()->InitCubeInertia();

    world.AddGameObject(cube);
    return cube;
}

// OBB  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
GameObject* MyGame::AddOBBCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass, Polarity polarity) {

    GameObject* cube = new GameObject("OBBCube");

    OBBVolume* volume = new OBBVolume(dimensions);
    cube->SetBoundingVolume(volume);

    cube->GetTransform()
        .SetPosition(position)
        .SetScale(dimensions * 2.0f);

    cube->SetRenderObject(new RenderObject(cube->GetTransform(), cubeMesh, notexMaterial));
    cube->GetRenderObject()->SetColour(ColourForPolarity(polarity));

    cube->SetPhysicsObject(new PhysicsObject(cube->GetTransform(), cube->GetBoundingVolume()));

    cube->GetPhysicsObject()->SetInverseMass(inverseMass);
    cube->GetPhysicsObject()->InitCubeInertia();
    cube->SetInitPosition(position);

    world.AddGameObject(cube);

    return cube;
}

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

void MyGame::ApplyMagnetPulse(Player* p, float dt) {
    if (!p->IsPolarityPulseActive()) {
        return;
    }

    Polarity pulse = p->GetPolarityPulse();
    if (pulse == Polarity::None) return;

    if (magneticCubes.empty()) return;

    Vector3 origin = p->GetMagnetOrigin();
    Vector3 aimFwd = Vector::Normalise(p->GetAimForward());

    // Pick best target: nearest cube within range + in-front cone
    GameObject* bestObj = nullptr;
    Polarity bestPolarity = Polarity::None;
    float bestDist = 1e9f;

    for (const auto& e : magneticCubes) {
        if (!e.obj) continue;

        Vector3 cubePos = e.obj->GetTransform().GetPosition();
        Vector3 toCube = cubePos - origin;
        float dist = Vector::Length(toCube);
        if (dist <= 0.0001f) continue;

        if (dist > magnetRange) continue;

        Vector3 dirToCube = toCube / dist;
        float facing = Vector::Dot(dirToCube, aimFwd);
        if (facing < magnetConeDot) continue;

        if (dist < bestDist) {
            bestDist = dist;
            bestObj = e.obj;
            bestPolarity = e.polarity;
        }
    }

    if (!bestObj) return;

    Vector3 cubePos = bestObj->GetTransform().GetPosition();
    Vector3 toCube = cubePos - origin;
    float dist = Vector::Length(toCube);
    if (dist <= 0.0001f) return;

    Vector3 dirToCube = toCube / dist;

    // Debug line (colour matches the player's pulse)
    Debug::DrawLine(origin, cubePos, ColourForPolarity(pulse));

    PhysicsObject* cubePhys = bestObj->GetPhysicsObject();
    PhysicsObject* playerPhys = p->GetPhysicsObject();
    if (!cubePhys || !playerPhys) return;

    // Determine attract/repel: opposite attracts, same repels
    const bool samePolarity = (pulse == bestPolarity);

    // Force that would be applied to the cube (repel: away from player, attract: toward player)
    Vector3 forceDirOnCube = samePolarity ? dirToCube : -dirToCube;
    Vector3 F = forceDirOnCube * magnetForce;

    // Special case: inverseMass == 0 (static / immovable object)
    const float invMass = cubePhys->GetInverseMass();
    const bool immovable = (invMass == 0.0f);

    if (!samePolarity) {
        // ===== Attract (opposite poles) =====
        if (immovable) {
            // Can't pull the cube -> pull the player toward the cube (grapple-like)
            playerPhys->AddForce(dirToCube * magnetForce);
        }
        else {
            // Normal: pull cube to player, plus optional recoil
            cubePhys->AddForce(F);
            playerPhys->AddForce(-F * selfRecoilFactor);
        }
    }
    else {
        // ===== Repel (same poles) =====
        if (immovable) {
            // Optional: being "pushed" off static objects feels good; remove if you dislike it
            playerPhys->AddForce(-dirToCube * magnetForce);
        }
        else {
            cubePhys->AddForce(F);
            playerPhys->AddForce(-F * selfRecoilFactor);
        }
    }
}
