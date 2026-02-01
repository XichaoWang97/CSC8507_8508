#pragma once

#include "Player.h"
#include "RenderObject.h"
#include <vector>

namespace NCL {
    class Controller;

    namespace Rendering {
        class Mesh;
        class Texture;
    }

    namespace CSC8503 {
        class GameTechRendererInterface;
        class PhysicsSystem;
        class GameWorld;
        class GameObject;

        class MyGame {
        public:
            MyGame(GameWorld& gameWorld, GameTechRendererInterface& renderer, PhysicsSystem& physics);
            ~MyGame();

            void UpdateGame(float dt);
            void ResetGame() { InitWorld(); }

        private:
            void InitCamera();
            void InitWorld();

            // Level building
            GameObject* AddFloorToWorld(const NCL::Maths::Vector3& position);
            Player* AddPlayerToWorld(const NCL::Maths::Vector3& position, float radius, float inverseMass);
            GameObject* AddMagneticCubeToWorld(const NCL::Maths::Vector3& position,
                const NCL::Maths::Vector3& halfDims,
                float inverseMass,
                Polarity polarity);

            GameObject* AddOBBCubeToWorld(const NCL::Maths::Vector3& position, NCL::Maths::Vector3 dimensions, 
                float inverseMass, Polarity polarity);

            // Camera follow logic (kept as requested)
            void SetCameraToPlayer(Player* player);

            // Magnet interaction (MVP)
            void ApplyMagnetPulse(Player* player, float dt);

        private:

            struct MagneticEntry {
                GameObject* obj = nullptr;
                Polarity polarity = Polarity::None;
            };

            GameWorld& world;
            GameTechRendererInterface& renderer;
            PhysicsSystem& physics;

            Controller* controller = nullptr;

            // Only objects we keep
            Player* player = nullptr;

            // Many magnetic cubes
            std::vector<MagneticEntry> magneticCubes;


            // Assets
            Rendering::Mesh* cubeMesh = nullptr;
            Rendering::Mesh* playerMesh = nullptr;

            Rendering::Texture* defaultTex = nullptr;
            GameTechMaterial notexMaterial;

            // Magnet tuning (simple)
            float magnetRange = 20.0f;
            float magnetConeDot = 0.6f;    // >0.6 means roughly in front
            float magnetForce = 250.0f;    // force applied to cube
            float selfRecoilFactor = 0.25f; // optional recoil on player
        };
    }
}
