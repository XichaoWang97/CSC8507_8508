#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace NCL::CSC8503 {

    // Dialogue data (pure structs)
    struct DAction {
        // set_flag / clear_flag
        std::string type;
        std::string key;
        bool value = false;
    };

    struct DChoice {
        std::string text;
        std::string next; // node id
        bool end = false;
        std::vector<DAction> actions;
    };

    struct DNode {
        std::string id;
        std::string speaker;
        std::string text;
        std::vector<DChoice> choices;
    };

    struct DGraph {
        std::string id;
        std::string start; // start node id
        std::unordered_map<std::string, DNode> nodes;
    };

    // Minimal blackboard: bool flags only
    struct DBlackboard {
        std::unordered_map<std::string, bool> flags;

        bool GetFlag(const std::string& k, bool def = false) const {
            auto it = flags.find(k);
            return it == flags.end() ? def : it->second;
        }
        void SetFlag(const std::string& k, bool v) { flags[k] = v; }
        void ClearFlag(const std::string& k) { flags.erase(k); }
    };

}