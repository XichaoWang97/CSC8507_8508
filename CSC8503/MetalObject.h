#pragma once
#include "GameObject.h"

namespace NCL::CSC8503 {

    /*
        MetalObject: objects that can be affected by the player's pull / push.
        Keep it minimal so it fits the existing NCL framework without extra dependencies.
    */
    class MetalObject : public GameObject {
    public:
        MetalObject(const std::string& name = "MetalObject") : GameObject(name) {}
        ~MetalObject();
    };
}
