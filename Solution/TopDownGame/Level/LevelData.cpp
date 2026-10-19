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
	if (ImGui::CollapsingHeader("Scaling", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (Slush::ImGuiWidgets::InputInt("Total Wave Count", &myTotalWaveCount))
			myTotalWaveCount = FW_Max(myTotalWaveCount, 1);
		if (Slush::ImGuiWidgets::InputInt("Base Enemy Count", &myBaseEnemyCount))
			myBaseEnemyCount = FW_Max(myBaseEnemyCount, 0);
		if (Slush::ImGuiWidgets::InputInt("Count Increment / Wave", &myEnemyCountPerWaveIncrement))
			myEnemyCountPerWaveIncrement = FW_Max(myEnemyCountPerWaveIncrement, 0);
		if (Slush::ImGuiWidgets::InputInt("Max Count / Wave", &myMaxEnemyCountPerWave))
			myMaxEnemyCountPerWave = FW_Max(myMaxEnemyCountPerWave, 0);
		if (Slush::ImGuiWidgets::InputInt("Difficulty Ramp Waves", &myDifficultyRampWaveCount))
			myDifficultyRampWaveCount = FW_Max(myDifficultyRampWaveCount, 0);
	}

	if (ImGui::CollapsingHeader("Timing", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (Slush::ImGuiWidgets::InputFloat("Spawn Interval (s)", &myEnemySpawnInterval))
			myEnemySpawnInterval = FW_Max(myEnemySpawnInterval, 0.f);
		if (Slush::ImGuiWidgets::InputFloat("Inter-Wave Rest (s)", &myInterWaveRestDuration))
			myInterWaveRestDuration = FW_Max(myInterWaveRestDuration, 0.f);
		if (Slush::ImGuiWidgets::InputFloat("Pre-First-Wave Grace (s)", &myPreFirstWaveGraceDuration))
			myPreFirstWaveGraceDuration = FW_Max(myPreFirstWaveGraceDuration, 0.f);
	}

	if (ImGui::CollapsingHeader("Enemy Unlocks", ImGuiTreeNodeFlags_DefaultOpen))
		BuildEnemyUnlocksUI();

	if (ImGui::CollapsingHeader("Preview", ImGuiTreeNodeFlags_DefaultOpen))
		BuildWavePreviewUI();
}

void LevelData::BuildEnemyUnlocksUI()
{
	// Reorder/remove are applied after the table so the rows never shift mid-iteration.
	int moveFrom = -1;
	int moveTo = -1;
	int removeIndex = -1;

	const ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingFixedFit;
	if (ImGui::BeginTable("EnemyUnlocks", 4, tableFlags))
	{
		ImGui::TableSetupColumn("#");
		ImGui::TableSetupColumn("Enemy prefab", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("Unlock wave");
		ImGui::TableSetupColumn("");
		ImGui::TableHeadersRow();

		for (int i = 0; i < myEnemyUnlocks.Count(); ++i)
		{
			EnemyUnlockEntry& unlock = myEnemyUnlocks[i];
			ImGui::PushID(i);
			ImGui::TableNextRow();

			ImGui::TableSetColumnIndex(0);
			ImGui::AlignTextToFramePadding();
			ImGui::Text("%d", i);

			ImGui::TableSetColumnIndex(1);
			ImGui::AlignTextToFramePadding();
			if (unlock.myEnemyPrefab.Get())
				ImGui::Text("%s", unlock.myEnemyPrefab.GetName().GetBuffer());
			else
				ImGui::TextDisabled("(drop a prefab)");
			if (ImGui::BeginDragDropTarget())
			{
				if (Slush::Asset* asset = ImGui::AcceptDraggedAsset(Slush::GetAssetID<Slush::EntityPrefab>()))
				{
					unlock.myEnemyPrefab.Set(static_cast<Slush::EntityPrefab*>(asset));
					MarkAsUnsaved();
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::TableSetColumnIndex(2);
			ImGui::SetNextItemWidth(90.f);
			if (Slush::ImGuiWidgets::InputInt("##UnlockWave", &unlock.myUnlockWave))
				unlock.myUnlockWave = FW_Max(unlock.myUnlockWave, 0);

			ImGui::TableSetColumnIndex(3);
			ImGui::BeginDisabled(i == 0);
			if (Slush::ImGuiWidgets::Button(ICON_FA_ARROW_UP))
			{
				moveFrom = i;
				moveTo = i - 1;
			}
			ImGui::EndDisabled();
			ImGui::SameLine();
			ImGui::BeginDisabled(i == myEnemyUnlocks.Count() - 1);
			if (Slush::ImGuiWidgets::Button(ICON_FA_ARROW_DOWN))
			{
				moveFrom = i;
				moveTo = i + 1;
			}
			ImGui::EndDisabled();
			ImGui::SameLine();
			if (Slush::ImGuiWidgets::Button(ICON_FA_XMARK))
				removeIndex = i;

			ImGui::PopID();
		}

		ImGui::EndTable();
	}

	if (moveFrom >= 0)
	{
		EnemyUnlockEntry movedUnlock = myEnemyUnlocks[moveFrom];
		myEnemyUnlocks[moveFrom] = myEnemyUnlocks[moveTo];
		myEnemyUnlocks[moveTo] = movedUnlock;
	}

	if (removeIndex >= 0)
		myEnemyUnlocks.RemoveNonCyclicAtIndex(removeIndex);

	if (Slush::ImGuiWidgets::Button("+ Add"))
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

void LevelData::BuildWavePreviewUI()
{
	ImGui::TextDisabled("Waves are 0-based. The pool is every unlocked enemy; later waves favour the later ones.");

	const ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingFixedFit;
	if (ImGui::BeginTable("WavePreview", 3, tableFlags))
	{
		ImGui::TableSetupColumn("Wave");
		ImGui::TableSetupColumn("Enemies");
		ImGui::TableSetupColumn("Pool", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableHeadersRow();

		WaveScaling::WavePreview preview;
		for (int wave = 0; wave < myTotalWaveCount; ++wave)
		{
			WaveScaling::GetWavePreview(wave, myBaseEnemyCount, myEnemyCountPerWaveIncrement, myMaxEnemyCountPerWave, myEnemyUnlocks, preview);

			FW_String pool;
			for (int i = 0; i < preview.myUnlockedIndices.Count(); ++i)
			{
				if (i > 0)
					pool += ", ";

				const EnemyUnlockEntry& unlock = myEnemyUnlocks[preview.myUnlockedIndices[i]];
				pool += unlock.myEnemyPrefab.Get() ? unlock.myEnemyPrefab.GetName().GetBuffer() : "(missing)";
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%d", wave);
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%d", preview.myEnemyCount);
			ImGui::TableSetColumnIndex(2);
			if (pool.Empty())
				ImGui::TextDisabled("(none unlocked)");
			else
				ImGui::TextWrapped("%s", pool.GetBuffer());
		}

		ImGui::EndTable();
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

	// Cleared first so reloading an already-loaded LevelData (Level Editor discard) doesn't keep stale entries
	myEnemyUnlocks.RemoveAll();
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

	// Cleared first so reloading an already-loaded LevelData (Level Editor discard) doesn't keep stale entries
	myEnemyUnlocks.RemoveAll();
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
