#include "MyGame.h"
#include "MetalObject.h"
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

    // assets
    cubeMesh = renderer.LoadMesh("cube.msh");
    playerMesh = renderer.LoadMesh("cat.msh");
    defaultTex = renderer.LoadTexture("checkerboard.png");
    notexMaterial = GameTechMaterial();

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

    // Keep your 3rd person camera-follow logic
    SetCameraToPlayer(player);

    // Update player movement & pull/push input
    player->Update(dt);

    // Apply pull/push interaction (hold LMB/RMB)
    ApplyPullPush(player);

    Debug::Print("LMB: Pull | RMB: Push", Vector2(5, 5), Vector4(1, 1, 1, 1));

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
    metalObjects.clear();

    // Floor
    AddFloorToWorld(Vector3(0, -1, 0));

    // Player
    player = AddPlayerToWorld(Vector3(0, 3, 0), 2.0f, 0.5f);
    // Let Player do pre-lock selection on our metal list
    player->SetMetalObjects(&metalObjects);

    // Example: place multiple metal objects
    // inverseMass == 0 => immovable "anchor" you can pull yourself to
    AddMetalCubeToWorld(Vector3(0, 2, -12), Vector3(1, 1, 1), 1.0f);
    AddMetalCubeToWorld(Vector3(6, 2, -10), Vector3(1, 1, 1), 1.0f);
    AddMetalCubeToWorld(Vector3(-6, 12, -10), Vector3(1, 1, 1), 0.0f);

    // Keep one OBB metal cube for testing (if your OBB is stable enough)
    AddMetalOBBCubeToWorld(Vector3(10, 2, -6), Vector3(1, 1, 1), 1.0f);
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
    p->GetRenderObject()->SetColour(Vector4(0,1,1,1));

    PhysicsObject* po = new PhysicsObject(p->GetTransform(), p->GetBoundingVolume());
    po->SetInverseMass(inverseMass);
    po->InitSphereInertia();
    po->SetElasticity(0.0f); // no bounciness

    p->SetPhysicsObject(po);

    world.AddGameObject(p);
    return p;
}

MetalObject* MyGame::AddMetalCubeToWorld(const Vector3& position,
    const Vector3& halfDims,
    float inverseMass) {

    MetalObject* cube = new MetalObject("MetalCube");

    AABBVolume* volume = new AABBVolume(halfDims);
    cube->SetBoundingVolume(volume);

    cube->GetTransform()
        .SetPosition(position)
        .SetScale(halfDims * 2.0f);

    cube->SetRenderObject(new RenderObject(cube->GetTransform(), cubeMesh, notexMaterial));
    cube->GetRenderObject()->SetColour(Vector4(0.65f, 0.65f, 0.7f, 1.0f)); // "metal-ish"

    cube->SetPhysicsObject(new PhysicsObject(cube->GetTransform(), cube->GetBoundingVolume()));
    cube->GetPhysicsObject()->SetInverseMass(inverseMass);
    cube->GetPhysicsObject()->InitCubeInertia();

    world.AddGameObject(cube);
    metalObjects.push_back(cube);
    return cube;
}

MetalObject* MyGame::AddMetalOBBCubeToWorld(const Vector3& position,
    Vector3 dimensions,
    float inverseMass) {

    MetalObject* cube = new MetalObject("MetalOBB");

    OBBVolume* volume = new OBBVolume(dimensions);
    cube->SetBoundingVolume(volume);

    cube->GetTransform()
        .SetPosition(position)
        .SetScale(dimensions * 2.0f);

    cube->SetRenderObject(new RenderObject(cube->GetTransform(), cubeMesh, notexMaterial));
    cube->GetRenderObject()->SetColour(Vector4(0.65f, 0.65f, 0.7f, 1.0f));

    cube->SetPhysicsObject(new PhysicsObject(cube->GetTransform(), cube->GetBoundingVolume()));
    cube->GetPhysicsObject()->SetInverseMass(inverseMass);
    cube->GetPhysicsObject()->InitCubeInertia();

    world.AddGameObject(cube);
    metalObjects.push_back(cube);
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
    MetalObject* bestObj = p->GetActiveTarget();
    //if (!bestObj) return; //XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX射出直线/屏幕中心点
    PhysicsObject* playerPhys = p->GetPhysicsObject();
    PhysicsObject* objPhys;
	Vector3 objPos;

    if (bestObj) {
        objPhys = bestObj->GetPhysicsObject();
        if (!objPhys) return;
        objPos = bestObj->GetTransform().GetPosition();
    }
    else {
        // 没有 bestObj：使用屏幕中心射线命中的交点作为锚点
        // 只在“还没有锚点”时做一次 Raycast，把点锁住
        if (!p->HasAnchor()) {
            // 这里用“屏幕中心点”射线：一般是 Camera position + forward
            // 你需要用你项目里的 Camera 接口替换下面两行
            Vector3 Pos = player->GetTransform().GetPosition();
            Vector3 camDir = camera->GetForwardVector(); // 归一化方向
            Ray ray(Pos, camDir);

            RayCollision hit;
            const float maxRayDist = interactRange; // 或者更长，比如 50.0f

            // 这里用你框架里的 Raycast：名字可能是 world->Raycast 或 gameWorld->Raycast
            // 你需要把参数对上你工程的实际函数签名
            if (gameWorld->Raycast(ray, hit, true, this)) {
                // 你需要在 RayCollision 里取到命中距离/点
                // 常见是 hit.collidedAt / hit.point / hit.rayIntersection
                Vector3 hitPos = hit.collidedAt;

                // 如果你只允许“攀爬墙”，那就做一层过滤：
                // 例如：命中对象带 tag "ClimbWall" 或者是某种 layer/type
                GameObject* hitObj = (GameObject*)hit.node;
                if (hitObj && hitObj->GetName() == "ClimbWall") { // 示例：按你的工程改
                    // 锚点锁定
                    // p->SetAnchor(hitPos, hitObj);
                    // 如果你没写 SetAnchor，就直接改成员（写成 public setter 更干净）
                    // 这里假设你写了一个 SetAnchor:
                }
                else {
                    // 不符合攀爬条件就别锁
                    return;
                }
            }
            else {
                // 射线没打到任何东西
                return;
            }
        }

        objPos = p->GetAnchorPos();

        // anchor 对墙施力没意义（墙一般 invMass=0），所以 objPhys 为空就行
        objPhys = nullptr;
    }

    //-----------------------------------------------------
    //objPos = bestObj->GetTransform().GetPosition();
    Vector3 toObj = objPos - origin;
    float dist = Vector::Length(toObj);
    if (dist <= 0.0001f) return;
    if (dist > interactRange) return;
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
