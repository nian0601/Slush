#include "stdafx.h"

#include "Core/IApp.h"
#include "Core/Engine.h"
#include "Core/Input.h"
#include "Core/Time.h"
#include "Core/Assets/AssetStorage.h"

#include "Core/Dockables/GameViewDockable.h"
#include "Core/Dockables/TextureViewerDockable.h"
#include "Core/Dockables/LogDockable.h"
#include "Core/Dockables/ContentBrowserDockable.h"

#include "Graphics/Window.h"
#include "Graphics/Texture.h"
#include "Graphics/Font.h"
#include "Graphics/CircleSprite.h"
#include "Graphics/RectSprite.h"
#include "Graphics/Animation/Animation.h"

#include "UI/UILayout.h"
#include "UI/UIManager.h"
#include "UI/UIButton.h"
#include "UI/UIEditorDockable.h"

#include "Physics/PhysicsWorld.h"
#include "Physics/PhysicsShapes.h"

#include "Entity.h"
#include "EntityManager.h"
#include "EntityPrefab.h"

#include "NPCWave.h"
#include "PhysicsComponent.h"
#include "EntityPrefabDockable.h"

#include "Level.h"

#include "ActionGameGlobals.h"

class App : public Slush::IApp
{
public:
	enum GameState
	{
		START_SCREEN,
		LOADING_LEVEL,
		PLAYING,
		GAME_OVER,
	};

	void Initialize() override
	{
		myPhysicsWorld = new Slush::PhysicsWorld();
		myEntityManager = new EntityManager(myEntityPrefabs);

		ActionGameGlobals::GetInstance().SetEntityManager(myEntityManager);
		ActionGameGlobals::GetInstance().SetPhysicsWorld(myPhysicsWorld);
		ActionGameGlobals::GetInstance().SetTextureStorage(myTextures);
		ActionGameGlobals::GetInstance().SetEntityPrefabStorage(myEntityPrefabs);

		myFont.Load("Data/OpenSans-Regular.ttf");

		myTextures.LoadAllAssets();
		myEntityPrefabs.LoadAllAssets();
		myUILayouts.LoadAllAssets();

		Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
		window.AddDockable(new Slush::GameViewDockable());
		window.AddDockable(new Slush::TextureViewerDockable(myTextures));
		window.AddDockable(new Slush::LogDockable());
		window.AddDockable(new Slush::UIEditorDockable(myUILayouts));
		window.AddDockable(new EntityPrefabDockable(myEntityPrefabs));
		window.AddDockable(new ActionGameGlobals::DebugSettingsDockable());

		Slush::ContentBrowserDockable* contentBrowser = new Slush::ContentBrowserDockable();
		window.AddDockable(contentBrowser);

		contentBrowser->AddAssetStorage(&myEntityPrefabs);
		contentBrowser->AddAssetStorage(&myTextures);
		contentBrowser->AddAssetStorage(&myUILayouts);

		myStartGameUIManager = new Slush::UIManager(myFont);
		myStartGameUIManager->SetLayout(myUILayouts.GetAsset("StartGame"));

		if (Slush::UIWidget* button = myStartGameUIManager->FindWidget("StartGame"))
			myStartGameButton = static_cast<Slush::UIButton*>(button);


		myRestartGameUIManager = new Slush::UIManager(myFont);
		myRestartGameUIManager->SetLayout(myUILayouts.GetAsset("GameOver"));

		if (Slush::UIWidget* button = myRestartGameUIManager->FindWidget("RestartGame"))
			myRestartGameButton = static_cast<Slush::UIButton*>(button);
	}

	void Shutdown() override
	{
		FW_SAFE_DELETE(myLevel);
		FW_SAFE_DELETE(myEntityManager);
		FW_SAFE_DELETE(myPhysicsWorld);
		FW_SAFE_DELETE(myStartGameUIManager);
		FW_SAFE_DELETE(myRestartGameUIManager);

		ActionGameGlobals::Destroy();
	}

	void Update() override
	{
		bool pauseEntityUpdate = false;
		if (myLevel)
			pauseEntityUpdate = myLevel->IsLevelingUp();

		UpdateGameState();

		if (!pauseEntityUpdate)
		{
			myEntityManager->PrePhysicsUpdate();
			myPhysicsWorld->TickLimited(Slush::Time::GetDelta());
			UpdatePhysics();
		}

		if (myLevel)
			myLevel->Update();

		if (!pauseEntityUpdate)
			myEntityManager->Update();

		myEntityManager->EndFrame();
	}

	void Render() override
	{
		Slush::Engine& engine = Slush::Engine::GetInstance();
		engine.GetWindow().StartOffscreenBuffer();

		if (myLevel)
			myLevel->RenderGame();

		myEntityManager->Render();

		switch (myGameState)
		{
		case App::START_SCREEN:
			myStartGameUIManager->Render();
			break;
		case App::GAME_OVER:
			myRestartGameUIManager->Render();
			break;
		}

		if (myLevel)
			myLevel->RenderUI();

		engine.GetWindow().EndOffscreenBuffer();
	}

	void UpdatePhysics()
	{
		const FW_GrowingArray<Slush::Contact>& contacts = myPhysicsWorld->GetContacts();
		for (const Slush::Contact& contact : contacts)
		{
			if (!contact.myFirst|| !contact.mySecond)
				continue;

			PhysicsComponent* physA = contact.myFirst->myUserData.Get<PhysicsComponent*>();
			PhysicsComponent* physB = contact.mySecond->myUserData.Get<PhysicsComponent*>();
			if (!physA || !physB)
			{
				SLUSH_WARNING("PhysContact with Entity without PhysicsComponent");
				continue;
			}

			physA->myEntity.OnCollision(physB->myEntity);
		}
	}

	void UpdateGameState()
	{
		switch (myGameState)
		{
		case App::START_SCREEN:
		{
			if (ActionGameGlobals::GetInstance().myDebugSettings.mySkipStartScreen)
				myGameState = LOADING_LEVEL;

			Slush::Engine& engine = Slush::Engine::GetInstance();
			myStartGameUIManager->Update(engine.GetInput());

			if (myStartGameButton && myStartGameButton->WasPressed())
				myGameState = LOADING_LEVEL;

			break;
		}
		case App::LOADING_LEVEL:
			myLevel = new Level(myFont);
			myGameState = PLAYING;
			break;
		case App::PLAYING:
			if (myLevel->IsPlayerDead())
			{
				FW_SAFE_DELETE(myLevel);
				myGameState = GAME_OVER;
			}
			break;
		case App::GAME_OVER:
		{
			Slush::Engine& engine = Slush::Engine::GetInstance();
			myRestartGameUIManager->Update(engine.GetInput());
			if (myRestartGameButton && myRestartGameButton->WasPressed())
				myGameState = LOADING_LEVEL;

			break;
		}
		default:
			break;
		}
	}

private:
	Slush::Font myFont;
	Slush::AssetStorage<Slush::Texture> myTextures;
	Slush::AssetStorage<EntityPrefab> myEntityPrefabs;
	Slush::AssetStorage<Slush::UILayout> myUILayouts;

	EntityManager* myEntityManager;
	Slush::PhysicsWorld* myPhysicsWorld;

	Level* myLevel;
	GameState myGameState = START_SCREEN;

	Slush::UIButton* myStartGameButton;
	Slush::UIManager* myStartGameUIManager;

	Slush::UIButton* myRestartGameButton;
	Slush::UIManager* myRestartGameUIManager;
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