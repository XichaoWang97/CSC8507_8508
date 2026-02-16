#pragma once
#include <string>

#include "GameObject.h"
#include "DialogueSystem.h"
#include "Vector.h"
#include "Debug.h"

namespace NCL::CSC8503 {

    class DialogueNPC : public GameObject {
    public:
        DialogueNPC(const std::string& dialogueGraphId, float interactRadius = 6.0f, const std::string& name = "DialogueNPC")
            : graphId(dialogueGraphId), radius(interactRadius), GameObject(name) {
        }

		// every frame, MyGame will call this for each NPC, passing in the player's position and whether E is pressed
        bool UpdateInteract(const NCL::Maths::Vector3& playerPos, bool keyFPressed) {
            // 正在对话就不提示/不触发（也可以选择仍然不提示）
            if (DialogueSystem::Get().IsActive()) return false;

            const NCL::Maths::Vector3& npcPos = GetTransform().GetPosition();
            auto d = playerPos - npcPos;

            float distSq = Vector::LengthSquared(d);
            const bool inRange = distSq <= radius * radius;

            // 1) 靠近提示
            if (inRange) {
                Debug::Print("Press E to talk", Vector2(5, 10), Vector4(1, 1, 1, 1));
            }
            // 2) 必须在范围内 + 按键才触发
            if (!inRange) return false;
            if (!keyFPressed) return false;

            bool ok = DialogueSystem::Get().TryStart(graphId);

            // 3) 触发成功时打印“对话成功”
            if (ok) {
                std::cout << "Dialogue Started: " << graphId << std::endl;
            }
            else {
				std::cout << "Dialogue Failed to start: " << graphId << std::endl;
            }

            return ok;
        }

        const std::string& GetGraphId() const { return graphId; }
        float GetInteractRadius() const { return radius; }

    private:
        std::string graphId;
        float radius = 20.0f;
    };

}