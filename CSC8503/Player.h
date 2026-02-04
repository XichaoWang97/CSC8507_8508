#pragma once
#include "GameObject.h"
#include "Window.h"
#include "RenderObject.h"

namespace NCL::CSC8503 {

    class GameWorld;

    struct PlayerInputs {
        NCL::Maths::Vector3 axis = NCL::Maths::Vector3(0, 0, 0);
        bool jump = false;

        // Magnet interaction (hold while pressed)
        bool pullHeld = false; // LMB
        bool pushHeld = false; // RMB

        float cameraYaw = 0.0f;
    };

    class Player : public GameObject {
    public:
        Player(GameWorld* world);
        ~Player();

        void Update(float dt);

        // For networked mode (optional)
        void SetIgnoreInput(bool ignore) { ignoreInput = ignore; }
        bool GetIgnoreInput() const { return ignoreInput; }
        void SetPlayerInput(const PlayerInputs& inputs) { currentInputs = inputs; }

        // Magnet helpers
        bool IsPullHeld() const { return currentInputs.pullHeld; }
        bool IsPushHeld() const { return currentInputs.pushHeld; }

        NCL::Maths::Vector3 GetMagnetOrigin();  // non-const to avoid const-correctness issues in your engine
        NCL::Maths::Vector3 GetAimForward();

    private:
        void PlayerControl(float dt);
        bool IsPlayerOnGround();               // non-const (Raycast ignore expects GameObject*)

        void ReadLocalInput();                 // fills currentInputs

        // Lock-on (you can flesh this out later; keeping a stub prevents linker errors)
        void SelectCandicatesForLockOn();

    private:
        GameWorld* gameWorld = nullptr;

        bool ignoreInput = false;
        bool lockModeHeld = false;
        PlayerInputs currentInputs;

        // movement tuning
        float moveForce = 60.0f;
        float maxSpeed = 15.0f;
        float rotationSpeed = 10.0f;
        float jumpImpulse = 15.0f;
    };
}
