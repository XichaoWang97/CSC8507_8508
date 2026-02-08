#pragma once

#include "Player.h"
#include "MetalObject.h"
#include <vector>
#include <memory>

namespace NCL {
    class Controller;

    namespace CSC8503 {
        class GameTechRendererInterface;
        class PhysicsSystem;
        class GameWorld;
        class Level;

        class MyGame {
        public:
            MyGame(GameWorld& gameWorld, GameTechRendererInterface& renderer, PhysicsSystem& physics);
            ~MyGame();

            void UpdateGame(float dt);
            void ResetGame() { InitWorld(); }

        private:
            void InitCamera();
            void InitWorld();

            // Camera follow logic
            void SetCameraToPlayer(Player* player);

            // Pull / Push interaction (MVP)
            void ApplyPullPush(Player* player);

        private:
            GameWorld& world;
            GameTechRendererInterface& renderer;
            PhysicsSystem& physics;

            Controller* controller = nullptr;

            Player* player = nullptr;

            // All metal objects that can be pulled / pushed (shared with levels)
            std::vector<MetalObject*> metalObjects;

            // Current level instance
            std::unique_ptr<Level> currentLevel;

            // Magnet tuning (simple)
            float interactConeDot = 0.6f;   // >0.6 means roughly in front
            float interactForce = 250.0f;   // base force magnitude (F). Acceleration will depend on mass automatically.
        };
    }
}
