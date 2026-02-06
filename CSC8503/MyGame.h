#pragma once

#include "Player.h"
#include "MetalObject.h"
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
            GameObject*   AddFloorToWorld(const NCL::Maths::Vector3& position);
            Player*       AddPlayerToWorld(const NCL::Maths::Vector3& position, float radius, float inverseMass);

            // Pullable / pushable objects
            MetalObject*  AddMetalCubeToWorld(const NCL::Maths::Vector3& position,
                const NCL::Maths::Vector3& halfDims,
                float inverseMass);

            MetalObject*  AddMetalOBBCubeToWorld(const NCL::Maths::Vector3& position,
                NCL::Maths::Vector3 dimensions,
                float inverseMass);

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

            // All metal objects that can be pulled / pushed
            std::vector<MetalObject*> metalObjects;

            // Assets
            Rendering::Mesh* cubeMesh = nullptr;
            Rendering::Mesh* playerMesh = nullptr;

            Rendering::Texture* defaultTex = nullptr;
            GameTechMaterial notexMaterial;

            // Magnet tuning (simple)
            float interactConeDot = 0.6f;   // >0.6 means roughly in front
            float interactForce = 250.0f;   // base force magnitude (F). Acceleration will depend on mass automatically.
        };
    }
}
