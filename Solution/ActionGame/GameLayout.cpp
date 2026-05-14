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
#include <UI\UILayout.h>
#include "Graphics\Text.h"
#include "Graphics\RectSprite.h"

GameLayout::GameLayout()
	: myFont(ActionGameGlobals::GetInstance().GetFont())
{
	ActionGameGlobals& globals = ActionGameGlobals::GetInstance();

	myPhysicsWorld = new Slush::PhysicsWorld();
	myEntityManager = new EntityManager();

	ActionGameGlobals::GetInstance().SetPhysicsWorld(myPhysicsWorld);
	ActionGameGlobals::GetInstance().SetEntityManager(myEntityManager);

	myLevel = new Level();

	myUIBackgroundStyle.SetPadding(16, 16);
	myUIBackgroundStyle.SetChildGap(16);
	myUIBackgroundStyle.SetColor(0xAA121212);
	myUIBackgroundStyle.SetXSizing(Slush::UIElementStyle::FIT);
	myUIBackgroundStyle.SetAlingment(Slush::UIElementStyle::CENTER);

	myUIButtonStyle.SetXSizing(Slush::UIElementStyle::FIXED, 250);
	myUIButtonStyle.SetYSizing(Slush::UIElementStyle::FIXED, 75);
	myUIButtonStyle.SetAlingment(Slush::UIElementStyle::CENTER);
	myUIButtonStyle.SetColor(0xFFAAFFAF);
	myUIButtonStyle.EnableButtonInteraction(0xFFDDDDDD);

	myUISprite = new Slush::RectSprite();
	myText = new Slush::Text(myFont);


	
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
	}
}

GameLayout::~GameLayout()
{
	FW_SAFE_DELETE(myLevel);
	FW_SAFE_DELETE(myEntityManager);
	FW_SAFE_DELETE(myPhysicsWorld);

	ActionGameGlobals::GetInstance().SetEntityManager(nullptr);
	ActionGameGlobals::GetInstance().SetPhysicsWorld(nullptr);
}

void GameLayout::Update()
{
	UpdateGameState();

	myEntityManager->EndFrame();
}

void GameLayout::Render()
{
	Slush::Engine& engine = Slush::Engine::GetInstance();
	engine.GetWindow().StartOffscreenBuffer();

	if (myLevel)
		myLevel->RenderGame();

	myEntityManager->Render();

	if (ActionGameGlobals::GetInstance().myDebugSettings.myShowPhysicsObjects)
		myPhysicsWorld->RenderAllObjects();

	if (ActionGameGlobals::GetInstance().myDebugSettings.myShowPhysicsContacts)
		myPhysicsWorld->RenderContacts();

	if (myLevel)
		myLevel->RenderUI();

	for (Slush::DynamicUIBuilder::RenderCommand& command : myUIRenderCommands)
	{
		if (command.myText.Empty())
		{
			myUISprite->SetOrigin(Slush::RectSprite::Origin::TOP_LEFT);
			myUISprite->SetPosition(command.myPosition.x, command.myPosition.y);
			myUISprite->SetSize(command.mySize.x, command.mySize.y);
			myUISprite->SetFillColor(command.myColor);
			myUISprite->Render();
		}
		else
		{
			myText->SetText(command.myText);
			myText->SetCharacterSize(command.myTextSize);
			myText->SetColor(command.myColor);
			myText->SetPosition(command.myPosition.x, command.myPosition.y);
			myText->Render();
		}
	}
	myUIRenderCommands.RemoveAll();

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

		physA->myEntity.OnCollision(physB->myEntity, contact.myContactPosition);
	}
}

void GameLayout::UpdateGameState()
{
	switch (myGameState)
	{
	case START_SCREEN:
	{
		UpdateStartScreen();
		break;
	}
	case LOADING_LEVEL:
		myLevel->Restart();
		myGameState = PLAYING;
		break;
	case PLAYING:
		UpdatePlaying();
		break;
	case GAME_OVER:
	{
		UpdateGameOver();
		break;
	}
	default:
		break;
	}
}

