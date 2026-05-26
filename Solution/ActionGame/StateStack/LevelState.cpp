#include "stdafx.h"

#include "ActionGameGlobals.h"
#include "LevelState.h"
#include "GameOverState.h"

#include "Physics\PhysicsWorld.h"
#include "EntitySystem\EntityManager.h"
#include "Level\Level.h"
#include "StateStack.h"
#include "Graphics\Window.h"
#include "PauseState.h"
#include "Core\Input.h"

LevelState::LevelState(const CharacterInfo& aCharacterInfo)
{
	myPhysicsWorld = new Slush::PhysicsWorld();
	myEntityManager = new EntityManager();

	ActionGameGlobals& globals = ActionGameGlobals::GetInstance();
	ActionGameGlobals::GetInstance().SetPhysicsWorld(myPhysicsWorld);
	ActionGameGlobals::GetInstance().SetEntityManager(myEntityManager);

	myLevel = new Level(aCharacterInfo);
	myLevel->Restart();

	Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
	window.StartFade(0.15f);
}

LevelState::~LevelState()
{
	FW_SAFE_DELETE(myLevel);

	ActionGameGlobals::GetInstance().SetPhysicsWorld(nullptr);
	ActionGameGlobals::GetInstance().SetEntityManager(nullptr);
}

void LevelState::ResumeState()
{
	Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
	window.StartFade(0.15f);
}

GameState::GameStateResult LevelState::Update()
{
	myEntityManager->PrePhysicsUpdate();
	myPhysicsWorld->TickLimited(Slush::Time::GetDelta());
	UpdateCollisions();
	myEntityManager->Update();

	myLevel->Update(*myStateStack);
	if (myLevel->IsPlayerDead())
		myStateStack->PushSubState(new GameOverState(*myLevel));

	Slush::Engine& engine = Slush::Engine::GetInstance();
	const Slush::Input& input = engine.GetInput();
	if (input.WasKeyReleased(Slush::Input::ESC))
	{
		myStateStack->PushSubState(new PauseState(myLevel->GetPlayer()));
	}

	myEntityManager->EndFrame();
	return GameState::KEEP;
}

void LevelState::Render()
{
	myLevel->RenderGame();
	myEntityManager->Render();
	
	if (ActionGameGlobals::GetInstance().myDebugSettings.myShowPhysicsObjects)
		myPhysicsWorld->RenderAllObjects();
	
	if (ActionGameGlobals::GetInstance().myDebugSettings.myShowPhysicsContacts)
		myPhysicsWorld->RenderContacts();
}

void LevelState::UpdateCollisions()
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
