#include "Level.h"
#include "EntityManager.h"
#include "HealthComponent.h"
#include "NPCWave.h"
#include <FW_Includes.h>
#include <Core\Input.h>
#include "ExperienceComponent.h"
#include "StatsComponent.h"
#include <imgui\imgui.h>

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

	myIsLevelingUp = player->myExperienceComponent->NeedsLevelUp();
	if (myIsLevelingUp)
	{
		if (StatsComponent* stats = player->myStatsComponent)
		{
			if (!stats->CanUpgradeCooldownReduction() && !stats->CanUpgradeDamage())
			{
				player->myExperienceComponent->LevelUp();
				myIsLevelingUp = false;
				SLUSH_WARNING("No more available upgrades, auto-leveling");
			}
			else if (stats->CanUpgradeCooldownReduction() && ImGui::Button("Upgrade Cooldown"))
			{
				stats->AddCooldownReductionUpgrade();
				player->myExperienceComponent->LevelUp();
				myIsLevelingUp = false;
			}
			else if (stats->CanUpgradeDamage() && ImGui::Button("Upgrade Damage"))
			{
				stats->AddDamageUpgrade();
				player->myExperienceComponent->LevelUp();
				myIsLevelingUp = false;
			}
		}
		else
		{
			SLUSH_ERROR("Player doesnt have a StatsComponent, not able to pick upgrade when Leveling");
			player->myExperienceComponent->LevelUp();
			myIsLevelingUp = false;
		}
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
