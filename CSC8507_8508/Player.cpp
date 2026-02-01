#include "Player.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "Ray.h"
#include "Debug.h"

using namespace NCL;
using namespace NCL::Maths;
using namespace NCL::CSC8503;

static Vector4 ColourForPolarity(Polarity p) {
    // South = Orange, North = Blue
    switch (p) {
    case Polarity::South: return Vector4(1.0f, 0.55f, 0.0f, 1.0f);
    case Polarity::North: return Vector4(0.2f, 0.5f, 1.0f, 1.0f);
    default:              return Vector4(0.9f, 0.9f, 0.9f, 1.0f);
    }
}

Player::Player(GameWorld* world)
    : GameObject("Player")
    , gameWorld(world) {
}

void Player::Update(float dt) {
    if (!gameWorld) return;

    // read input (local) unless we are in networked mode
    if (!ignoreInput) {
        ReadLocalInput();
    }

    // polarity pulse trigger (momentary)
    if (currentInputs.pulseSouth) TriggerPolarityPulse(Polarity::South);
    if (currentInputs.pulseNorth) TriggerPolarityPulse(Polarity::North);

    // decay pulse timer
    if (polarityPulseTimer > 0.0f) {
        polarityPulseTimer -= dt;
        if (polarityPulseTimer <= 0.0f) {
            polarityPulseTimer = 0.0f;
            polarityPulse = Polarity::None;
        }
    }

    // update colour (pulse colour)
    if (GetRenderObject()) {
        GetRenderObject()->SetColour(ColourForPolarity(GetPolarityPulse()));
    }

    PlayerControl(dt);
}

void Player::ReadLocalInput() {
    currentInputs.axis = Vector3(0, 0, 0);
    currentInputs.jump = false;
    currentInputs.pulseSouth = false;
    currentInputs.pulseNorth = false;

    currentInputs.cameraYaw = gameWorld->GetMainCamera().GetYaw();

    if (Window::GetKeyboard()->KeyDown(KeyCodes::W)) currentInputs.axis.z -= 1;
    if (Window::GetKeyboard()->KeyDown(KeyCodes::S)) currentInputs.axis.z += 1;
    if (Window::GetKeyboard()->KeyDown(KeyCodes::A)) currentInputs.axis.x -= 1;
    if (Window::GetKeyboard()->KeyDown(KeyCodes::D)) currentInputs.axis.x += 1;

    if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {
        currentInputs.jump = true;
    }

    if (Window::GetMouse()->ButtonPressed(MouseButtons::Left)) {
        currentInputs.pulseSouth = true;
    }
    if (Window::GetMouse()->ButtonPressed(MouseButtons::Right)) {
        currentInputs.pulseNorth = true;
    }
}

void Player::TriggerPolarityPulse(Polarity p) {
    polarityPulse = p;
    polarityPulseTimer = polarityPulseDuration;
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
    // 以模型朝向为准
    Quaternion q = GetTransform().GetOrientation();

    // 这里的 (0,0,-1) 是“你认为模型本地前方”的轴
    // 如果还是反，就改成 (0,0,1)
    return q * Vector3(0, 0, 1);
}

