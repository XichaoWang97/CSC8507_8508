#pragma once
/*
    AssimpModelLoader.h (header-only)

    Purpose:
    - Runtime import of FBX/OBJ/GLTF (and any format supported by Assimp) into a lightweight CPU-side ModelData.
    - Includes:
        * Mesh geometry (positions, normals, UV0, tangents, indices)
        * Material references (basic color + texture file paths)
        * Skinning data (up to 4 bone influences per vertex) + per-bone inverse bind pose

    Notes:
    - This file only PARSES data from Assimp. It does NOT create GPU resources.
      In your NCL framework you should convert ModelData -> NCL::Rendering::Mesh via your renderer factory
      (CreateMesh / UploadMesh), and convert MaterialData texture paths -> renderer->LoadTexture(...).
    - Animation sampling (aiScene->mAnimations) and skeleton hierarchy (joint parents / bind pose) are NOT
      included here. This file only provides the data you need to start skinning and materials.

    Integration:
    - Requires:
        * Assimp headers/libraries
        * NCL Maths types: Vector2, Vector3, Vector4, Vector4i, Matrix4
        * Assets.h for NCL::Assets::MESHDIR (your existing asset base directory)

    Common pitfalls:
    - FBX files often contain multiple meshes (body/head/eyes/etc). You must merge or render all submeshes.
    - Many rigged FBX assets need actual animation/skeleton evaluation to look correct.
*/

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <filesystem>
#include <cstdint>
#include <string>
#include <vector>
#include <stdexcept>
#include <unordered_map>

#include "Assets.h"   // for NCL::Assets::MESHDIR
#include "Vector.h"
#include "Matrix.h"

namespace NCL::Assets {

    // -----------------------------
    // Vertex data we extract per mesh
    // -----------------------------
    struct Vertex {
        float px = 0, py = 0, pz = 0;     // position
        float nx = 0, ny = 0, nz = 0;     // normal
        float u = 0, v = 0;             // UV0
        float tx = 0, ty = 0, tz = 0;     // tangent (xyz)
        float bx = 0, by = 0, bz = 0;     // bitangent (xyz)
    };

    // -----------------------------
    // Minimal material metadata
    // (Your renderer/material system decides how to use these.)
    // -----------------------------
    struct MaterialData {
        NCL::Maths::Vector4 baseColor = NCL::Maths::Vector4(1, 1, 1, 1);
        float metalness = 0.0f;
        float roughness = 1.0f;

        // Texture paths resolved relative to the model directory.
        std::string albedoTex;
        std::string normalTex;
        std::string ormTex;       // occlusion/roughness/metalness (not always available)
        std::string emissiveTex;
    };

    // -----------------------------
    // Per-mesh data extracted from the scene graph
    // -----------------------------
    struct MeshData {
        std::vector<Vertex>   vertices;
        std::vector<uint32_t> indices;

        // Which aiMaterial this mesh uses
        int materialIndex = -1;

        // Skinning (up to 4 influences per vertex) - only filled if the mesh has bones.
        std::vector<NCL::Maths::Vector4>  skinWeights;  // (w0,w1,w2,w3)
        std::vector<NCL::Maths::Vector4i> skinIndices;  // (i0,i1,i2,i3)
        bool isSkinned = false;
    };

    // -----------------------------
    // Whole model container
    // -----------------------------
    struct ModelData {
        std::vector<MeshData> meshes;

        // One entry per aiMaterial in the Assimp scene
        std::vector<MaterialData> materials;

        // Directory of the loaded model file (used for resolving texture paths)
        std::string directory;

        // Global joint tables shared across meshes (bone name -> joint index)
        std::vector<std::string> jointNames;
        std::unordered_map<std::string, int> jointIndex;

        // Inverse bind pose per joint (Assimp aiBone::mOffsetMatrix)
        std::vector<NCL::Maths::Matrix4> inverseBindPose;
    };

    // =========================================================
    // Helpers: texture path resolution & material parsing
    // =========================================================

    inline std::string ResolveTexPath(const aiString& p, const std::string& dir) {
        std::string s = p.C_Str();
        if (s.empty()) return {};

        // FBX may store absolute paths; keep only file name if absolute.
        std::filesystem::path fp(s);
        if (fp.is_absolute()) {
            fp = fp.filename();
        }
        return (std::filesystem::path(dir) / fp).generic_string();
    }

