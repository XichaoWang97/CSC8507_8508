#pragma once

namespace NCL::Rendering {
	class Mesh;
	class Texture;
	class Shader;
}

namespace NCL::CSC8503 {
	class GameTechRendererInterface
	{
	public:
		virtual NCL::Rendering::Mesh*		LoadMesh(const std::string& name)		= 0;
		virtual NCL::Rendering::Texture*	LoadTexture(const std::string& name)	= 0;

		// New: create a mesh from rendering backedn, and upload it to GPU
		virtual NCL::Rendering::Mesh* CreateMesh() = 0;
		virtual void UploadMesh(NCL::Rendering::Mesh* mesh) = 0;
	};
}

