#include "Level.h"
#include "LevelRegistry.h"

using namespace NCL::CSC8503;
using namespace NCL::Maths;

// Tutorial Level
namespace {
    class Level_00 : public Level {
    public:
        void Build() override {
            AddFloorToWorld(Vector3(0, -1, 0));

            AddPlayerToWorld(Vector3(0, 3, 0), 2.0f, 0.5f);

            AddMetalCubeToWorld(Vector3(0, 2, -12), Vector3(2, 2, 2), 1.0f);
            AddMetalCubeToWorld(Vector3(6, 2, -10), Vector3(2, 2, 2), 1.0f);

			// if you want to make it to an anchor, set inverse mass to 0 and it won't move
            AddMetalCubeToWorld(Vector3(-6, 20, -10), Vector3(2, 2, 2), 0.0f, Vector4(0.9f, 0.9f, 0.2f, 1.0f));

			LoadDialogue("../../CSC8503/Dialogue/Tutorial_Level.json"); // Make sure the path is correct for your project structure
            AddDialogueNPCToWorld("npc_tutorial", Vector3(-5, 2, -5), 1.0f, 0.1f, Vector4(0.5f, 0.0f, 0.5f, 1.0f));
        }
    };

    struct AutoRegister {
        AutoRegister() {
            LevelRegistry::Get().Register([]() { return std::make_unique<Level_00>(); });
        }
    } _autoRegister;
}