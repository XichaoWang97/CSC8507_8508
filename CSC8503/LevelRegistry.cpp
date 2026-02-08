#include "LevelRegistry.h"
#include "Level.h"

using namespace NCL::CSC8503;

LevelRegistry& LevelRegistry::Get() {
    static LevelRegistry inst;
    return inst;
}

void LevelRegistry::Register(Factory f) {
    factories.push_back(std::move(f));
}

int LevelRegistry::Count() const {
    return (int)factories.size();
}

std::unique_ptr<Level> LevelRegistry::Create(int index) {
    if (index < 0 || index >= (int)factories.size()) return nullptr;
    return factories[index]();
}