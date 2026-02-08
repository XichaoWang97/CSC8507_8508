#pragma once
#include <vector>
#include <memory>
#include <string>

#include "Vector.h"
#include "RenderObject.h" // GameTechMaterial, Rendering::Mesh/Texture forward deps in this framework

namespace NCL {
    namespace Rendering {
        class Mesh;
        class Texture;
    }
}

namespace NCL::CSC8503 {
    class GameWorld;
    class PhysicsSystem;
    class GameTechRendererInterface;
    class GameObject;
    class Player;
    class MetalObject;

    struct LevelContext {
        GameWorld* world = nullptr;
        PhysicsSystem* physics = nullptr;
        GameTechRendererInterface* renderer = nullptr;

        // Optional outputs / shared lists owned by MyGame
        std::vector<MetalObject*>* metalObjects = nullptr;
        Player** playerOut = nullptr;
    };

    class Level {
    public:
        virtual ~Level() = default;

        void SetContext(const LevelContext& ctx) { context = ctx; }

        virtual void Build() = 0;

    protected:
        // Shared helpers for building levels
        GameObject* AddFloorToWorld(const NCL::Maths::Vector3& position);
        Player* AddPlayerToWorld(const NCL::Maths::Vector3& position, float radius, float inverseMass);
        MetalObject* AddMetalCubeToWorld(const NCL::Maths::Vector3& position,
            const NCL::Maths::Vector3& halfDims,
            float inverseMass,
            const NCL::Maths::Vector4& colour = NCL::Maths::Vector4(0.65f, 0.65f, 0.7f, 1.0f));

        void ClearWorld();

    protected:
        // assets
        void EnsureAssetsLoaded();

        NCL::Rendering::Mesh* cubeMesh = nullptr;
        NCL::Rendering::Mesh* playerMesh = nullptr;
        NCL::Rendering::Texture* defaultTex = nullptr;
        GameTechMaterial notexMaterial;

        LevelContext context;
    };
}