#include "stdafx.h"

#include "TowerTestState.h"

#include "Level/Level.h"
#include "Level/LevelData.h"
#include "Components/BuildCostComponent.h"

#include "Core/Assets/AssetStorage.h"
#include "Core/Engine.h"
#include "Core/Input.h"
#include <EntitySystem/EntityPrefab.h>
#include <EntitySystem/Components/SpriteComponent.h>

namespace
{
	// Picked from navmesh_main's geometry: a U-shaped mesh whose bottom corridor (y 542-916) is the
	// start->goal path. A and B sit in that corridor within tower range (300) of the path, leaving it open
	// on both sides; C is open mesh in the bottom-left corner, clear of A and B; OffMesh is outside the mesh.
	const Vector2f ourSlotA = { 700.f, 700.f };
	const Vector2f ourSlotB = { 1300.f, 700.f };
	const Vector2f ourSlotC = { 150.f, 780.f };
	const Vector2f ourOffMeshPosition = { -500.f, -500.f };

	const char* GetPlaceTowerResultName(PlaceTowerResult aResult)
	{
		switch (aResult)
		{
		case PlaceTowerResult::Placed: return "Placed";
		case PlaceTowerResult::FootprintBlocked: return "FootprintBlocked";
		case PlaceTowerResult::NotAffordable: return "NotAffordable";
		}

		return "Unknown";
	}

	void CheckSnapPlacement(bool aPassed, const char* aName)
	{
		if (aPassed)
		{
			SLUSH_INFO("[TowerSnapTest] %s - PASS", aName);
		}
		else
		{
			SLUSH_ERROR("[TowerSnapTest] %s - FAIL", aName);
		}
	}
}

TowerTestState::TowerTestState(LevelData& aLevelData)
{
	myLevel = new Level(aLevelData, true);

	Slush::AssetRegistry& assetRegistry = Slush::AssetRegistry::GetInstance();
	myBasicTowerPrefab = assetRegistry.GetAsset<Slush::EntityPrefab>("Tower_Basic");
	mySplashTowerPrefab = assetRegistry.GetAsset<Slush::EntityPrefab>("Tower_Splash");
	FW_ASSERT(myBasicTowerPrefab, "-towertest requires a 'Tower_Basic' EntityPrefab");
	FW_ASSERT(mySplashTowerPrefab, "-towertest requires a 'Tower_Splash' EntityPrefab");
	// Keep F1/F4 affordable and F5 unaffordable as authored tower costs change.
	myLevel->TrySpendResources(myLevel->GetResources());
	myLevel->AddResources(BuildCostComponent::GetCost(*myBasicTowerPrefab) + BuildCostComponent::GetCost(*mySplashTowerPrefab));
}

TowerTestState::~TowerTestState()
{
	FW_SAFE_DELETE(myLevel);
}

Slush::IGameState::GameStateResult TowerTestState::Update()
{
	UpdateScenarioKeys();
	myLevel->Update();
	return Slush::IGameState::KEEP;
}

void TowerTestState::Render()
{
	myLevel->Render();
}

void TowerTestState::UpdateScenarioKeys()
{
	// Key order is part of the contract: run F1-F5 before F6/F7. Kills earn resources, so spawning
	// enemies first would change F5's NotAffordable outcome (starting budget = Basic + Splash costs).
	const Slush::Input& input = Slush::Engine::GetInstance().GetInput();

	if (input.WasKeyReleased(Slush::Input::_F1))
		RunPlacementScenario("Basic at slot A", *myBasicTowerPrefab, ourSlotA, PlaceTowerResult::Placed);

	if (input.WasKeyReleased(Slush::Input::_F2))
		RunPlacementScenario("Basic off mesh", *myBasicTowerPrefab, ourOffMeshPosition, PlaceTowerResult::FootprintBlocked);

	if (input.WasKeyReleased(Slush::Input::_F3))
		RunPlacementScenario("Basic overlapping slot A", *myBasicTowerPrefab, ourSlotA, PlaceTowerResult::FootprintBlocked);

	if (input.WasKeyReleased(Slush::Input::_F4))
		RunPlacementScenario("Splash at slot B", *mySplashTowerPrefab, ourSlotB, PlaceTowerResult::Placed);

	if (input.WasKeyReleased(Slush::Input::_F5))
		RunPlacementScenario("Basic at slot C without resources", *myBasicTowerPrefab, ourSlotC, PlaceTowerResult::NotAffordable);

	if (input.WasKeyReleased(Slush::Input::_F6))
		myLevel->SpawnEnemyNormal();

	// Spawned one per frame rather than all at once: same-frame spawns share an exact position, and a
	// projectile damages every target within its splash radius of the target - for Projectile_Basic
	// (radius 0) that still includes exact duplicates, so an exact stack would all die to the basic tower
	// before reaching the splash tower. One frame apart they stay a tight cluster for the splash radius.
	if (input.WasKeyReleased(Slush::Input::_F7))
		myPendingClusterEnemyCount += 3;

	if (input.WasKeyReleased(Slush::Input::_F8))
		RunSnapPlacementScenarios();

	if (myPendingClusterEnemyCount > 0)
	{
		myLevel->SpawnEnemyNormal();
		--myPendingClusterEnemyCount;
	}
}

