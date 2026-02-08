#pragma once
#include <memory>
#include <vector>
#include <functional>

namespace NCL::CSC8503 {
    class Level;

    class LevelRegistry {
    public:
        using Factory = std::function<std::unique_ptr<Level>()>;

        static LevelRegistry& Get(); // singleton accessor

        void Register(Factory f); // register a factory function for creating a level
        int  Count() const; // get the number of registered levels
        std::unique_ptr<Level> Create(int index); // create a level instance by index

    private:
        std::vector<Factory> factories;
    };
}