#pragma once
#include <string>
#include <unordered_map>

#include "DialogueTypes.h"
#include <../GLTFLoader/json.hpp>
#include <fstream>

namespace NCL::CSC8503 {

    // One JSON file -> one graph (simple)
    class DialogueDB {
    public:
        bool LoadGraphFromFile(const std::string& filepath) {
            std::ifstream f(filepath);
            if (!f.is_open()) return false;

            nlohmann::json root;
            try { f >> root; }
            catch (...) { return false; }

            DGraph g;
            g.id = root.value("id", "");
            g.start = root.value("start", "");
            if (g.id.empty() || g.start.empty()) return false;

            if (!root.contains("nodes") || !root["nodes"].is_object()) return false;

            for (auto it = root["nodes"].begin(); it != root["nodes"].end(); ++it) {
                DNode node;
                node.id = it.key();
                node.speaker = it.value().value("speaker", "");
                node.text = it.value().value("text", "");

                if (it.value().contains("choices") && it.value()["choices"].is_array()) {
                    for (auto& cj : it.value()["choices"]) {
                        DChoice ch;
                        ch.text = cj.value("text", "");
                        ch.next = cj.value("next", "");
                        ch.end = cj.value("end", false);

                        if (cj.contains("actions") && cj["actions"].is_array()) {
                            for (auto& aj : cj["actions"]) {
                                DAction a;
                                a.type = aj.value("type", "");
                                a.key = aj.value("key", "");
                                a.value = aj.value("value", false);
                                ch.actions.push_back(a);
                            }
                        }
                        node.choices.push_back(std::move(ch));
                    }
                }

                g.nodes.emplace(node.id, std::move(node));
            }

            if (g.nodes.find(g.start) == g.nodes.end()) return false;

            graphs[g.id] = std::move(g);
            return true;
        }

        const DGraph* Get(const std::string& id) const {
            auto it = graphs.find(id);
            return it == graphs.end() ? nullptr : &it->second;
        }

    private:
        // graphId -> graph
        std::unordered_map<std::string, DGraph> graphs;
    };

}
