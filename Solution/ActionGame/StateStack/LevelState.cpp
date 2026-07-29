#include "stdafx.h"

#include "ActionGameGlobals.h"
#include "LevelState.h"
#include "GameOverState.h"

#include "Physics\PhysicsWorld.h"
#include "EntitySystem\Components\PhysicsComponent.h"
#include "EntitySystem\EntityManager.h"
#include "Level\Level.h"
#include "StateStack/StateStack.h"
#include "Graphics\Window.h"
#include "Graphics\Renderer.h"
#include "PauseState.h"
#include "Core\Input.h"

LevelState::LevelState(const CharacterInfo& aCharacterInfo)
	: myCharacterInfo(aCharacterInfo)
{
	myPhysicsWorld = new Slush::PhysicsWorld();
	myEntityManager = new Slush::EntityManager();

	ActionGameGlobals& globals = ActionGameGlobals::GetInstance();
	myEntityManager->SetPhysicsWorld(myPhysicsWorld);
	ActionGameGlobals::GetInstance().SetEntityManager(myEntityManager);

	myLevel = new Level(aCharacterInfo);
	myLevel->Restart();

	Slush::Engine::GetInstance().GetWindow().GetRenderer().StartFade(0.15f);
}

LevelState::~LevelState()
{
	FW_SAFE_DELETE(myLevel);

	ActionGameGlobals::GetInstance().SetEntityManager(nullptr);
}

void LevelState::ResumeState()
{
	Slush::Engine::GetInstance().GetWindow().GetRenderer().StartFade(0.15f);
}

Slush::IGameState::GameStateResult LevelState::Update()
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
		myStateStack->PushSubState(new PauseState(myLevel->GetPlayer(), myCharacterInfo));
	}

	myEntityManager->EndFrame();
	return Slush::IGameState::KEEP;
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

		Slush::PhysicsComponent* physA = contact.myFirst->myUserData.Get<Slush::PhysicsComponent*>();
		Slush::PhysicsComponent* physB = contact.mySecond->myUserData.Get<Slush::PhysicsComponent*>();
		if (!physA || !physB)
		{
			SLUSH_WARNING("PhysContact with Entity without PhysicsComponent");
			continue;
		}

		physA->myEntity.OnCollision(physB->myEntity, contact.myContactPosition);
	}
}
