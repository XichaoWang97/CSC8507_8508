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

        // polarity pulse triggers (momentary)
        bool pulseSouth = false; // LMB
        bool pulseNorth = false; // RMB

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

        // Polarity pulse
        bool     IsPolarityPulseActive() const { return polarityPulseTimer > 0.0f; }
        Polarity GetPolarityPulse() const { return IsPolarityPulseActive() ? polarityPulse : Polarity::None; }

        // Magnet helpers
        NCL::Maths::Vector3 GetMagnetOrigin();  // non-const to avoid const-correctness issues in your engine
        NCL::Maths::Vector3 GetAimForward();

    private:
        void PlayerControl(float dt);
        bool IsPlayerOnGround();               // non-const (Raycast ignore expects GameObject*)
        void TriggerPolarityPulse(Polarity p);

        void ReadLocalInput();                 // fills currentInputs

    private:
        GameWorld* gameWorld = nullptr;

        bool ignoreInput = false;
        PlayerInputs currentInputs;

        // movement tuning
        float moveForce = 60.0f;
        float maxSpeed = 15.0f;
        float rotationSpeed = 10.0f;
        float jumpImpulse = 15.0f;

        // polarity pulse tuning
        Polarity polarityPulse = Polarity::None;
        float    polarityPulseTimer = 0.0f;
        float    polarityPulseDuration = 0.08f; // “一瞬间”，可改为 0.02f 更短
    };
}
