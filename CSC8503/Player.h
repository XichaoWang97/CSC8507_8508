#pragma once
#include <vector>
#include "GameObject.h"
#include "Window.h"
#include "RenderObject.h"
#include "MetalObject.h"

namespace NCL::CSC8503 {

    class GameWorld;

    enum class LockMode { PreLock, HardLock };

    struct PlayerInputs {
        NCL::Maths::Vector3 axis = NCL::Maths::Vector3(0, 0, 0);
        bool jump = false;
        bool pullHeld = false;
        bool pushHeld = false;
        bool toggleHardLockPressed = false;
        float cameraYaw = 0.0f;
    };

    class Player : public GameObject {
    public:
        Player(GameWorld* world);
        ~Player();

        void Update(float dt);

        void SetIgnoreInput(bool ignore) { ignoreInput = ignore; }
        bool GetIgnoreInput() const { return ignoreInput; }
        void SetPlayerInput(const PlayerInputs& inputs) { currentInputs = inputs; }

        void SetMetalObjects(const std::vector<MetalObject*>* list) { metalObjects = list; }

		// Get the current target based on lock mode. Will return nullptr if no valid target.
        MetalObject* GetLockedTarget() const {
            return hardTarget;
        }
		float GetLockRadius() const { return lockRadius; }
        bool IsPullHeld() const { return currentInputs.pullHeld; }
        bool IsPushHeld() const { return currentInputs.pushHeld; }

        NCL::Maths::Vector3 GetMagnetOrigin();
        NCL::Maths::Vector3 GetAimForward();

    private:
        void PlayerControl(float dt);
        bool IsPlayerOnGround();
        void ReadLocalInput();
        MetalObject* SelectBestPreTarget();

    private:
        GameWorld* gameWorld = nullptr;
        bool ignoreInput = false;
		bool useFirstPerson = true;
        PlayerInputs currentInputs;

        LockMode lockMode = LockMode::PreLock;
        MetalObject* preTarget = nullptr;
        MetalObject* hardTarget = nullptr;

		float lockRadius = 60.0f; // Max distance for locking onto targets and for pull/push interactions
        float minFacingDot = 0.15f;
        float centerTieEps = 0.02f;

        const std::vector<MetalObject*>* metalObjects = nullptr;

        float moveForce = 60.0f;
        float maxSpeed = 15.0f;
        float rotationSpeed = 10.0f;
        float jumpImpulse = 30.0f;
    };

}