    inline std::string GetTex(aiMaterial* mat, aiTextureType type, const std::string& dir) {
        aiString path;
        if (mat->GetTexture(type, 0, &path) == AI_SUCCESS) {
            return ResolveTexPath(path, dir);
        }
        return {};
    }

    inline void ProcessMaterials(const aiScene* scene, ModelData& out) {
        out.materials.resize(scene->mNumMaterials);

        for (unsigned i = 0; i < scene->mNumMaterials; ++i) {
            aiMaterial* m = scene->mMaterials[i];
            MaterialData md{};

            // Base colour: Assimp commonly provides diffuse; some PBR assets use base color.
            aiColor4D col;
            if (aiGetMaterialColor(m, AI_MATKEY_COLOR_DIFFUSE, &col) == AI_SUCCESS) {
                md.baseColor = NCL::Maths::Vector4(col.r, col.g, col.b, col.a);
            }

            // Texture conventions vary widely between exporters.
            // We try common slots and fall back where exporters misuse types.
            md.albedoTex = GetTex(m, aiTextureType_BASE_COLOR, out.directory);
            if (md.albedoTex.empty()) md.albedoTex = GetTex(m, aiTextureType_DIFFUSE, out.directory);

            md.normalTex = GetTex(m, aiTextureType_NORMALS, out.directory);
            if (md.normalTex.empty()) md.normalTex = GetTex(m, aiTextureType_HEIGHT, out.directory);

            md.emissiveTex = GetTex(m, aiTextureType_EMISSIVE, out.directory);

            // ORM has no standard in Assimp; many pipelines pack it into "UNKNOWN" or split textures.
            // Leave empty for now and handle per-project if needed.
            // md.ormTex = GetTex(m, aiTextureType_UNKNOWN, out.directory);

            out.materials[i] = std::move(md);
        }
    }

    // =========================================================
    // Helpers: matrix conversion + skin influence packing
    // =========================================================

    inline NCL::Maths::Matrix4 ToMat4(const aiMatrix4x4& m) {
        using namespace NCL::Maths;
        Matrix4 r;
        r.SetRow(0, Vector4(m.a1, m.b1, m.c1, m.d1));
        r.SetRow(1, Vector4(m.a2, m.b2, m.c2, m.d2));
        r.SetRow(2, Vector4(m.a3, m.b3, m.c3, m.d3));
        r.SetRow(3, Vector4(m.a4, m.b4, m.c4, m.d4));
        return r;
    }

    inline void AddInfluence(NCL::Maths::Vector4& w, NCL::Maths::Vector4i& idx, int bone, float weight) {
        // Insert (bone, weight) into the top-4 influences, sorted by descending weight.
        // This is a simple 4-slot insertion sort.
        for (int k = 0; k < 4; ++k) {
            if (weight > w[k]) {
                for (int s = 3; s > k; --s) {
                    w[s] = w[s - 1];
                    idx[s] = idx[s - 1];
                }
                w[k] = weight;
                idx[k] = bone;
                return;
            }
        }
    }

    inline void ProcessMeshSkinned(const aiMesh* m, ModelData& model, MeshData& out) {
        if (m->mNumBones == 0) {
            return;
        }

        out.isSkinned = true;
        out.skinWeights.assign(m->mNumVertices, NCL::Maths::Vector4(0, 0, 0, 0));
        out.skinIndices.assign(m->mNumVertices, NCL::Maths::Vector4i(0, 0, 0, 0));

        for (unsigned b = 0; b < m->mNumBones; ++b) {
            aiBone* bone = m->mBones[b];
            std::string name = bone->mName.C_Str();

            int boneIndex = 0;
            auto it = model.jointIndex.find(name);
            if (it == model.jointIndex.end()) {
                boneIndex = (int)model.jointNames.size();
                model.jointIndex[name] = boneIndex;
                model.jointNames.push_back(name);

                // aiBone::mOffsetMatrix is typically the inverse bind pose (mesh space)
                model.inverseBindPose.push_back(ToMat4(bone->mOffsetMatrix));
            }
            else {
                boneIndex = it->second;
            }

            for (unsigned w = 0; w < bone->mNumWeights; ++w) {
                const aiVertexWeight& vw = bone->mWeights[w];
                if (vw.mVertexId < m->mNumVertices) {
                    AddInfluence(out.skinWeights[vw.mVertexId], out.skinIndices[vw.mVertexId], boneIndex, vw.mWeight);
                }
            }
        }

        // Normalize weights per vertex
        for (size_t i = 0; i < out.skinWeights.size(); ++i) {
            float sum = out.skinWeights[i].x + out.skinWeights[i].y + out.skinWeights[i].z + out.skinWeights[i].w;
            if (sum > 0.00001f) {
                out.skinWeights[i] /= sum;
            }
            else {
                // If no weights, default to bone 0 with weight 1
                out.skinWeights[i] = NCL::Maths::Vector4(1, 0, 0, 0);
                out.skinIndices[i] = NCL::Maths::Vector4i(0, 0, 0, 0);
            }
        }
    }

