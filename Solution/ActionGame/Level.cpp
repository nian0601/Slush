#include "Level.h"
#include "EntityManager.h"
#include "HealthComponent.h"
#include "NPCWave.h"
#include <FW_Includes.h>
#include <Core\Input.h>
#include "ExperienceComponent.h"

Level::Level(EntityManager& aEntityManager, Slush::PhysicsWorld& aPhysicsWorld)
	: myEntityManager(aEntityManager)
	, myPhysicsWorld(aPhysicsWorld)
{
	Entity* player = myEntityManager.CreateEntity({ 400.f, 400.f }, "Player");
	myPlayerHandle = player->myHandle;

	myNPCWave = new NPCWave(myEntityManager, myPhysicsWorld);
}

Level::~Level()
{
	FW_SAFE_DELETE(myNPCWave);
	myEntityManager.DeleteAllEntities();
}

void Level::Update()
{
	myNPCWave->Update();

	Entity* player = myPlayerHandle.Get();
	if (!player)
		return;

	Slush::Engine& engine = Slush::Engine::GetInstance();
	const Slush::Input& input = engine.GetInput();
	if (input.WasKeyReleased(Slush::Input::E))
	{
		player->myExperienceComponent->AddExperience(1);
	}
}

bool Level::IsPlayerDead() const
{
	Entity* player = myPlayerHandle.Get();
	if (!player)
		return true;

	if (HealthComponent* health = player->myHealthComponent)
		return health->IsDead();

	return true;
}
