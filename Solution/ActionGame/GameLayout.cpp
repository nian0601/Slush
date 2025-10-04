#include "stdafx.h"

#include <Graphics\Window.h>
#include <Physics\PhysicsWorld.h>
#include <UI\UIButton.h>
#include <UI\UIManager.h>

#include "Core/Dockables/GameViewDockable.h"
#include "Core/Dockables/TextureViewerDockable.h"
#include "Core/Dockables/LogDockable.h"
#include "Core/Dockables/ContentBrowserDockable.h"

#include "EntitySystem/EntityManager.h"

#include "ActionGameGlobals.h"
#include "Level/Level.h"
#include "Level/LevelData.h"
#include "GameLayout.h"

GameLayout::GameLayout()
{
	ActionGameGlobals& globals = ActionGameGlobals::GetInstance();

	myPhysicsWorld = new Slush::PhysicsWorld();
	myEntityManager = new EntityManager(globals.GetEntityPrefabStorage());

	ActionGameGlobals::GetInstance().SetPhysicsWorld(myPhysicsWorld);
	ActionGameGlobals::GetInstance().SetEntityManager(myEntityManager);

	myStartGameUIManager = new Slush::UIManager(globals.GetFont());
	myStartGameUIManager->SetLayout(globals.GetUILayoutStorage().GetAsset("StartGame"));

	if (Slush::UIWidget* button = myStartGameUIManager->FindWidget("StartGame"))
		myStartGameButton = static_cast<Slush::UIButton*>(button);

	myRestartGameUIManager = new Slush::UIManager(globals.GetFont());
	myRestartGameUIManager->SetLayout(globals.GetUILayoutStorage().GetAsset("GameOver"));

	if (Slush::UIWidget* button = myRestartGameUIManager->FindWidget("RestartGame"))
		myRestartGameButton = static_cast<Slush::UIButton*>(button);


	bool disableEditorStuff = false;
	Slush::Window& window = Slush::Engine::GetInstance().GetWindow();

	if (disableEditorStuff)
	{
		window.ToggleEditorUI();
	}
	else
	{
		window.AddDockable(new Slush::GameViewDockable());
		window.AddDockable(new Slush::LogDockable());
		window.AddDockable(new ActionGameGlobals::DebugSettingsDockable());

		//Slush::ContentBrowserDockable* contentBrowser = new Slush::ContentBrowserDockable();
		//window.AddDockable(contentBrowser);
		//
		//contentBrowser->AddAssetStorage(&globals.GetEntityPrefabStorage());
		//contentBrowser->AddAssetStorage(&globals.GetTextureStorage());
		//contentBrowser->AddAssetStorage(&globals.GetUILayoutStorage());
	}
}

GameLayout::~GameLayout()
{
	FW_SAFE_DELETE(myLevel);
	FW_SAFE_DELETE(myEntityManager);
	FW_SAFE_DELETE(myPhysicsWorld);
	FW_SAFE_DELETE(myStartGameUIManager);
	FW_SAFE_DELETE(myRestartGameUIManager);

	ActionGameGlobals::GetInstance().SetEntityManager(nullptr);
	ActionGameGlobals::GetInstance().SetPhysicsWorld(nullptr);
}

void GameLayout::Update()
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

void GameLayout::Render()
{
	Slush::Engine& engine = Slush::Engine::GetInstance();
	engine.GetWindow().StartOffscreenBuffer();

	if (myLevel)
		myLevel->RenderGame();

	myEntityManager->Render();

	switch (myGameState)
	{
	case START_SCREEN:
		myStartGameUIManager->Render();
		break;
	case GAME_OVER:
		myRestartGameUIManager->Render();
		break;
	}

	if (myLevel)
		myLevel->RenderUI();

	engine.GetWindow().EndOffscreenBuffer();
}

void GameLayout::UpdatePhysics()
{
	const FW_GrowingArray<Slush::Contact>& contacts = myPhysicsWorld->GetContacts();
	for (const Slush::Contact& contact : contacts)
	{
		if (!contact.myFirst || !contact.mySecond)
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

void GameLayout::UpdateGameState()
{
	switch (myGameState)
	{
	case START_SCREEN:
	{
		if (ActionGameGlobals::GetInstance().myDebugSettings.mySkipStartScreen)
			myGameState = LOADING_LEVEL;

		Slush::Engine& engine = Slush::Engine::GetInstance();
		myStartGameUIManager->Update(engine.GetInput());

		if (myStartGameButton && myStartGameButton->WasPressed())
			myGameState = LOADING_LEVEL;

		break;
	}
	case LOADING_LEVEL:
		myLevel = new Level();
		myGameState = PLAYING;
		break;
	case PLAYING:
		if (myLevel->IsPlayerDead())
		{
			FW_SAFE_DELETE(myLevel);
			myGameState = GAME_OVER;
		}
		break;
	case GAME_OVER:
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