#include "stdafx.h"

#include "ActionGameGlobals.h"

#include "Components/ExperienceComponent.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "Components/WeaponComponent.h"

#include "Core/Assets/AssetStorage.h"
#include "Core/Input.h"

#include "EntitySystem/EntityManager.h"

#include "Level.h"
#include "LevelData.h"

#include "StateStack/StateStack.h"
#include "StateStack/UpgradeStatsState.h"
#include "StateStack/UpgradeWeaponState.h"

#include "Tilemap.h"
#include "Graphics/Window.h"

Level::Level(const CharacterInfo& aCharacterInfo)
	: myPlayerCharacterInfo(aCharacterInfo)
	, myEntityManager(ActionGameGlobals::GetInstance().GetEntityManager())
{
	Slush::AssetRegistry& assets = Slush::AssetRegistry::GetInstance();
	myLevelData = assets.GetAsset<LevelData>("level_main");
	
	myTilemap = new Tilemap();
}

Level::~Level()
{
	FW_SAFE_DELETE(myTilemap);
	myEntityManager.DeleteAllEntities();
}

void Level::Update(StateStack& aStateStack)
{
	Entity* player = myPlayerHandle.Get();
	if (!player)
		return;

	ExperienceComponent* expComp = player->GetComponent<ExperienceComponent>();
	WeaponComponent* weaponComp = player->GetComponent<WeaponComponent>();

	Slush::Engine& engine = Slush::Engine::GetInstance();
	const Slush::Input& input = engine.GetInput();
	if (input.WasKeyReleased(Slush::Input::E))
	{
		expComp->AddExperience(1);
	}
	else if (input.WasKeyReleased(Slush::Input::Q))
	{
		weaponComp->AddPendingUpgrade();
	}

	if (expComp->NeedsLevelUp())
		aStateStack.PushSubState(new UpgradeStatsState(myPlayerHandle));
	else if (weaponComp->HasPendingUpgrade())
		aStateStack.PushSubState(new UpgradeWeaponState(myPlayerHandle));
	else
		HandleEnemyWaves();
}

void Level::Restart()
{
	myEntityManager.DeleteAllEntities();

	myEnemyWaveData = &myLevelData->myEnemyWaves[0];
	myWaveTimer.Start(3.f);

	Entity* player = myEntityManager.CreateEntity(myLevelData->myPlayerStartPosition, myLevelData->myPlayerEntityPrefab);
	myPlayerHandle = player->myHandle;

	myTilemap->CreateWallEntities(myEntityManager);
}

void Level::RenderGame()
{
	myTilemap->Render();
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

void Level::HandleEnemyWaves()
{
	if (ActionGameGlobals::GetInstance().myDebugSettings.myPauseEnemySpawning)
		return;

	if (myWaveTimer.IsStarted() && !myWaveTimer.HasExpired())
		return;

	myWaveTimer.Start(myEnemyWaveData->myDuration);

	Entity* player = myPlayerHandle.Get();

	const int iterationLimit = 100;
	int iterations = 0;
	Vector2f spawnArea = { 1920.f, 1080.f };
	Vector2f spawnAreaMargin = { 75.f, 75.f };
	const int numEnemies = FW_RandInt(myEnemyWaveData->myMinEnemyCount, myEnemyWaveData->myMaxEnemyCount);
	FW_GrowingArray<Entity*> spawnedEnemies;
	while (spawnedEnemies.Count() < numEnemies && iterations <= iterationLimit)
	{
		++iterations;

		float randX = FW_RandFloat(spawnAreaMargin.x, spawnArea.x - spawnAreaMargin.x);
		float randY = FW_RandFloat(spawnAreaMargin.y, spawnArea.y - spawnAreaMargin.y);
		Vector2f position;
		position.x += randX;
		position.y += randY;

		if (IsTooClose(player->myPosition, position, myPlayerClearanceRadius))
			continue;

		for (Entity* enemy : spawnedEnemies)
		{
			if (IsTooClose(enemy->myPosition, position, myEnemyClearanceRadius))
				continue;
		}

		int enemyPick = FW_RandInt(0, myEnemyWaveData->myEnemyPrefabs.Count() -1);

		Entity* enemy = myEntityManager.CreateEntity(position, myEnemyWaveData->myEnemyPrefabs[enemyPick]);
		if (ProjectileShootingComponent* projShoot = enemy->GetComponent<ProjectileShootingComponent>())
			projShoot->TriggerCooldown();

		spawnedEnemies.Add(enemy);
	}

	++myWaveCounter;
	if (myWaveCounter >= myLevelData->myEnemyWaves.Count())
		myWaveCounter = 0;

	myEnemyWaveData = &myLevelData->myEnemyWaves[myWaveCounter];
}

bool Level::IsTooClose(const Vector2f& aPosition, const Vector2f& aTestPosition, float aTestClearance)
{
	float distance2 = Length2(aPosition - aTestPosition);
	return distance2 <= FW_Square(aTestClearance);
}
