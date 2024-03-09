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

#include "Entity.h"
#include "EntityManager.h"

#include "ProjectileManager.h"
#include "SpriteComponent.h"
#include "AnimationComponent.h"
#include "ProjectileShootingComponent.h"
#include "PlayerControllerComponent.h"
#include "NPCControllerComponent.h"
#include "HealthComponent.h"
#include "PhysicsComponent.h"

#include "Physics/PhysicsWorld.h"
#include "Physics/PhysicsShapes.h"

#include "NPCWave.h"

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

		myEntityManager = new EntityManager();
		myPhysicsWorld = new Slush::PhysicsWorld();

		myProjectileManager = new ProjectileManager(*myEntityManager, *myPhysicsWorld);
		myNPCWave = new NPCWave(*myEntityManager, *myProjectileManager, *myPhysicsWorld);

		Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
		window.AddDockable(new Slush::GameViewDockable());
		window.AddDockable(new Slush::TextureViewerDockable(myTextures));
		window.AddDockable(new Slush::LogDockable());

		Vector2f rectSize = { 1000.f, 100.f };
		Entity* wall = myEntityManager->CreateEntity();
		wall->myPosition = { 500.f, 800.f };
		wall->mySpriteComponent = new SpriteComponent(*wall);
		wall->mySpriteComponent->MakeRect(rectSize.x, rectSize.y, 0xFFFFFF00);
		wall->myPhysicsComponent = new PhysicsComponent(*wall, *myPhysicsWorld);
		wall->myPhysicsComponent->myObject = new Slush::PhysicsObject(new Slush::AABBShape(rectSize));
		wall->myPhysicsComponent->myObject->SetPosition(wall->myPosition);
		wall->myPhysicsComponent->myObject->MakeStatic();
		wall->myPhysicsComponent->myObject->myUserData.Set(wall->myPhysicsComponent);
		myPhysicsWorld->AddObject(wall->myPhysicsComponent->myObject);
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
		Entity* player = myEntityManager->CreateEntity();
		myPlayer = player->myHandle;
		myNPCWave->SetPlayerHandle(myPlayer);

		player->myType = Entity::PLAYER;
		player->myPosition = { 400.f, 400.f };
		player->mySpriteComponent = new SpriteComponent(*player);
		player->mySpriteComponent->MakeCircle(20.f, 0xFFFF0000);
		player->myAnimationComponent = new AnimationComponent(*player);
		player->myProjectileShootingComponent = new ProjectileShootingComponent(*player, *myProjectileManager);
		player->myProjectileShootingComponent->SetCooldown(0.1f);
		player->myPlayerControllerComponent = new PlayerControllerComponent(*player);
		player->myHealthComponent = new HealthComponent(*player);
		player->myHealthComponent->SetMaxHealth(3);
		player->myPhysicsComponent = new PhysicsComponent(*player, *myPhysicsWorld);
		player->myPhysicsComponent->myObject = new Slush::PhysicsObject(new Slush::CircleShape(20.f));
		player->myPhysicsComponent->myObject->SetPosition(player->myPosition);
		player->myPhysicsComponent->myObject->myUserData.Set(player->myPhysicsComponent);
		myPhysicsWorld->AddObject(player->myPhysicsComponent->myObject);
	}

private:
	Slush::Font myFont;
	Slush::AssetStorage<Slush::Texture> myTextures;

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