#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"

#include <FW_Includes.h>

#include "Core/IApp.h"
#include "Core/Engine.h"
#include "Core/Input.h"
#include "Core/Time.h"
#include "Core/AssetStorage.h"

#include "Core/Dockables/GameViewDockable.h"
#include "Core/Dockables/TextureViewerDockable.h"
#include "Core/Dockables/LogDockable.h"

#include "Graphics/Window.h"
#include "Graphics/Texture.h"
#include "Graphics/Font.h"
#include "Graphics/CircleSprite.h"
#include "Graphics/RectSprite.h"
#include "Graphics/Animation/Animation.h"

#include "Physics/PhysicsWorld.h"
#include "Physics/PhysicsShapes.h"

#include "Entity.h"
#include "EntityManager.h"
#include "EntityPrefab.h"

#include "ProjectileManager.h"

#include "NPCWave.h"
#include "PhysicsComponent.h"
#include "EntityPrefabDockable.h"

class App : public Slush::IApp
{
public:
	void Initialize() override
	{
		FW_GrowingArray<FW_FileSystem::FileInfo> textureInfos;
		FW_FileSystem::GetAllFilesFromRelativeDirectory("Data/Textures", textureInfos);

		for (const FW_FileSystem::FileInfo& info : textureInfos)
			myTextures.Load(info.myFileNameNoExtention.GetBuffer(), info.myRelativeFilePath.GetBuffer());

		myFont.Load("Data/OpenSans-Regular.ttf");
		CreatePrefabs();

		myPhysicsWorld = new Slush::PhysicsWorld();
		myEntityManager = new EntityManager(myEntityPrefabs);

		myProjectileManager = new ProjectileManager(*myEntityManager, *myPhysicsWorld);
		myNPCWave = new NPCWave(*myEntityManager, *myProjectileManager, *myPhysicsWorld);

		Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
		window.AddDockable(new Slush::GameViewDockable());
		window.AddDockable(new Slush::TextureViewerDockable(myTextures));
		window.AddDockable(new Slush::LogDockable());
		window.AddDockable(new EntityPrefabDockable(myEntityPrefabs));

		myEntityManager->CreateEntity({ 500.f, 800.f }, "Wall", *myPhysicsWorld, *myProjectileManager);
	}

	void Shutdown() override
	{
		FW_SAFE_DELETE(myNPCWave);
		FW_SAFE_DELETE(myProjectileManager);
		FW_SAFE_DELETE(myEntityManager);
		FW_SAFE_DELETE(myPhysicsWorld);
	}

	void Update() override
	{
		if (!myPlayer.IsValid())
			CreatePlayer();

		myEntityManager->PrePhysicsUpdate();

		myPhysicsWorld->TickLimited(Slush::Time::GetDelta());

		const FW_GrowingArray<Slush::Manifold>& contacts = myPhysicsWorld->GetContacts();
		for (const Slush::Manifold& contact : contacts)
		{
			if (!contact.myObjectA || !contact.myObjectB)
				continue;
		
			PhysicsComponent* physA = contact.myObjectA->myUserData.Get<PhysicsComponent*>();
			PhysicsComponent* physB = contact.myObjectB->myUserData.Get<PhysicsComponent*>();
			if (!physA || !physB)
			{
				SLUSH_WARNING("PhysContact with Entity without PhysicsComponent");
				continue;
			}

			if (physA->myEntity.myType == physB->myEntity.myType)
			{
				SLUSH_WARNING("PhysContact between entities of the same type");
				continue;
			}
		
			physA->myEntity.OnCollision(physB->myEntity);
			physB->myEntity.OnCollision(physA->myEntity);
		}

		myNPCWave->Update();
		myEntityManager->Update();

		myEntityManager->EndFrame();
	}

	void Render() override
	{
		Slush::Engine& engine = Slush::Engine::GetInstance();
		engine.GetWindow().StartOffscreenBuffer();

		myEntityManager->Render();

		engine.GetWindow().EndOffscreenBuffer();
	}

	void CreatePlayer()
	{
		Entity* player = myEntityManager->CreateEntity({ 400.f, 400.f }, "Player", *myPhysicsWorld, *myProjectileManager);
		myPlayer = player->myHandle;
		myNPCWave->SetPlayerHandle(myPlayer);
	}

