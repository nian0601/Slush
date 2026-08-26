#include "stdafx.h"

#include "Level.h"
#include <Core\Assets\AssetStorage.h>
#include <Core\Engine.h>
#include <Core\Input.h>

#include "TopDownGameGlobals.h"
#include "Components/HealthComponent.h"

Level::Level()
{
	myLevelData = Slush::AssetRegistry::GetInstance().GetAsset<LevelData>("level_main");
	FW_ASSERT(myLevelData, "Level has no valid LevelData - expected a 'level_main' LevelData asset");
	FW_ASSERT(myLevelData->myNavmeshData.Get() != nullptr, "Level's LevelData has an unresolved NavmeshData reference");

	TopDownGameGlobals::GetInstance().SetLevel(this);
}

Level::~Level()
{
	TopDownGameGlobals::GetInstance().SetLevel(nullptr);
}

Navmesh& Level::GetNavmesh()
{
	return myLevelData->myNavmeshData.Get()->myNavmesh;
}

NavmeshData& Level::GetNavmeshDataAsset()
{
	return *myLevelData->myNavmeshData.Get();
}

void Level::Update()
{
	Slush::Engine& engine = Slush::Engine::GetInstance();
	if (engine.GetInput().WasKeyReleased(Slush::Input::_7))
	{
		SpawnEnemyNormal();
	}
	if (engine.GetInput().WasKeyReleased(Slush::Input::_8))
	{
		DamageAllEnemies();
	}

	GetNavmesh().Update();
	myEntityManager.Update();
	myEntityManager.EndFrame();
}

void Level::Render()
{
	GetNavmesh().Render();
	myEntityManager.Render();
}

void Level::SpawnEnemyNormal()
{
	myEntityManager.CreateEntity(myLevelData->myStartPosition, "Enemy_Normal");
}

void Level::DamageAllEnemies()
{
	FW_GrowingArray<Slush::EntityHandle> entities;
	myEntityManager.GetAllEntities(entities);

	for (const Slush::EntityHandle& handle : entities)
	{
		Slush::Entity* entity = handle.Get();
		if (!entity || entity->myIsMarkedForRemoval)
			continue;

		if (HealthComponent* health = entity->GetComponent<HealthComponent>())
			health->DealDamage(10);
	}
}
