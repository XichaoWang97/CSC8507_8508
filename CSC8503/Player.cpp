#include "Player.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "Ray.h"
#include "Debug.h"

using namespace NCL;
using namespace NCL::Maths;
using namespace NCL::CSC8503;

Player::Player(GameWorld* world)
    : GameObject("Player")
    , gameWorld(world) {
}

Player::~Player() = default;

void Player::Update(float dt) {
    if (!gameWorld) return;

    // read input (local) unless we are in networked mode
    if (!ignoreInput) {
        ReadLocalInput();
    }

    // select lock-on candidates (optional debug / future feature)
    if (lockModeHeld) {
        SelectCandicatesForLockOn();
    }

    PlayerControl(dt);
}

void Player::ReadLocalInput() {
    currentInputs.axis = Vector3(0, 0, 0);
    currentInputs.jump = false;
    currentInputs.pullHeld = false;
    currentInputs.pushHeld = false;

    currentInputs.cameraYaw = gameWorld->GetMainCamera().GetYaw();

    if (Window::GetKeyboard()->KeyDown(KeyCodes::W)) currentInputs.axis.z -= 1;
    if (Window::GetKeyboard()->KeyDown(KeyCodes::S)) currentInputs.axis.z += 1;
    if (Window::GetKeyboard()->KeyDown(KeyCodes::A)) currentInputs.axis.x -= 1;
    if (Window::GetKeyboard()->KeyDown(KeyCodes::D)) currentInputs.axis.x += 1;

    if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {
        currentInputs.jump = true;
    }

    // LMB = Pull, RMB = Push
    if (Window::GetMouse()->ButtonDown(MouseButtons::Left)) {
        currentInputs.pullHeld = true;
    }
    if (Window::GetMouse()->ButtonDown(MouseButtons::Right)) {
        currentInputs.pushHeld = true;
    }

    // Optional: hold ALT to enter "selection mode" (stub for now)
    if (Window::GetKeyboard()->KeyDown(KeyCodes::LMENU)) {
        lockModeHeld = true;
    }
    else {
        lockModeHeld = false;
    }
}

void Player::PlayerControl(float dt) {
    PhysicsObject* phys = GetPhysicsObject();
    if (!phys) return;

    Transform& transform = GetTransform();

    // camera yaw rotation only (third-person style)
    Quaternion cameraRot = Quaternion::EulerAnglesToQuaternion(0, currentInputs.cameraYaw, 0);

    if (Vector::LengthSquared(currentInputs.axis) > 0.0001f) {
        Vector3 inputDir = Vector::Normalise(currentInputs.axis);
        Vector3 targetDir = cameraRot * inputDir;

        // face movement direction
        Matrix4 lookAtMat = Matrix::View(Vector3(0, 0, 0), -targetDir, Vector3(0, 1, 0));
        Quaternion targetOrientation(Matrix::Inverse(lookAtMat));
        Quaternion currentOrientation = transform.GetOrientation();

        float dot = Quaternion::Dot(currentOrientation, targetOrientation);
        if (dot < 0.0f) targetOrientation = targetOrientation * -1.0f;

        Quaternion newOrientation = Quaternion::Slerp(currentOrientation, targetOrientation, rotationSpeed * dt);
        transform.SetOrientation(newOrientation);

        phys->SetAngularVelocity(Vector3(0, 0, 0));
        phys->AddForce(targetDir * moveForce);
    }

    // limit planar speed
    Vector3 v = phys->GetLinearVelocity();
    Vector3 planar(v.x, 0, v.z);
    if (Vector::Length(planar) > maxSpeed) {
        planar = Vector::Normalise(planar) * maxSpeed;
        phys->SetLinearVelocity(Vector3(planar.x, v.y, planar.z));
    }

    // jump
    if (currentInputs.jump && IsPlayerOnGround()) {
        phys->ApplyLinearImpulse(Vector3(0, jumpImpulse, 0));
        currentInputs.jump = false;
    }
}

bool Player::IsPlayerOnGround() {
    Vector3 pos = GetTransform().GetPosition();
    Ray ray(pos, Vector3(0, -1, 0));
    RayCollision hit;

    const float groundCheckDist = 1.1f;
    if (gameWorld->Raycast(ray, hit, true, this)) {
        return hit.rayDistance < groundCheckDist;
    }
    return false;
}

Vector3 Player::GetMagnetOrigin() {
    return GetTransform().GetPosition() + Vector3(0, 1.2f, 0);
}

Vector3 Player::GetAimForward() {
    // Default: model forward is +Z in this framework
    Quaternion q = GetTransform().GetOrientation();
    return q * Vector3(0, 0, 1);
}

void Player::SelectCandicatesForLockOn() {
    // Stub: keep empty for now (prevents linker error).
    // You can later implement:
    //  - gather visible MetalObjects in front of camera
    //  - sort by screen center distance then by player distance
}
