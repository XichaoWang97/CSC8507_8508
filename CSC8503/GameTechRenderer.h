#pragma once
#include "OGLRenderer.h"
#include "GameTechRendererInterface.h"
#include "OGLMesh.h"
#include "OGLShader.h"

namespace NCL {
	namespace Rendering {
		class OGLMesh;
		class OGLShader;
		class OGLTexture;
		class OGLBuffer;
	};
	namespace CSC8503 {
		class RenderObject;
		class GameWorld;

		class GameTechRenderer : 
			public OGLRenderer,
			public NCL::CSC8503::GameTechRendererInterface	
		{
		public:
			GameTechRenderer(GameWorld& world);
			~GameTechRenderer();

			Mesh*		LoadMesh(const std::string& name)									override;
			Texture*	LoadTexture(const std::string& name)								override;
	
		protected:

			struct ObjectSortState {
				const RenderObject* object;
				float distanceFromCamera;
			};

			// -------- Shared render pass data (UBO) --------
			struct alignas(16) PassDataCPU {
				Matrix4 viewMatrix;
				Matrix4 projMatrix;
				Matrix4 viewProjMatrix;
				Matrix4 shadowMatrix;

				Vector4 lightColour;     // rgb + intensity (or 1)
				Vector4 lightPosRadius;  // xyz + radius
				Vector4 cameraPos;       // xyz + 1
				Vector4 misc;            // time / mode / flags etc.
			};

			// -------- Shared render pass data (SSBO) --------
			struct alignas(16) ObjectDataCPU {
				NCL::Maths::Matrix4 modelMatrix;
				NCL::Maths::Vector4 colour;
				uint32_t materialID;
				uint32_t flags;
				uint32_t pad0;
				uint32_t pad1;
			};

			std::vector<ObjectDataCPU> frameObjects;

			// -------- Material data (SSBO) --------
			struct alignas(16) MaterialDataCPU {
				uint32_t flags;     // bit0: hasTexture
				uint32_t texIndex;  // C1 暂时不用
				uint32_t pad0;
				uint32_t pad1;
			};

			GLuint materialSSBO = 0;
			size_t materialSSBOCapacityBytes = 0;
			size_t materialSSBOCount = 0;

			void InitMaterialSSBO();
			void UpdateMaterialSSBO(const std::vector<MaterialDataCPU>& materials);

			GLuint   mainTexArray = 0;
			uint32_t mainTexArrayW = 0;
			uint32_t mainTexArrayH = 0;
			uint32_t mainTexArrayLayers = 0;
			void BuildMainTexArray(const std::vector<OGLTexture*>& textures, std::unordered_map<const OGLTexture*, 
				uint32_t>& outLayerMap, uint32_t& outW, uint32_t& outH);

			// -------- Pass data (UBO) --------
			void InitPassUBO();
			void UpdatePassUBO(const PassDataCPU& data);
			void UpdateScenePassData();
			GLuint passUBO = 0;

			// -------- Per-object data (SSBO) --------
			GLuint objectSSBO = 0;
			size_t objectSSBOCapacityBytes = 0;
			size_t objectSSBOCount = 0; // 当前 objects 数量（可选，用来 debug）

			void InitObjectSSBO();
			void UpdateObjectSSBO(const std::vector<ObjectDataCPU>& objects);
			void BuildAndUploadObjectTable();

			// -------- Render passes --------
			void RenderLines();
			void RenderText();
			void RenderTextures();

			void RenderFrame()	override;

			void BuildObjectLists();

			void RenderSkyboxPass();
			void RenderOpaquePass(std::vector<ObjectSortState>& list);
			void RenderTransparentPass(std::vector<ObjectSortState>& list);
			void RenderShadowMapPass(std::vector<ObjectSortState>& list);

			void LoadSkybox();

			void SetDebugStringBufferSizes(size_t newVertCount);
			void SetDebugLineBufferSizes(size_t newVertCount);

			Mesh* CreateMesh() override {
				return new OGLMesh();
			}

			void UploadMesh(Mesh* mesh) override {
				mesh->UploadToGPU(this);
			}

			std::vector<ObjectSortState> opaqueObjects;
			std::vector<ObjectSortState> transparentObjects;

			GameWorld&	gameWorld;

			OGLShader*	defaultShader;

			//Skybox pass data
			OGLShader*  skyboxShader;
			OGLMesh*	skyboxMesh;
			GLuint		skyboxTex;

			//shadow map pass data
			OGLShader*	shadowShader;
			GLuint		shadowTex;
			GLuint		shadowFBO;
			Matrix4     shadowMatrix;

			//Debug data
			OGLShader*  debugShader;
			OGLMesh*	debugTexMesh;

			std::vector<Vector3> debugLineData;

			std::vector<Vector3> debugTextPos;
			std::vector<Vector4> debugTextColours;
			std::vector<Vector2> debugTextUVs;

			GLuint lineVAO;
			GLuint lineVertVBO;
			size_t lineCount;

			GLuint textVAO;
			GLuint textVertVBO;
			GLuint textColourVBO;
			GLuint textTexVBO;
			size_t textCount;
		};
	}
}
