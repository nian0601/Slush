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
		myEntityManager = new EntityManager(myEntityPrefabs, *myPhysicsWorld);

		myNPCWave = new NPCWave(*myEntityManager, *myPhysicsWorld);

		Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
		window.AddDockable(new Slush::GameViewDockable());
		window.AddDockable(new Slush::TextureViewerDockable(myTextures));
		window.AddDockable(new Slush::LogDockable());
		window.AddDockable(new EntityPrefabDockable(myEntityPrefabs));

		myEntityManager->CreateEntity({ 500.f, 800.f }, "Wall");
	}

	void Shutdown() override
	{
		FW_SAFE_DELETE(myNPCWave);
		FW_SAFE_DELETE(myEntityManager);
		FW_SAFE_DELETE(myPhysicsWorld);
	}

	void Update() override
	{
		if (!myPlayer.IsValid())
			CreatePlayer();

		myEntityManager->PrePhysicsUpdate();

		myPhysicsWorld->TickLimited(Slush::Time::GetDelta());

		UpdatePhysics();

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

	void UpdatePhysics()
	{
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
	}

	void CreatePlayer()
	{
		Entity* player = myEntityManager->CreateEntity({ 400.f, 400.f }, "Player");
		myPlayer = player->myHandle;
		myNPCWave->SetPlayerHandle(myPlayer);
	}

	void CreatePrefabs()
	{
		FW_GrowingArray<FW_FileSystem::FileInfo> prefabInfos;
		FW_FileSystem::GetAllFilesFromRelativeDirectory("Data/EntityPrefabs", prefabInfos);
		
		for (const FW_FileSystem::FileInfo& info : prefabInfos)
			myEntityPrefabs.Load(info.myFileNameNoExtention.GetBuffer(), info.myRelativeFilePath.GetBuffer());
	}

private:
	Slush::Font myFont;
	Slush::AssetStorage<Slush::Texture> myTextures;
	Slush::AssetStorage<EntityPrefab> myEntityPrefabs;

	EntityHandle myPlayer;
	EntityManager* myEntityManager;
	Slush::PhysicsWorld* myPhysicsWorld;

	NPCWave* myNPCWave;
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