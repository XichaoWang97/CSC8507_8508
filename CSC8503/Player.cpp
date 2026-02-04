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
    if (currentInputs.toggleHardLockPressed) {
        lockMode = LockMode::HardLock;
		hardTarget = preTarget;
    }
    else {
        lockMode = LockMode::PreLock;
		preTarget = SelectBestPreTarget(); // always update pre-target
		hardTarget = nullptr;
    }

	Vector3 player_position = GetTransform().GetPosition();

	// Debug lines to targets
	if (lockMode == LockMode::PreLock)  {
		Vector3 pos = preTarget->GetTransform().GetPosition();
        Debug::DrawLine(player_position, pos, Vector4(1.0f, 0.55f, 0.0f, 0.25f)); // orange for pre-target
    }

    if (lockMode == LockMode::HardLock) {
        Vector3 pos = hardTarget->GetTransform().GetPosition();
        Debug::DrawLine(player_position, pos, Vector4(1.0f, 0.55f, 0.0f, 0.75f));
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

	// F: toggle lock-on mode
    if (Window::GetKeyboard()->KeyPressed(KeyCodes::F)) currentInputs.toggleHardLockPressed = !currentInputs.toggleHardLockPressed;
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

// select best pre-target based on camera position/direction
MetalObject* Player::SelectBestPreTarget() {
    auto& cam = gameWorld->GetMainCamera();
    Vector3 camPos = cam.GetPosition();
    Vector3 camFwd = Vector::Normalise(cam.GetForwardVector());
    Vector3 playerPos = transform.GetPosition();

    MetalObject* best = nullptr;
    float bestCenter = 1e9f;
    float bestDist = 1e9f;

	if (!metalObjects) return nullptr;

	for (MetalObject* obj : *metalObjects) { // list of potential targets
        if (!obj) continue;

        Vector3 objPos = obj->GetTransform().GetPosition();
        Vector3 toObj = objPos - camPos;
        float distCam = Vector::Length(toObj);
        if (distCam < 0.01f || distCam > lockRadius) continue;

        Vector3 dir = toObj / distCam;
        float dot = Vector::Dot(camFwd, dir);
        if (dot < minFacingDot) continue;

        float centerError = 1.0f - dot;
        float distPlayer = Vector::Length((objPos - playerPos));

        // Optional: line-of-sight check (avoid locking through walls)
        Ray ray(camPos, dir);
        RayCollision hit;
        if (gameWorld->Raycast(ray, hit, true, this)) {
            // If the first thing we hit isn't the candidate object, skip it
            if (hit.node != obj) {
                continue;
            }
        }

		// best candidate selection
        bool better = false;
        if (centerError + centerTieEps < bestCenter) better = true;
        else if (fabs(centerError - bestCenter) <= centerTieEps && distPlayer < bestDist) better = true;

        if (better) {
            best = obj;
            bestCenter = centerError;
            bestDist = distPlayer;
        }
    }
    return best;
}
