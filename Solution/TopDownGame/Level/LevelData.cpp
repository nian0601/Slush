#include "stdafx.h"

#include "LevelData.h"
#include <EntitySystem\EntityPrefab.h>
#include <imgui\ImGuiWidgets.h>

LevelData::LevelData(const char* aName, unsigned int aAssetID)
	: DataAsset(aName, aAssetID)
{
}

void LevelData::OnParse(Slush::AssetParser::Handle aRootHandle, unsigned int /*aVersion*/)
{
	if (aRootHandle.IsReading())
		OnLoad(aRootHandle);
	else
		OnSave(aRootHandle);
}

void LevelData::ResolveDependencies()
{
	myNavmeshData.ResolveDependency();

	for (EnemyUnlockEntry& unlock : myEnemyUnlocks)
		unlock.myEnemyPrefab.ResolveDependency();
}

void LevelData::BuildUI()
{
	Slush::ImGuiWidgets::InputFloat2("Start Position", &myStartPosition.x);
	Slush::ImGuiWidgets::InputFloat2("Goal Position", &myGoalPosition.x);
	Slush::ImGuiWidgets::InputInt("Starting Resources", &myStartingResources);

	ImGui::Text("Navmesh: %s", myNavmeshData.GetName().GetBuffer());
	if (ImGui::BeginDragDropTarget())
	{
		if (Slush::Asset* asset = ImGui::AcceptDraggedAsset(Slush::GetAssetID<NavmeshData>()))
			myNavmeshData.Set(static_cast<NavmeshData*>(asset));

		ImGui::EndDragDropTarget();
	}
}

void LevelData::OnLoad(Slush::AssetParser::Handle aRootHandle)
{
	aRootHandle.ParseVec2fField("startPosition", myStartPosition);
	aRootHandle.ParseVec2fField("goalPosition", myGoalPosition);
	myNavmeshData.Parse(aRootHandle, "navmeshData");
	aRootHandle.ParseIntField("startingResources", myStartingResources);

	aRootHandle.ParseIntField("totalWaveCount", myTotalWaveCount);
	aRootHandle.ParseIntField("baseEnemyCount", myBaseEnemyCount);
	aRootHandle.ParseIntField("enemyCountPerWaveIncrement", myEnemyCountPerWaveIncrement);
	aRootHandle.ParseIntField("maxEnemyCountPerWave", myMaxEnemyCountPerWave);
	aRootHandle.ParseIntField("difficultyRampWaveCount", myDifficultyRampWaveCount);
	aRootHandle.ParseFloatField("enemySpawnInterval", myEnemySpawnInterval);
	aRootHandle.ParseFloatField("interWaveRestDuration", myInterWaveRestDuration);

	Slush::AssetParser::Handle enemyUnlocksHandle = aRootHandle.ParseChildElement("enemyUnlocks");
	if (enemyUnlocksHandle.IsValid())
	{
		int numUnlocks = enemyUnlocksHandle.GetNumChildElements();
		myEnemyUnlocks.Reserve(numUnlocks);

		for (int i = 0; i < numUnlocks; ++i)
		{
			Slush::AssetParser::Handle unlockHandle = enemyUnlocksHandle.GetChildElementAtIndex(i);

			EnemyUnlockEntry& unlock = myEnemyUnlocks[i];
			unlock.myEnemyPrefab.Parse(unlockHandle, "enemyPrefab");
			unlockHandle.ParseIntField("unlockWave", unlock.myUnlockWave);
		}
	}
}

void LevelData::OnSave(Slush::AssetParser::Handle aRootHandle)
{
	aRootHandle.ParseVec2fField("startPosition", myStartPosition);
	aRootHandle.ParseVec2fField("goalPosition", myGoalPosition);
	myNavmeshData.Parse(aRootHandle, "navmeshData");
	aRootHandle.ParseIntField("startingResources", myStartingResources);

	aRootHandle.ParseIntField("totalWaveCount", myTotalWaveCount);
	aRootHandle.ParseIntField("baseEnemyCount", myBaseEnemyCount);
	aRootHandle.ParseIntField("enemyCountPerWaveIncrement", myEnemyCountPerWaveIncrement);
	aRootHandle.ParseIntField("maxEnemyCountPerWave", myMaxEnemyCountPerWave);
	aRootHandle.ParseIntField("difficultyRampWaveCount", myDifficultyRampWaveCount);
	aRootHandle.ParseFloatField("enemySpawnInterval", myEnemySpawnInterval);
	aRootHandle.ParseFloatField("interWaveRestDuration", myInterWaveRestDuration);

	Slush::AssetParser::Handle enemyUnlocksHandle = aRootHandle.ParseChildElement("enemyUnlocks");
	if (enemyUnlocksHandle.IsValid())
	{
		for (EnemyUnlockEntry& unlock : myEnemyUnlocks)
		{
			Slush::AssetParser::Handle unlockHandle = enemyUnlocksHandle.ParseChildElement("unlock");

			unlock.myEnemyPrefab.Parse(unlockHandle, "enemyPrefab");
			unlockHandle.ParseIntField("unlockWave", unlock.myUnlockWave);
		}
	}
}
