// Assimp/NCLAssimpMesh.h
#pragma once
#include <string>

namespace NCL::Rendering { class Mesh; }
namespace NCL::CSC8503 { class GameTechRendererInterface; }

namespace NCL::Assets {
    NCL::Rendering::Mesh* LoadNCLMeshFromAssimp(
        NCL::CSC8503::GameTechRendererInterface& renderer,
        const std::string& fbxPath);
}