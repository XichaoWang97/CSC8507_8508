#include "NCLAssimpMesh.h"
#include "ModelLoader.h"
#include "../GameTechRendererInterface.h"
#include "Mesh.h"

using namespace NCL;
using namespace NCL::Maths;

namespace NCL::Assets {

    NCL::Rendering::Mesh* LoadNCLMeshFromAssimp(
        NCL::CSC8503::GameTechRendererInterface& renderer,
        const std::string& fbxPath)
    {
        auto model = LoadModelFromFile(fbxPath);
        if (model.meshes.empty()) {
            throw std::runtime_error("Assimp: no meshes in " + fbxPath);
        }

        auto* mesh = renderer.CreateMesh();

        std::vector<NCL::Maths::Vector3> positions;
        std::vector<NCL::Maths::Vector3> normals;
        std::vector<NCL::Maths::Vector2> uvs;
        std::vector<NCL::Maths::Vector4> tangents;
        std::vector<unsigned int> indices;

        mesh->SetPrimitiveType(NCL::Rendering::GeometryPrimitive::Triangles);
        mesh->SetDebugName(fbxPath);

        unsigned int baseVertex = 0;
        int startIndex = 0;

        for (size_t mi = 0; mi < model.meshes.size(); ++mi) {
            const auto& src = model.meshes[mi];

            // append vertices
            for (const auto& v : src.vertices) {
                positions.emplace_back(v.px, v.py, v.pz);
                normals.emplace_back(v.nx, v.ny, v.nz);
                uvs.emplace_back(v.u, v.v);
                tangents.emplace_back(v.tx, v.ty, v.tz, 1.0f);
            }

            // append indices with baseVertex offset
            for (auto i : src.indices) {
                indices.push_back(baseVertex + (unsigned int)i);
            }

            // record submesh range (optional but nice)
            const int indexCount = (int)src.indices.size();
            mesh->AddSubMesh(startIndex, indexCount, 0, "AssimpSubMesh" + std::to_string(mi));

            startIndex += indexCount;
            baseVertex += (unsigned int)src.vertices.size();
        }

        mesh->SetVertexPositions(positions);
        mesh->SetVertexNormals(normals);
        mesh->SetVertexTextureCoords(uvs);
        mesh->SetVertexTangents(tangents);
        mesh->SetVertexIndices(indices);

        renderer.UploadMesh(mesh);
        return mesh;
    }
}