#include "stdafx.h"

#include "Core/IApp.h"
#include "Core/Engine.h"
#include "Core/Input.h"
#include "Core/Time.h"
#include "Core/Assets/AssetStorage.h"

#include "Core/Dockables/GameViewDockable.h"
#include "Core/Dockables/TextureViewerDockable.h"
#include "Core/Dockables/LogDockable.h"

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
		FW_GrowingArray<FW_FileSystem::FileInfo> textureInfos;
		FW_FileSystem::GetAllFilesFromRelativeDirectory("Data/Textures", textureInfos);

		for (const FW_FileSystem::FileInfo& info : textureInfos)
			myTextures.Load(info.myFileNameNoExtention.GetBuffer(), info.myRelativeFilePath.GetBuffer());

		myFont.Load("Data/OpenSans-Regular.ttf");
		myEntityPrefabs.LoadAllAssets();
		myUILayouts.LoadAllAssets();

		myPhysicsWorld = new Slush::PhysicsWorld();
		myEntityManager = new EntityManager(myEntityPrefabs, *myPhysicsWorld);

		Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
		window.AddDockable(new Slush::GameViewDockable());
		window.AddDockable(new Slush::TextureViewerDockable(myTextures));
		window.AddDockable(new Slush::LogDockable());
		window.AddDockable(new EntityPrefabDockable(myEntityPrefabs));
		window.AddDockable(new Slush::UIEditorDockable(myUILayouts));

		myStartGameUILayout = myUILayouts.GetAsset("StartGame");

		myUIManager = new Slush::UIManager(myFont);
		myUIManager->SetLayout(myStartGameUILayout);

		if (Slush::UIWidget* button = myUIManager->FindWidget("StartGame"))
			myStartGameButton = static_cast<Slush::UIButton*>(button);
	}

	void Shutdown() override
	{
		FW_SAFE_DELETE(myLevel);
		FW_SAFE_DELETE(myEntityManager);
		FW_SAFE_DELETE(myPhysicsWorld);
		FW_SAFE_DELETE(myUIManager);
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

		myEntityManager->Render();

		switch (myGameState)
		{
		case App::START_SCREEN:
		{
			myUIManager->Render();
		}
		}
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
			Slush::Engine& engine = Slush::Engine::GetInstance();
			myUIManager->Update(engine.GetInput());

			if (myStartGameButton->WasPressed())
				myGameState = LOADING_LEVEL;

			break;
		}
		case App::LOADING_LEVEL:
			myLevel = new Level(*myEntityManager, *myPhysicsWorld);
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
			if (ImGui::Button("Restart Game"))
				myGameState = LOADING_LEVEL;
			break;
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
	Slush::UILayout* myStartGameUILayout;
	Slush::UIManager* myUIManager;
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