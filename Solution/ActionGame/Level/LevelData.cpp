#include "stdafx.h"

#include "LevelData.h"
#include <ActionGameGlobals.h>
#include <Core\Assets\AssetStorage.h>
#include <EntitySystem\EntityPrefab.h>

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
	myPlayerEntityPrefab.ResolveDependency();

	for (EnemyWaveData& waveData : myEnemyWaves)
		for (Slush::AssetReference<Slush::EntityPrefab>& prefab : waveData.myEnemyPrefabs)
			prefab.ResolveDependency();
}

void LevelData::BuildUI()
{
	ActionGameGlobals& globals = ActionGameGlobals::GetInstance();

	ImGui::InputFloat2("Player Position", &myPlayerStartPosition.x);
	ImGui::Text("Player Prefab: %s", myPlayerEntityPrefab.GetName().GetBuffer());

	if (ImGui::BeginDragDropTarget())
	{
		if (Slush::Asset* asset = ImGui::AcceptDraggedAsset(Slush::GetAssetID<Slush::EntityPrefab>()))
			myPlayerEntityPrefab.Set(static_cast<Slush::EntityPrefab*>(asset));

		ImGui::EndDragDropTarget();
	}

	if (ImGui::CollapsingHeader("Enemy Waves"))
	{
		for (int i = 0; i < myEnemyWaves.Count(); ++i)
		{
			EnemyWaveData& waveData = myEnemyWaves[i];

			ImGui::PushID(i);
			if (ImGui::TreeNode("Wave"))
			{
				ImGui::InputFloat("Duration", &waveData.myDuration);
				ImGui::InputInt("Min Enemy Count", &waveData.myMinEnemyCount);
				ImGui::InputInt("Max Enemy Count", &waveData.myMaxEnemyCount);

				if (ImGui::TreeNode("Enemy Prefas"))
				{
					for (int j = 0; j < waveData.myEnemyPrefabs.Count(); ++j)
					{
						ImGui::PushID(j);
						if (ImGui::Button("-", { 25, 0 }))
						{
							waveData.myEnemyPrefabs.RemoveNonCyclicAtIndex(j);
							--j;
							ImGui::PopID();
							continue;
						}

						ImGui::SameLine();
						ImGui::Text("%s", waveData.myEnemyPrefabs[j].GetName().GetBuffer());
						if (ImGui::BeginDragDropTarget())
						{
							if (Slush::Asset* asset = ImGui::AcceptDraggedAsset(Slush::GetAssetID<Slush::EntityPrefab>()))
								waveData.myEnemyPrefabs[j].Set(static_cast<Slush::EntityPrefab*>(asset));

							ImGui::EndDragDropTarget();
						}

						ImGui::PopID();
					}

					if (ImGui::Button("Add Enemy.."))
						waveData.myEnemyPrefabs.Add();

					ImGui::TreePop();
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}

		if (ImGui::Button("Add Wave.."))
			myEnemyWaves.Add();
	}
}

void LevelData::OnLoad(Slush::AssetParser::Handle aRootHandle)
{
	aRootHandle.ParseVec2fField("playerStartPosition", myPlayerStartPosition);
	myPlayerEntityPrefab.Parse(aRootHandle, "playerEntityPrefab");

	Slush::AssetParser::Handle enemyWavesHandle = aRootHandle.ParseChildElement("enemyWaves");
	if (enemyWavesHandle.IsValid())
	{
		int numWaves = enemyWavesHandle.GetNumChildElements();
		myEnemyWaves.Reserve(numWaves);

		for (int i = 0; i < numWaves; ++i)
		{
			Slush::AssetParser::Handle waveHandle = enemyWavesHandle.GetChildElementAtIndex(i);

			EnemyWaveData& waveData = myEnemyWaves[i];
			waveHandle.ParseFloatField("duration", waveData.myDuration);
			waveHandle.ParseIntField("minEnemyCount", waveData.myMinEnemyCount);
			waveHandle.ParseIntField("maxEnemyCount", waveData.myMaxEnemyCount);

			Slush::AssetParser::Handle prefabHandle = waveHandle.ParseChildElement("enemyPrefabs");
			int numEnemyPrefabs = prefabHandle.GetNumFields();
			waveData.myEnemyPrefabs.Reserve(numEnemyPrefabs);
			for (int j = 0; j < numEnemyPrefabs; ++j)
				waveData.myEnemyPrefabs[j].ParseAtIndex(prefabHandle, "prefab", j);
		}
	}
}

void LevelData::OnSave(Slush::AssetParser::Handle aRootHandle)
{
	aRootHandle.ParseVec2fField("playerStartPosition", myPlayerStartPosition);
	myPlayerEntityPrefab.Parse(aRootHandle, "playerEntityPrefab");

	Slush::AssetParser::Handle enemyWavesHandle = aRootHandle.ParseChildElement("enemyWaves");
	if (enemyWavesHandle.IsValid())
	{
		for (EnemyWaveData& waveData : myEnemyWaves)
		{
			Slush::AssetParser::Handle waveHandle = enemyWavesHandle.ParseChildElement("wave");

			waveHandle.ParseFloatField("duration", waveData.myDuration);
			waveHandle.ParseIntField("minEnemyCount", waveData.myMinEnemyCount);
			waveHandle.ParseIntField("maxEnemyCount", waveData.myMaxEnemyCount);

			Slush::AssetParser::Handle prefabHandle = waveHandle.ParseChildElement("enemyPrefabs");
			for (Slush::AssetReference<Slush::EntityPrefab>& prefab : waveData.myEnemyPrefabs)
				prefab.Parse(prefabHandle, "prefab");
		}
	}
}
