#include "stdafx.h"

#include "ActionGameGlobals.h"
#include "LevelState.h"
#include "GameOverState.h"

#include "Physics\PhysicsWorld.h"
#include "EntitySystem\EntityManager.h"
#include "Level\Level.h"
#include "StateStack.h"

LevelState::LevelState()
{
	myPhysicsWorld = new Slush::PhysicsWorld();
	myEntityManager = new EntityManager();

	ActionGameGlobals& globals = ActionGameGlobals::GetInstance();
	ActionGameGlobals::GetInstance().SetPhysicsWorld(myPhysicsWorld);
	ActionGameGlobals::GetInstance().SetEntityManager(myEntityManager);

	myLevel = new Level();
	myLevel->Restart();
}

LevelState::~LevelState()
{
	FW_SAFE_DELETE(myLevel);

	ActionGameGlobals::GetInstance().SetPhysicsWorld(nullptr);
	ActionGameGlobals::GetInstance().SetEntityManager(nullptr);
}

void LevelState::ResumeState()
{
	myLevel->Restart();
}

GameState::GameStateResult LevelState::Update()
{
	bool pauseEntityUpdate = myLevel->IsShowingUI(); // Push SubStates instead
	if (!pauseEntityUpdate)
	{
		myEntityManager->PrePhysicsUpdate();
		myPhysicsWorld->TickLimited(Slush::Time::GetDelta());
		UpdateCollisions();
		myEntityManager->Update();
	}

	myLevel->Update();
	if (myLevel->IsPlayerDead())
		myStateStack->PushSubState(new GameOverState());

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
	
	myLevel->RenderUI();
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
