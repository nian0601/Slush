#include "stdafx.h"

#include "LevelData.h"
#include "WaveScaling.h"
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
	BuildLevelSettingsUI();

	if (ImGui::CollapsingHeader("Waves"))
		BuildWaveScalingUI();
}

void LevelData::BuildLevelSettingsUI()
{
	ImGui::Text("Navmesh: %s", myNavmeshData.GetName().GetBuffer());
	if (ImGui::BeginDragDropTarget())
	{
		if (Slush::Asset* asset = ImGui::AcceptDraggedAsset(Slush::GetAssetID<NavmeshData>()))
		{
			myNavmeshData.Set(static_cast<NavmeshData*>(asset));
			MarkAsUnsaved();
		}

		ImGui::EndDragDropTarget();
	}

	Slush::ImGuiWidgets::InputInt("Starting Resources", &myStartingResources);
	Slush::ImGuiWidgets::InputInt("Starting Lives", &myStartingLives);
	Slush::ImGuiWidgets::InputFloat2("Start Position", &myStartPosition.x);
	Slush::ImGuiWidgets::InputFloat2("Goal Position", &myGoalPosition.x);
}

void LevelData::BuildWaveScalingUI()
{
	if (Slush::ImGuiWidgets::InputInt("Total Wave Count", &myTotalWaveCount))
		myTotalWaveCount = FW_Max(myTotalWaveCount, 1);
	if (Slush::ImGuiWidgets::InputInt("Base Enemy Count", &myBaseEnemyCount))
		myBaseEnemyCount = FW_Max(myBaseEnemyCount, 0);
	if (Slush::ImGuiWidgets::InputInt("Enemy Count Per Wave Increment", &myEnemyCountPerWaveIncrement))
		myEnemyCountPerWaveIncrement = FW_Max(myEnemyCountPerWaveIncrement, 0);
	if (Slush::ImGuiWidgets::InputInt("Max Enemy Count Per Wave", &myMaxEnemyCountPerWave))
		myMaxEnemyCountPerWave = FW_Max(myMaxEnemyCountPerWave, 0);
	if (Slush::ImGuiWidgets::InputInt("Difficulty Ramp Wave Count", &myDifficultyRampWaveCount))
		myDifficultyRampWaveCount = FW_Max(myDifficultyRampWaveCount, 0);
	if (Slush::ImGuiWidgets::InputFloat("Enemy Spawn Interval", &myEnemySpawnInterval))
		myEnemySpawnInterval = FW_Max(myEnemySpawnInterval, 0.f);
	if (Slush::ImGuiWidgets::InputFloat("Inter-Wave Rest Duration", &myInterWaveRestDuration))
		myInterWaveRestDuration = FW_Max(myInterWaveRestDuration, 0.f);
	if (Slush::ImGuiWidgets::InputFloat("Pre-First-Wave Grace Duration", &myPreFirstWaveGraceDuration))
		myPreFirstWaveGraceDuration = FW_Max(myPreFirstWaveGraceDuration, 0.f);

	ImGui::Text("Enemy Unlocks");
	for (int i = 0; i < myEnemyUnlocks.Count(); ++i)
	{
		EnemyUnlockEntry& unlock = myEnemyUnlocks[i];
		ImGui::PushID(i);
		ImGui::Separator();
		ImGui::Text("Enemy Prefab: %s", unlock.myEnemyPrefab.GetName().GetBuffer());
		if (ImGui::BeginDragDropTarget())
		{
			if (Slush::Asset* asset = ImGui::AcceptDraggedAsset(Slush::GetAssetID<Slush::EntityPrefab>()))
			{
				unlock.myEnemyPrefab.Set(static_cast<Slush::EntityPrefab*>(asset));
				MarkAsUnsaved();
			}
			ImGui::EndDragDropTarget();
		}

		if (Slush::ImGuiWidgets::InputInt("Unlock Wave", &unlock.myUnlockWave))
			unlock.myUnlockWave = FW_Max(unlock.myUnlockWave, 0);

		int moveTo = i;
		ImGui::BeginDisabled(i == 0);
		if (Slush::ImGuiWidgets::Button("Up"))
			moveTo = i - 1;
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(i == myEnemyUnlocks.Count() - 1);
		if (Slush::ImGuiWidgets::Button("Down"))
			moveTo = i + 1;
		ImGui::EndDisabled();
		ImGui::SameLine();
		if (Slush::ImGuiWidgets::Button("Remove"))
		{
			myEnemyUnlocks.RemoveNonCyclicAtIndex(i);
			--i;
			ImGui::PopID();
			continue;
		}

		if (moveTo != i)
		{
			EnemyUnlockEntry movedUnlock = unlock;
			unlock = myEnemyUnlocks[moveTo];
			myEnemyUnlocks[moveTo] = movedUnlock;
		}
		ImGui::PopID();
	}

	if (Slush::ImGuiWidgets::Button("Add"))
		myEnemyUnlocks.Add();

	if (!WaveScaling::AreEnemyUnlocksAscending(myEnemyUnlocks))
		ImGui::TextWrapped("Warning: unlock waves are not in ascending order. List order controls enemy difficulty.");
	for (const EnemyUnlockEntry& unlock : myEnemyUnlocks)
	{
		if (!unlock.myEnemyPrefab.Get())
		{
			ImGui::TextWrapped("Warning: an enemy unlock has no prefab. Its spawns will be skipped.");
			break;
		}
	}
}

void LevelData::OnLoad(Slush::AssetParser::Handle aRootHandle)
{
	aRootHandle.ParseVec2fField("startPosition", myStartPosition);
	aRootHandle.ParseVec2fField("goalPosition", myGoalPosition);
	myNavmeshData.Parse(aRootHandle, "navmeshData");
	aRootHandle.ParseIntField("startingResources", myStartingResources);
	aRootHandle.ParseIntField("startingLives", myStartingLives);

	aRootHandle.ParseIntField("totalWaveCount", myTotalWaveCount);
	aRootHandle.ParseIntField("baseEnemyCount", myBaseEnemyCount);
	aRootHandle.ParseIntField("enemyCountPerWaveIncrement", myEnemyCountPerWaveIncrement);
	aRootHandle.ParseIntField("maxEnemyCountPerWave", myMaxEnemyCountPerWave);
	aRootHandle.ParseIntField("difficultyRampWaveCount", myDifficultyRampWaveCount);
	aRootHandle.ParseFloatField("enemySpawnInterval", myEnemySpawnInterval);
	aRootHandle.ParseFloatField("interWaveRestDuration", myInterWaveRestDuration);
	aRootHandle.ParseFloatField("preFirstWaveGraceDuration", myPreFirstWaveGraceDuration);

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
	aRootHandle.ParseIntField("startingLives", myStartingLives);

	aRootHandle.ParseIntField("totalWaveCount", myTotalWaveCount);
	aRootHandle.ParseIntField("baseEnemyCount", myBaseEnemyCount);
	aRootHandle.ParseIntField("enemyCountPerWaveIncrement", myEnemyCountPerWaveIncrement);
	aRootHandle.ParseIntField("maxEnemyCountPerWave", myMaxEnemyCountPerWave);
	aRootHandle.ParseIntField("difficultyRampWaveCount", myDifficultyRampWaveCount);
	aRootHandle.ParseFloatField("enemySpawnInterval", myEnemySpawnInterval);
	aRootHandle.ParseFloatField("interWaveRestDuration", myInterWaveRestDuration);
	aRootHandle.ParseFloatField("preFirstWaveGraceDuration", myPreFirstWaveGraceDuration);

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
