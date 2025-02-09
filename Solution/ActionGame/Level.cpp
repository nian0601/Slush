#include "stdafx.h"

#include "EntityManager.h"
#include "ExperienceComponent.h"
#include "HealthComponent.h"
#include "Level.h"
#include "NPCWave.h"
#include "StatsComponent.h"

#include <Core\Input.h>
#include <UI\UIManager.h>
#include <UI\UIButton.h>

Level::Level(EntityManager& aEntityManager, Slush::PhysicsWorld& aPhysicsWorld, Slush::Font& aFont, Slush::AssetStorage<Slush::UILayout>& someUILayouts)
	: myEntityManager(aEntityManager)
	, myPhysicsWorld(aPhysicsWorld)
{
	Entity* player = myEntityManager.CreateEntity({ 400.f, 400.f }, "Player");
	myPlayerHandle = player->myHandle;

	myNPCWave = new NPCWave(myEntityManager, myPhysicsWorld);

	myUIManager = new Slush::UIManager(aFont);
	myUIManager->SetLayout(someUILayouts.GetAsset("LevelUp"));

	if (Slush::UIWidget* button = myUIManager->FindWidget("DamageUpgrade"))
		myDamageUpgradeButton = static_cast<Slush::UIButton*>(button);

	if (Slush::UIWidget* button = myUIManager->FindWidget("CooldownUpgrade"))
		myCooldownUpgradeButton = static_cast<Slush::UIButton*>(button);

	if (Slush::UIWidget* button = myUIManager->FindWidget("ProjectileUpgrade"))
		myProjectileUpgradeButton = static_cast<Slush::UIButton*>(button);
}

Level::~Level()
{
	FW_SAFE_DELETE(myUIManager);
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

	if (StatsComponent* stats = player->myStatsComponent)
	{
		Slush::Engine& engine = Slush::Engine::GetInstance();
		if (engine.GetInput().WasKeyPressed(Slush::Input::Q))
			stats->AddAdditionalProjectilesUpgrade();
	}

	myIsLevelingUp = player->myExperienceComponent->NeedsLevelUp();
	if (myIsLevelingUp)
	{
		if (StatsComponent* stats = player->myStatsComponent)
		{
			Slush::Engine& engine = Slush::Engine::GetInstance();
			myUIManager->Update(engine.GetInput());

			if (!stats->CanUpgradeCooldownReduction() && !stats->CanUpgradeDamage() && !stats->CanUpgradeAdditionalProjectiles())
			{
				player->myExperienceComponent->LevelUp();
				myIsLevelingUp = false;
				SLUSH_WARNING("No more available upgrades, auto-leveling");
			}
			else if (stats->CanUpgradeCooldownReduction() && myCooldownUpgradeButton->WasPressed())
			{
				stats->AddCooldownReductionUpgrade();
				player->myExperienceComponent->LevelUp();
				myIsLevelingUp = false;
			}
			else if (stats->CanUpgradeDamage() && myDamageUpgradeButton->WasPressed())
			{
				stats->AddDamageUpgrade();
				player->myExperienceComponent->LevelUp();
				myIsLevelingUp = false;
			}
			else if (stats->CanUpgradeAdditionalProjectiles() && myProjectileUpgradeButton->WasPressed())
			{
				stats->AddAdditionalProjectilesUpgrade();
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

void Level::Render()
{
	if (myIsLevelingUp)
		myUIManager->Render();
}

bool Level::IsPlayerDead() const
{
	Entity* player = myPlayerHandle.Get();
	if (!player)
		return true;

	if (HealthComponent* health = player->GetComponent<HealthComponent>())
		return health->IsDead();

	return true;
}