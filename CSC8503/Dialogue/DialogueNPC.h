#pragma once
#include <string>

#include "GameObject.h"
#include "DialogueDB.h"
#include "DialogueRunner.h"
#include "DialogueTypes.h"

namespace NCL::CSC8503 {

    class Keyboard; // 前置声明，避免 include 太多

    class DialogueNPC : public GameObject {
    public:
        DialogueNPC(const std::string& dialogueGraphId, float interactRadius = 6.0f, const std::string& name = "DialogueNPC")
            : graphId(dialogueGraphId) , radius(interactRadius), GameObject(name) {
        }

        // 每帧调用：如果玩家在范围内且按 E，就开始对话
        // keyEPressed: 这一帧是否按下了 E（用 MyGame 里读键盘后传进来，最稳）
        bool UpdateInteract(const NCL::Maths::Vector3& playerPos,
            bool keyEPressed,
            DialogueDB& db,
            DialogueRunner& runner)
        {
            if (!keyEPressed) return false;      // 必须按E
            if (runner.IsActive()) return false; // 正在对话就不重复触发

            // 距离检测
            const Vector3& npcPos = GetTransform().GetPosition();

            // 兼容不同 Vector3 实现：优先用 LengthSquared()，没有就改成 d.Length() * d.Length()
            const float distSq = Vector::LengthSquared(playerPos - npcPos);
            if (distSq > radius * radius) return false;

            // 开始对话
            const DGraph* g = db.Get(graphId);
            if (!g) return false;

            runner.Begin(*g);
            return true;
        }

        const std::string& GetGraphId() const { return graphId; }
        float GetInteractRadius() const { return radius; }

    private:
        std::string graphId;
        float radius = 6.0f;
    };

}