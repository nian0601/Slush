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
#include "ProjectileManager.h"
#include "SpriteComponent.h"
#include "AnimationComponent.h"
#include "ProjectileShootingComponent.h"
#include "PlayerControllerComponent.h"
#include "NPCControllerComponent.h"
#include "CollisionComponent.h"
#include "HealthComponent.h"

#include "Physics/PhysicsWorld.h"
#include "Physics/PhysicsShapes.h"

#include "NPCWave.h"
#include "PhysicsComponent.h"

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

		myPlayer = new Entity();
		myPlayer->myType = Entity::PLAYER;
		myPlayer->myPosition = { 400.f, 400.f };
		myPlayer->mySpriteComponent = new SpriteComponent(*myPlayer);
		myPlayer->mySpriteComponent->MakeCircle(20.f, 0xFFFF0000);
		myPlayer->myAnimationComponent = new AnimationComponent(*myPlayer);
		myPlayer->myProjectileShootingComponent = new ProjectileShootingComponent(*myPlayer, myProjectileManager);
		myPlayer->myProjectileShootingComponent->SetCooldown(0.1f);
		myPlayer->myPlayerControllerComponent = new PlayerControllerComponent(*myPlayer);
		myPlayer->myCollisionComponent = new CollisionComponent(*myPlayer);
		myPlayer->myCollisionComponent->SetSize(20.f);
		myPlayer->myHealthComponent = new HealthComponent(*myPlayer);
		myPlayer->myHealthComponent->SetMaxHealth(3);
		myPlayer->myPhysicsComponent = new PhysicsComponent(*myPlayer);
		Slush::PhysicsObject* phys = new Slush::PhysicsObject(new Slush::CircleShape(20.f));
		phys->SetMass(0.1f);
		phys->SetPosition(myPlayer->myPosition);
		myPhysicsWorld.AddObject(phys);
		myPlayer->myPhysicsComponent->myObject = phys;



		myNPCWave = new NPCWave(*myPlayer, myProjectileManager);

		Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
		window.AddDockable(new Slush::GameViewDockable());
		window.AddDockable(new Slush::TextureViewerDockable(myTextures));
		window.AddDockable(new Slush::LogDockable());

		Vector2f rectSize = { 1000.f, 100.f };
		myStaticShape = new Slush::RectSprite();
		myStaticShape->SetSize(rectSize.x, rectSize.y);
		myStaticObject = new Slush::PhysicsObject(new Slush::AABBShape(rectSize));
		myPhysicsWorld.AddObject(myStaticObject);
		myStaticObject->MakeStatic();
		myStaticObject->SetPosition({ 500.f, 800.f });
	}

	void Shutdown() override
	{
		FW_SAFE_DELETE(myPlayer);
		FW_SAFE_DELETE(myNPCWave);
		FW_SAFE_DELETE(myStaticShape);
	}

	void Update() override
	{
		myPlayer->PrePhysicsUpdate();
		myNPCWave->PrePhysicsUpdate();

		myPhysicsWorld.TickLimited(Slush::Time::GetDelta());

		myPlayer->Update();
		myNPCWave->Update();
		myProjectileManager.Update();
		myProjectileManager.CheckCollisionsWithEntity(*myPlayer);

		if (HealthComponent* health = myPlayer->myHealthComponent)
		{
			if (health->IsDead())
			{
				health->SetMaxHealth(3);
				myPlayer->myPosition = { 400.f, 400.f };
				if (myPlayer->myPhysicsComponent)
					myPlayer->myPhysicsComponent->myObject->SetPosition(myPlayer->myPosition);
			}
		}
	}

	void Render() override
	{
		Slush::Engine& engine = Slush::Engine::GetInstance();
		engine.GetWindow().StartOffscreenBuffer();

		myPlayer->Render();
		myNPCWave->Render();
		myProjectileManager.Render();

		myStaticShape->Render(myStaticObject->myPosition.x - 500.f, myStaticObject->myPosition.y - 50.f);

		engine.GetWindow().EndOffscreenBuffer();
	}

private:
	Slush::Font myFont;
	Slush::AssetStorage<Slush::Texture> myTextures;

	Entity* myPlayer;

	NPCWave* myNPCWave;

	ProjectileManager myProjectileManager;

	Slush::PhysicsWorld myPhysicsWorld;
	Slush::PhysicsObject* myStaticObject;
	Slush::RectSprite* myStaticShape;
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