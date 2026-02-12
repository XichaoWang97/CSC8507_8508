#pragma once
#include <string>

#include "DialogueTypes.h"

namespace NCL::CSC8503 {
    class DialogueRunner {
    public:
        bool Begin(const DGraph& g) {
            graph = &g;
            nodeId = g.start;
            active = true;
            return true;
        }

        void End() {
            graph = nullptr;
            nodeId.clear();
            active = false;
        }

        bool IsActive() const { return active; }

        const DNode* CurrentNode() const {
            if (!active || !graph) return nullptr;
            auto it = graph->nodes.find(nodeId);
            return it == graph->nodes.end() ? nullptr : &it->second;
        }

        int ChoiceCount() const {
            const DNode* n = CurrentNode();
            return n ? (int)n->choices.size() : 0;
        }

        // idx: 0-based
        bool Choose(int idx, DBlackboard& bb) {
            const DNode* n = CurrentNode();
            if (!n) return false;
            if (idx < 0 || idx >= (int)n->choices.size()) return false;

            const DChoice& ch = n->choices[idx];

            // actions
            for (const auto& a : ch.actions) {
                if (a.type == "set_flag") bb.SetFlag(a.key, a.value);
                else if (a.type == "clear_flag") bb.ClearFlag(a.key);
            }

            // end?
            if (ch.end) { End(); return true; }

            // jump
            if (!ch.next.empty() && graph->nodes.find(ch.next) != graph->nodes.end()) {
                nodeId = ch.next;
                return true;
            }

            // invalid next -> end to avoid dead state
            End();
            return true;
        }

    private:
        const DGraph* graph = nullptr;
        std::string nodeId;
        bool active = false;
    };

}