void TowerTestState::RunSnapPlacementScenarios()
{
	Level level(myLevel->GetLevelDataAsset(), true, true);
	level.GetNavmesh().GenerateDefaultGrid();
	const int cost = BuildCostComponent::GetCost(*myBasicTowerPrefab);
	level.TrySpendResources(level.GetResources());
	level.AddResources(cost * 2);
	const Vector2f size = myBasicTowerPrefab->GetComponentData<Slush::SpriteComponent>().mySize;
	CheckSnapPlacement(level.TryPlaceTower(*myBasicTowerPrefab, ourSlotA) == PlaceTowerResult::Placed
		&& level.GetResources() == cost, "First placement spends exactly one cost");

	const int verticesBeforeQuery = level.GetNavmesh().GetVertexCount();
	const int trianglesBeforeQuery = level.GetNavmesh().GetTriangleCount();
	const Vector2f requested = ourSlotA + Vector2f(size.x + 5.f, 3.f);
	const Vector2f resolved = level.ResolveTowerPosition(*myBasicTowerPrefab, requested);
	const Vector2f expected = ourSlotA + Vector2f(size.x, 3.f);
	CheckSnapPlacement(resolved == expected, "Resolve contact using actual prefab size while leaving Y free");
	CheckSnapPlacement(level.GetResources() == cost && level.GetNavmesh().GetVertexCount() == verticesBeforeQuery
		&& level.GetNavmesh().GetTriangleCount() == trianglesBeforeQuery, "Preview resolution changes no resources or mesh");
	CheckSnapPlacement(level.TryPlaceTower(*myBasicTowerPrefab, resolved) == PlaceTowerResult::Placed
		&& level.GetResources() == 0, "Snapped contact places and spends the remaining cost");
	level.GetEntityManager().EndFrame();
	FW_GrowingArray<Slush::EntityHandle> entities;
	level.GetEntityManager().GetAllEntities(entities);
	CheckSnapPlacement(entities.Count() == 2, "Successful placements create exactly two entities");
	if (entities.Count() == 2)
		CheckSnapPlacement(entities[1].Get()->myPosition == resolved, "Entity lands at the shared preview position without another snap");

	const int verticesAfterPlacement = level.GetNavmesh().GetVertexCount();
	const int trianglesAfterPlacement = level.GetNavmesh().GetTriangleCount();
	CheckSnapPlacement(level.TryPlaceTower(*myBasicTowerPrefab, ourSlotA) == PlaceTowerResult::FootprintBlocked,
		"Actual overlap is rejected by final validation");
	const Vector2f thirdPosition = level.ResolveTowerPosition(*myBasicTowerPrefab, expected + Vector2f(size.x + 5.f, 0.f));
	CheckSnapPlacement(level.TryPlaceTower(*myBasicTowerPrefab, thirdPosition) == PlaceTowerResult::NotAffordable,
		"Valid snapped contact still requires affordability");
	const Vector2f offMesh = level.ResolveTowerPosition(*myBasicTowerPrefab, ourOffMeshPosition);
	CheckSnapPlacement(offMesh == ourOffMeshPosition
		&& level.TryPlaceTower(*myBasicTowerPrefab, offMesh) == PlaceTowerResult::FootprintBlocked,
		"Invalid unsnapped fallback still gets rejected");
	level.GetEntityManager().EndFrame();
	entities.RemoveAll();
	level.GetEntityManager().GetAllEntities(entities);
	CheckSnapPlacement(level.GetResources() == 0 && entities.Count() == 2
		&& level.GetNavmesh().GetVertexCount() == verticesAfterPlacement
		&& level.GetNavmesh().GetTriangleCount() == trianglesAfterPlacement,
		"Rejected placement changes no resources, entities or mesh");
}

void TowerTestState::RunPlacementScenario(const char* aName, const Slush::EntityPrefab& aTowerPrefab, const Vector2f& aPosition, PlaceTowerResult anExpectedResult)
{
	const PlaceTowerResult result = myLevel->TryPlaceTower(aTowerPrefab, aPosition);
	const char* expectedName = GetPlaceTowerResultName(anExpectedResult);
	const char* resultName = GetPlaceTowerResultName(result);

	if (result == anExpectedResult)
	{
		SLUSH_INFO("[TowerTest] %s: expected %s, got %s - PASS", aName, expectedName, resultName);
	}
	else
	{
		SLUSH_ERROR("[TowerTest] %s: expected %s, got %s - FAIL", aName, expectedName, resultName);
	}
}
