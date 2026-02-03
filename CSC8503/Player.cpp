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

    // polarity hold: press = set, release = neutral
    if (currentInputs.holdSouth && !currentInputs.holdNorth) {
        SetPolarityState(Polarity::South);
    }
    else if (currentInputs.holdNorth && !currentInputs.holdSouth) {
        SetPolarityState(Polarity::North);
    }
    else {
        SetPolarityState(Polarity::None);
    }

    // update colour (current polarity)
    if (GetRenderObject()) {
        GetRenderObject()->SetColour(ColourForPolarity(GetPolarityPulse()));
    }
	// select lock-on candidates
    if (lockModeHeld) {
        SelectCandicatesForLockOn();
    }

    PlayerControl(dt);
}

void Player::ReadLocalInput() {
    currentInputs.axis = Vector3(0, 0, 0);
    currentInputs.jump = false;
    currentInputs.holdSouth = false;
    currentInputs.holdNorth = false;

    currentInputs.cameraYaw = gameWorld->GetMainCamera().GetYaw();

    if (Window::GetKeyboard()->KeyDown(KeyCodes::W)) currentInputs.axis.z -= 1;
    if (Window::GetKeyboard()->KeyDown(KeyCodes::S)) currentInputs.axis.z += 1;
    if (Window::GetKeyboard()->KeyDown(KeyCodes::A)) currentInputs.axis.x -= 1;
    if (Window::GetKeyboard()->KeyDown(KeyCodes::D)) currentInputs.axis.x += 1;

    if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {
        currentInputs.jump = true;
    }

    if (Window::GetMouse()->ButtonDown(MouseButtons::Left)) {
        currentInputs.holdSouth = true;
    }
    if (Window::GetMouse()->ButtonDown(MouseButtons::Right)) {
        currentInputs.holdNorth = true;
    }

    if (Window::GetKeyboard()->KeyDown(KeyCodes::LMENU)) {
        // select candicates in the screen
        lockModeHeld = true;
    }
    else {
		lockModeHeld = false;
    }
}

void Player::SetPolarityState(Polarity p) {
    // avoid repeating one-shot effects every frame
    if (polarityState == p) return;
    polarityState = p;
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
    // ģͳΪ׼
    Quaternion q = GetTransform().GetOrientation();

    //  (0,0,-1) ǡΪģͱǰ
    // Ƿ͸ĳ (0,0,1)
    return q * Vector3(0, 0, 1);
}

/*struct Candidate {
    GameObject* obj;
    float centerError;   // 越小越靠近屏幕中心
    float distPlayer;    // 越小越近
    float score;         // 综合排序
};

void Player::SelectCandicatesForLockOn() {
    candidates.clear();

    Vector3 camPos = camera->GetPosition();
    Vector3 camFwd = camera->GetForwardVector().Normalised();
    Vector3 playerPos = transform.GetPosition();

    for (GameObject* obj : magneticObjects) { // 你的磁性物体列表
        if (!obj) continue;

        Vector3 objPos = obj->GetTransform().GetPosition();
        Vector3 toObj = objPos - camPos;
        float distCam = toObj.Length();
        if (distCam <= 0.01f) continue;
        if (distCam > lockRadius) continue;

        Vector3 dir = toObj / distCam;
        float dot = Vector3::Dot(camFwd, dir);

        // 在视线前方 + 粗略视锥（你可调阈值）
        if (dot < 0.15f) continue;

        // centerError 越小越接近屏幕中心
        float centerError = 1.0f - dot;

        float distPlayer = (objPos - playerPos).Length();

        Candidate c;
        c.obj = obj;
        c.centerError = centerError;
        c.distPlayer = distPlayer;

        // 综合评分：中心优先，距离次之（权重可调）
        c.score = centerError * 10.0f + distPlayer * 0.05f;

        candidates.push_back(c);
    }

    std::sort(candidates.begin(), candidates.end(),
        [](const Candidate& a, const Candidate& b) {
            // 二级排序：中心优先，中心接近时再比距离
            if (fabs(a.centerError - b.centerError) > 0.02f)
                return a.centerError < b.centerError;
            return a.distPlayer < b.distPlayer;
        });

    // 选中第一个作为 hover
    if (!candidates.empty()) {
        hoverTarget = candidates[0].obj;
        hoverIndex = 0;
    }
    else {
        hoverTarget = nullptr;
        hoverIndex = -1;
    }
}*/

