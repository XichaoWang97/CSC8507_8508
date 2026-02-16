#pragma once

#include "Player.h"
#include "MetalObject.h"
#include "Dialogue/DialogueNPC.h" 

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

            std::vector<MetalObject*> metalObjects; // All metal objects that can be pulled / pushed
			std::vector<DialogueNPC*> dialogueNPCs; // List of dialogue NPCs for interaction checks
            std::unique_ptr<Level> currentLevel; // Current level instance

            // Magnet tuning (simple)
            float interactConeDot = 0.6f;   // >0.6 means roughly in front
            float interactForce = 250.0f;   // base force magnitude (F). Acceleration will depend on mass automatically.
        };
    }
}
