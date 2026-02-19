#include "Level.h"

#include "GameWorld.h"
#include "PhysicsSystem.h"
#include "GameTechRendererInterface.h"

#include "GameObject.h"
#include "Player.h"
#include "MetalObject.h"

#include "AABBVolume.h"
#include "SphereVolume.h"
#include "PhysicsObject.h"
#include "RenderObject.h"

#include "Assimp/ModelLoader.h"
#include "Assimp/NCLAssimpMesh.h"

using namespace NCL;
using namespace NCL::Maths;
using namespace NCL::CSC8503;

void Level::EnsureAssetsLoaded() {
    if (cubeMesh && playerMesh && NPCMesh) {
        return;
    }
    if (!context.renderer) {
        return;
    }

    // Keep the same assets as MyGame used before
    cubeMesh = context.renderer->LoadMesh("cube.msh");
    playerMesh = context.renderer->LoadMesh("Characters/Meshes/MaleGuard/Male_Guard.msh");
    //NPCMesh = context.renderer->LoadMesh("Characters/Meshes/Max/Rig_Maximilian.msh");
    NPCMesh = NCL::Assets::LoadNCLMeshFromAssimp(*context.renderer, "Adelie_Tomorin/Adelie_Tomori.fbx");
    defaultTex = context.renderer->LoadTexture("checkerboard.png");

    notexMaterial = GameTechMaterial();
}

void Level::ClearWorld() {
    if (!context.world) return;

    // MyGame already calls ClearAndErase + physics.Clear before Build()
    // This is here as an optional helper if a level wants to rebuild itself
    if (context.metalObjects) {
        context.metalObjects->clear();
    }
    if (context.playerOut) {
        *context.playerOut = nullptr;
    }
    /*if (context.playerOut) {
        *context.playerOut = nullptr;
    }*/
}

GameObject* Level::AddFloorToWorld(const Vector3& position) {
    EnsureAssetsLoaded();
    if (!context.world) return nullptr;

    GameObject* floor = new GameObject("Floor");

    Vector3 floorHalfSize = Vector3(200, 1, 200);
    AABBVolume* volume = new AABBVolume(floorHalfSize);
    floor->SetBoundingVolume(volume);

    floor->GetTransform()
        .SetScale(floorHalfSize * 2.0f)
        .SetPosition(position);

    floor->SetRenderObject(new RenderObject(floor->GetTransform(), cubeMesh, notexMaterial));
    floor->GetRenderObject()->SetColour(Vector4(0.25f, 0.25f, 0.25f, 1.0f));

    floor->SetPhysicsObject(new PhysicsObject(floor->GetTransform(), floor->GetBoundingVolume()));
    floor->GetPhysicsObject()->SetInverseMass(0.0f);
    floor->GetPhysicsObject()->InitCubeInertia();

    context.world->AddGameObject(floor);
    return floor;
}

Player* Level::AddPlayerToWorld(const Vector3& position, float radius, float inverseMass) {
    EnsureAssetsLoaded();
    if (!context.world) return nullptr;

    Player* p = new Player(context.world);

    SphereVolume* volume = new SphereVolume(radius * 0.5f);
    p->SetBoundingVolume(volume);

    p->GetTransform()
        .SetScale(Vector3(radius, radius, radius))
        .SetPosition(position);

    p->SetRenderObject(new RenderObject(p->GetTransform(), playerMesh, notexMaterial));
    p->GetRenderObject()->SetColour(Vector4(0, 1, 1, 1));

    PhysicsObject* po = new PhysicsObject(p->GetTransform(), p->GetBoundingVolume());
    po->SetInverseMass(inverseMass);
    po->InitSphereInertia();
    po->SetElasticity(0.0f); // no bounciness
    p->SetPhysicsObject(po);

    context.world->AddGameObject(p);

    if (context.playerOut) {
        *context.playerOut = p;
    }

    return p;
}

MetalObject* Level::AddMetalCubeToWorld(const Vector3& position,
    const Vector3& halfDims,
    float inverseMass,
    const Vector4& colour) {

    EnsureAssetsLoaded();
    if (!context.world) return nullptr;

    // Keep your existing MetalObject default construction
    MetalObject* cube = new MetalObject();
    AABBVolume* volume = new AABBVolume(halfDims);
    cube->SetBoundingVolume(volume);

    cube->GetTransform()
        .SetPosition(position)
        .SetScale(halfDims * 2.0f);

    cube->SetRenderObject(new RenderObject(cube->GetTransform(), cubeMesh, notexMaterial));
    cube->GetRenderObject()->SetColour(colour);

    cube->SetPhysicsObject(new PhysicsObject(cube->GetTransform(), cube->GetBoundingVolume()));
    cube->GetPhysicsObject()->SetInverseMass(inverseMass);
    cube->GetPhysicsObject()->InitCubeInertia();

    context.world->AddGameObject(cube);

    if (context.metalObjects) {
        context.metalObjects->push_back(cube);
    }

    return cube;
}

bool Level::LoadDialogue(const std::string& filepath) {
    const bool ok = DialogueSystem::Get().LoadGraphFromFile(filepath);
    if (!ok) {
        std::cout << "Dialogue Failed to load: " << filepath << std::endl;
    }
    else {
		std::cout << "Dialogue Loaded: " << filepath << std::endl;
    }
    return ok;
}

DialogueNPC* Level::AddDialogueNPCToWorld(const std::string& dialogueGraphId, const NCL::Maths::Vector3& position,
    float radius, float inverseMass, const Vector4& colour, float interactRadius) {
    EnsureAssetsLoaded();
    if (!context.world) return nullptr;

	DialogueNPC* npc = new DialogueNPC(dialogueGraphId, interactRadius);
    SphereVolume* volume = new SphereVolume(radius * 0.5f);
	npc->SetBoundingVolume(volume);

    npc->GetTransform()
        .SetScale(Vector3(radius, radius, radius))
		.SetPosition(position);
	npc->SetRenderObject(new RenderObject(npc->GetTransform(), NPCMesh, notexMaterial));
	npc->GetRenderObject()->SetColour(colour);

	PhysicsObject* NPCpo = new PhysicsObject(npc->GetTransform(), npc->GetBoundingVolume());
    NPCpo->SetInverseMass(inverseMass);
    NPCpo->InitSphereInertia();
    NPCpo->SetElasticity(0.0f); // no bounciness
    npc->SetPhysicsObject(NPCpo);

	context.world->AddGameObject(npc);

    if (context.dialogueNPCs) {
        context.dialogueNPCs->push_back(npc);
    }
	return npc;
}
