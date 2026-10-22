#include "stdafx.h"

#include "WaveScaling.h"

namespace WaveScaling
{
	int GetEnemyCountForWave(int aWaveIndex, int aBaseEnemyCount, int anEnemyCountPerWaveIncrement, int aMaxEnemyCountPerWave)
	{
		int count = aBaseEnemyCount + aWaveIndex * anEnemyCountPerWaveIncrement;
		return FW_Min(count, aMaxEnemyCountPerWave);
	}

	void GetUnlockedEnemyIndices(const FW_GrowingArray<EnemyUnlockEntry>& anEnemyUnlocks, int aWaveIndex, FW_GrowingArray<int>& someOutIndices)
	{
		someOutIndices.RemoveAll();
		for (int i = 0; i < anEnemyUnlocks.Count(); ++i)
		{
			if (anEnemyUnlocks[i].myUnlockWave <= aWaveIndex)
				someOutIndices.Add(i);
		}
	}

	void GetWavePreview(int aWaveIndex, int aBaseEnemyCount, int anEnemyCountPerWaveIncrement, int aMaxEnemyCountPerWave,
		const FW_GrowingArray<EnemyUnlockEntry>& anEnemyUnlocks, WavePreview& anOutPreview)
	{
		anOutPreview.myEnemyCount = GetEnemyCountForWave(aWaveIndex, aBaseEnemyCount, anEnemyCountPerWaveIncrement, aMaxEnemyCountPerWave);
		GetUnlockedEnemyIndices(anEnemyUnlocks, aWaveIndex, anOutPreview.myUnlockedIndices);

		anOutPreview.myTotalWeight = 0;
		anOutPreview.myPrefabWeights.RemoveAll();
		for (int unlockIndex : anOutPreview.myUnlockedIndices)
		{
			const EnemyUnlockEntry& unlock = anEnemyUnlocks[unlockIndex];
			anOutPreview.myTotalWeight += unlock.myWeight;

			PrefabWeight* group = nullptr;
			for (PrefabWeight& prefabWeight : anOutPreview.myPrefabWeights)
			{
				if (prefabWeight.myPrefabName == unlock.myEnemyPrefab.GetName())
				{
					group = &prefabWeight;
					break;
				}
			}

			if (!group)
			{
				group = &anOutPreview.myPrefabWeights.Add();
				group->myPrefabName = unlock.myEnemyPrefab.GetName();
				group->myWeight = 0;
			}

			group->myWeight += unlock.myWeight;
		}
	}

	int PickUnlockedEnemyIndex(const FW_GrowingArray<EnemyUnlockEntry>& anEnemyUnlocks, int aWaveIndex, float aRandomRoll01)
	{
		int totalWeight = 0;
		int lastUnlockedIndex = -1;
		for (int i = 0; i < anEnemyUnlocks.Count(); ++i)
		{
			if (anEnemyUnlocks[i].myUnlockWave <= aWaveIndex)
			{
				totalWeight += anEnemyUnlocks[i].myWeight;
				lastUnlockedIndex = i;
			}
		}

		if (lastUnlockedIndex < 0)
			return -1;

		float scaledRoll = aRandomRoll01 * static_cast<float>(totalWeight);
		int accumulatedWeight = 0;
		for (int i = 0; i < anEnemyUnlocks.Count(); ++i)
		{
			if (anEnemyUnlocks[i].myUnlockWave > aWaveIndex)
				continue;

			accumulatedWeight += anEnemyUnlocks[i].myWeight;
			if (scaledRoll < static_cast<float>(accumulatedWeight))
				return i;
		}

		// Float rounding on a roll just under 1 can land exactly on the total; that belongs to the last entry
		return lastUnlockedIndex;
	}
}