	void CreatePrefabs()
	{
		EntityPrefab& playerPrefab = myEntityPrefabs.CreateNewAsset("Player");
		playerPrefab.myEntityType = Entity::PLAYER;
		playerPrefab.mySprite.myEnabled = true;
		playerPrefab.mySprite.myRadius = 20.f;
		playerPrefab.mySprite.myColor = 0xFFFF0000;
		FW_ARGB_To_RGBAFloat(playerPrefab.mySprite.myColor, playerPrefab.mySprite.myFloatColor);
		playerPrefab.myAnimation.myEnabled = true;
		playerPrefab.myProjectileShooting.myEnabled = true;
		playerPrefab.myProjectileShooting.myCooldown = 0.1f;
		playerPrefab.myPlayerController.myEnabled = true;
		playerPrefab.myHealth.myEnabled = true;
		playerPrefab.myHealth.myMaxHealth = 3;
		playerPrefab.myPhysics.myEnabled = true;
		playerPrefab.myPhysics.myMatchSprite = true;

		EntityPrefab& npcPrefab = myEntityPrefabs.CreateNewAsset("NPC");
		npcPrefab.myEntityType = Entity::NPC;
		npcPrefab.mySprite.myEnabled = true;
		npcPrefab.mySprite.myRadius = 20.f;
		npcPrefab.mySprite.myColor = 0xFF0000FF;
		FW_ARGB_To_RGBAFloat(npcPrefab.mySprite.myColor, npcPrefab.mySprite.myFloatColor);
		npcPrefab.myAnimation.myEnabled = true;
		npcPrefab.myProjectileShooting.myEnabled = true;
		npcPrefab.myProjectileShooting.myCooldown = 1.f;
		npcPrefab.myNPCController.myEnabled = true;
		npcPrefab.myHealth.myEnabled = true;
		npcPrefab.myHealth.myMaxHealth = 5;
		npcPrefab.myPhysics.myEnabled = true;
		npcPrefab.myPhysics.myMatchSprite = true;

		EntityPrefab& npcProjectilePrefab = myEntityPrefabs.CreateNewAsset("NPCProjectile");
		npcProjectilePrefab.myEntityType = Entity::NPC;
		npcProjectilePrefab.mySprite.myEnabled = true;
		npcProjectilePrefab.mySprite.myRadius = 5.f;
		npcProjectilePrefab.mySprite.myColor = 0xFFFF0000;
		FW_ARGB_To_RGBAFloat(npcProjectilePrefab.mySprite.myColor, npcProjectilePrefab.mySprite.myFloatColor);
		npcProjectilePrefab.myPhysics.myEnabled = true;
		npcProjectilePrefab.myPhysics.myMatchSprite = true;
		npcProjectilePrefab.myRemoveOnCollision.myEnabled = true;

		EntityPrefab& playerProjectilePrefab = myEntityPrefabs.CreateNewAsset("PlayerProjectile");
		playerProjectilePrefab.myEntityType = Entity::PLAYER;
		playerProjectilePrefab.mySprite.myEnabled = true;
		playerProjectilePrefab.mySprite.myRadius = 5.f;
		playerProjectilePrefab.mySprite.myColor = 0xFFFF0000;
		FW_ARGB_To_RGBAFloat(playerProjectilePrefab.mySprite.myColor, playerProjectilePrefab.mySprite.myFloatColor);
		playerProjectilePrefab.myPhysics.myEnabled = true;
		playerProjectilePrefab.myPhysics.myMatchSprite = true;
		playerProjectilePrefab.myRemoveOnCollision.myEnabled = true;

		EntityPrefab& wallPrefab = myEntityPrefabs.CreateNewAsset("Wall");
		wallPrefab.myEntityType = Entity::ENVIRONMENT;
		wallPrefab.mySprite.myEnabled = true;
		wallPrefab.mySprite.mySize = { 1000.f, 100.f };
		wallPrefab.mySprite.myColor = 0xFFFFFF00;
		FW_ARGB_To_RGBAFloat(wallPrefab.mySprite.myColor, wallPrefab.mySprite.myFloatColor);
		wallPrefab.myPhysics.myEnabled = true;
		wallPrefab.myPhysics.myStatic = true;
		wallPrefab.myPhysics.myMatchSprite = true;
	}

private:
	Slush::Font myFont;
	Slush::AssetStorage<Slush::Texture> myTextures;
	Slush::AssetStorage<EntityPrefab> myEntityPrefabs;

	EntityHandle myPlayer;
	EntityManager* myEntityManager;
	Slush::PhysicsWorld* myPhysicsWorld;

	NPCWave* myNPCWave;
	ProjectileManager* myProjectileManager;
};

#include <FW_UnitTestSuite.h>
int main()
{
	FW_UnitTestSuite::RunTests();

	Slush::Engine& engine = Slush::Engine::GetInstance();
	engine.Initialize();

	App app;
	engine.Run(app);

	engine.Destroy();
	
	return 0;
}