    // =========================================================
    // Geometry parsing: mesh + node traversal
    // =========================================================

    inline void ProcessMesh(const aiMesh* m, ModelData& model, MeshData& out) {
        out.vertices.reserve(m->mNumVertices);

        const bool hasNormals = m->HasNormals();
        const bool hasUV0 = m->HasTextureCoords(0);
        const bool hasTangents = (m->mTangents != nullptr);
        const bool hasBitangents = (m->mBitangents != nullptr);

        for (unsigned i = 0; i < m->mNumVertices; ++i) {
            Vertex v{};

            // Position
            v.px = m->mVertices[i].x;
            v.py = m->mVertices[i].y;
            v.pz = m->mVertices[i].z;

            // Normal
            if (hasNormals) {
                v.nx = m->mNormals[i].x;
                v.ny = m->mNormals[i].y;
                v.nz = m->mNormals[i].z;
            }

            // UV0
            if (hasUV0) {
                v.u = m->mTextureCoords[0][i].x;
                v.v = m->mTextureCoords[0][i].y;
                // If your UVs look upside-down in your renderer, try:
                // v.v = 1.0f - v.v;
            }

            // Tangent / Bitangent
            if (hasTangents) {
                v.tx = m->mTangents[i].x;
                v.ty = m->mTangents[i].y;
                v.tz = m->mTangents[i].z;
            }
            if (hasBitangents) {
                v.bx = m->mBitangents[i].x;
                v.by = m->mBitangents[i].y;
                v.bz = m->mBitangents[i].z;
            }

            out.vertices.push_back(v);
        }

        // Indices (assumes triangulated)
        out.indices.reserve(static_cast<size_t>(m->mNumFaces) * 3);
        for (unsigned f = 0; f < m->mNumFaces; ++f) {
            const aiFace& face = m->mFaces[f];
            for (unsigned k = 0; k < face.mNumIndices; ++k) {
                out.indices.push_back(static_cast<uint32_t>(face.mIndices[k]));
            }
        }

        // Skinning (optional)
        ProcessMeshSkinned(m, model, out);
    }

    inline void ProcessNode(const aiNode* node, const aiScene* scene, ModelData& out) {
        for (unsigned i = 0; i < node->mNumMeshes; ++i) {
            const aiMesh* m = scene->mMeshes[node->mMeshes[i]];
            MeshData data;

            data.materialIndex = (int)m->mMaterialIndex; // record which material this mesh uses
            ProcessMesh(m, out, data);

            out.meshes.push_back(std::move(data));
        }

        for (unsigned c = 0; c < node->mNumChildren; ++c) {
            ProcessNode(node->mChildren[c], scene, out);
        }
    }

    // =========================================================
    // Import flags + main entry point
    // =========================================================

    inline unsigned DefaultAssimpFlags() {
        return
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_GenSmoothNormals |
            aiProcess_CalcTangentSpace |
            aiProcess_ImproveCacheLocality |
            aiProcess_SortByPType;
    }

    inline ModelData LoadModelFromFile(const std::string& path, unsigned flags = DefaultAssimpFlags()) {
        // Try direct path first; if not found, prepend MESHDIR.
        std::string fullPath = path;
        if (!std::filesystem::exists(fullPath)) {
            fullPath = NCL::Assets::MESHDIR + path;
        }

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(fullPath, flags);
        if (!scene || !scene->mRootNode) {
            throw std::runtime_error(
                std::string("Assimp load failed: ") + importer.GetErrorString() +
                " (path: " + fullPath + ")"
            );
        }

        ModelData result;

        // Directory used for resolving textures
        result.directory = std::filesystem::path(fullPath).parent_path().generic_string();

        // Materials first (so texture paths can be resolved consistently)
        ProcessMaterials(scene, result);

        // Traverse the node hierarchy and extract meshes
        ProcessNode(scene->mRootNode, scene, result);

        return result;
    }

} // namespace NCL::Assets
