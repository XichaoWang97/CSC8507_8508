#pragma once
#include "GameObject.h"
#include "Window.h"
#include "RenderObject.h"

namespace NCL::CSC8503 {

    class GameWorld;

    enum class Polarity : uint8_t {
        None = 0,
        South = 1, // Orange
        North = 2  // Blue
    };

    struct PlayerInputs {
        NCL::Maths::Vector3 axis = NCL::Maths::Vector3(0, 0, 0);
        bool jump = false;

        // polarity hold (while pressed)
        bool holdSouth = false; // LMB
        bool holdNorth = false; // RMB

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

        // Polarity (hold)
        bool     IsPolarityPulseActive() const { return polarityState != Polarity::None; }
        Polarity GetPolarityPulse() const { return polarityState; }

        // Magnet helpers
        NCL::Maths::Vector3 GetMagnetOrigin();  // non-const to avoid const-correctness issues in your engine
        NCL::Maths::Vector3 GetAimForward();

    private:
        void PlayerControl(float dt);
        bool IsPlayerOnGround();               // non-const (Raycast ignore expects GameObject*)
        
        void SetPolarityState(Polarity p);

        void ReadLocalInput();                 // fills currentInputs
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

        // polarity state (hold)
        Polarity polarityState = Polarity::None;
    };
}
