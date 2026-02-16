#pragma once
#include <string>

#include "DialogueDB.h"
#include "DialogueRunner.h"
#include "DialogueTypes.h"

namespace NCL::CSC8503 {

    class DialogueSystem {
    public:
        static DialogueSystem& Get() {
            static DialogueSystem s;
            return s;
        }

        // Load once (or multiple files) into DB
        bool LoadGraphFromFile(const std::string& filepath) {
            return db.LoadGraphFromFile(filepath);
        }

        // Try to begin a graph by id
        bool TryStart(const std::string& graphId) {
            if (runner.IsActive()) return false;
            const DGraph* g = db.Get(graphId);
            if (!g) return false;
            return runner.Begin(*g);
        }

        void ForceEnd() { runner.End(); }

        bool IsActive() const { return runner.IsActive(); }

        // Called by MyGame each frame after input is gathered
        void Update(bool choose1, bool choose2, bool choose3, bool choose4, bool choose5);

        // Expose blackboard if you want later
        DBlackboard& Blackboard() { return bb; }
        const DBlackboard& Blackboard() const { return bb; }

    private:
        DialogueSystem() = default;

        DialogueDB db;
        DialogueRunner runner;
        DBlackboard bb;
    };

}