#include "DialogueSystem.h"

#include "Debug.h"
#include <string>

using namespace NCL::CSC8503;

static std::string SafeStr(const std::string& s) {
    return s.empty() ? std::string("(none)") : s;
}

void DialogueSystem::Update(bool choose1, bool choose2, bool choose3, bool choose4, bool choose5) {
    if (!runner.IsActive()) return;

    const DNode* node = runner.CurrentNode();
    if (!node) {
        runner.End();
        return;
    }

    // o(=•ェ•=)o---- Render via Debug::Print ----o(=•ェ•=)o
    // You can tune positions to your liking.
    // If Debug::Print signature differs in your framework, adjust here only.
    NCL::Debug::Print(SafeStr(node->speaker) + ": " + SafeStr(node->text), Vector2(10, 60));

    int c = (int)node->choices.size();
    for (int i = 0; i < c; ++i) {
        std::string line = std::to_string(i + 1) + ") " + SafeStr(node->choices[i].text);
        NCL::Debug::Print(line, Vector2(10, 70 + 5*i));
    }

    // Input to choose
    int idx = -1;
    if (choose1) idx = 0;
    else if (choose2) idx = 1;
    else if (choose3) idx = 2;
    else if (choose4) idx = 3;
    else if (choose5) idx = 4;

    if (idx >= 0) {
        runner.Choose(idx, bb);
    }
}