void GameLayout::UpdateStartScreen()
{
	if (ActionGameGlobals::GetInstance().myDebugSettings.mySkipStartScreen)
		myGameState = LOADING_LEVEL;

	Slush::DynamicUIBuilder uiBuilder;
	uiBuilder.Start();
	{
		uiBuilder.OpenElement("Title", myUIBackgroundStyle);
		uiBuilder.GetStyle().SetXSizing(Slush::UIElementStyle::GROW);
		uiBuilder.GetStyle().SetYSizing(Slush::UIElementStyle::GROW);
	
		uiBuilder.CloseElement(); // Title
	}
	uiBuilder.Finish(myUIRenderCommands);

	uiBuilder.Start();
	{
		uiBuilder.OpenElement();
		uiBuilder.GetStyle().SetChildGap(40);
		uiBuilder.GetStyle().SetLayoutDirection(Slush::UIElementStyle::TOP_TO_BOTTOM);
		uiBuilder.GetStyle().SetColor(0x00000000);

		uiBuilder.OpenElement("title");
		uiBuilder.GetStyle().SetAlingment(Slush::UIElementStyle::CENTER);
		uiBuilder.SetText("Action Game!", myFont, 50);
		uiBuilder.CloseElement();

		uiBuilder.OpenElement("StartGame", myUIButtonStyle);
		uiBuilder.GetStyle().SetColor(0xFFFF3333);

		uiBuilder.OpenElement();
		uiBuilder.SetText("Start Game", myFont, 25);
		uiBuilder.GetStyle().SetColor(0xFF000000);
		uiBuilder.CloseElement();

		uiBuilder.CloseElement();

		uiBuilder.CloseElement();
	}

	uiBuilder.Finish(myUIRenderCommands);
	

	if (uiBuilder.WasClicked("StartGame"))
	{
		myGameState = LOADING_LEVEL;
	}
}

void GameLayout::UpdatePlaying()
{
	bool pauseEntityUpdate = false;
	if (myLevel)
		pauseEntityUpdate = myLevel->IsShowingUI();

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

	if (myLevel->IsPlayerDead())
		myGameState = GAME_OVER;
}

void GameLayout::UpdateGameOver()
{
	Slush::DynamicUIBuilder uiBuilder;
	uiBuilder.Start();
	{
		uiBuilder.OpenElement("Title", myUIBackgroundStyle);
		uiBuilder.GetStyle().SetXSizing(Slush::UIElementStyle::GROW);
		uiBuilder.GetStyle().SetYSizing(Slush::UIElementStyle::GROW);

		uiBuilder.CloseElement(); // Title
	}
	uiBuilder.Finish(myUIRenderCommands);

	uiBuilder.Start();
	{
		uiBuilder.OpenElement();
		uiBuilder.GetStyle().SetChildGap(40);
		uiBuilder.GetStyle().SetLayoutDirection(Slush::UIElementStyle::TOP_TO_BOTTOM);
		uiBuilder.GetStyle().SetColor(0x00000000);

		uiBuilder.OpenElement("title");
		uiBuilder.GetStyle().SetAlingment(Slush::UIElementStyle::CENTER);
		uiBuilder.SetText("Game Over!", myFont, 50);
		uiBuilder.CloseElement();

		uiBuilder.OpenElement("RestartGame", myUIButtonStyle);
		uiBuilder.GetStyle().SetColor(0xFFFF3333);

		uiBuilder.OpenElement();
		uiBuilder.SetText("Try Again?", myFont, 25);
		uiBuilder.GetStyle().SetColor(0xFF000000);
		uiBuilder.CloseElement();

		uiBuilder.CloseElement();

		uiBuilder.CloseElement();
	}

	uiBuilder.Finish(myUIRenderCommands);


	if (uiBuilder.WasClicked("RestartGame"))
	{
		myGameState = LOADING_LEVEL;
	}
}
