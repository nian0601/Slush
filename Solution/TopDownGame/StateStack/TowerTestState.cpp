#include "stdafx.h"

#include "TowerTestState.h"

#include "Level/Level.h"
#include "Level/LevelData.h"

#include "Core/Assets/AssetStorage.h"
#include "Core/Engine.h"
#include "Core/Input.h"
#include <EntitySystem/EntityPrefab.h>

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
}

TowerTestState::TowerTestState(LevelData& aLevelData)
{
	myLevel = new Level(aLevelData);

	Slush::AssetRegistry& assetRegistry = Slush::AssetRegistry::GetInstance();
	myBasicTowerPrefab = assetRegistry.GetAsset<Slush::EntityPrefab>("Tower_Basic");
	mySplashTowerPrefab = assetRegistry.GetAsset<Slush::EntityPrefab>("Tower_Splash");
	FW_ASSERT(myBasicTowerPrefab, "-towertest requires a 'Tower_Basic' EntityPrefab");
	FW_ASSERT(mySplashTowerPrefab, "-towertest requires a 'Tower_Splash' EntityPrefab");
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
	// enemies first would change F5's NotAffordable outcome (125 starting - 50 Basic - 75 Splash = 0).
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

	if (myPendingClusterEnemyCount > 0)
	{
		myLevel->SpawnEnemyNormal();
		--myPendingClusterEnemyCount;
	}
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
