#include "stdafx.h"

#include "WaveScaling.h"

namespace WaveScaling
{
	int GetEnemyCountForWave(int aWaveIndex, int aBaseEnemyCount, int anEnemyCountPerWaveIncrement, int aMaxEnemyCountPerWave)
	{
		int count = aBaseEnemyCount + aWaveIndex * anEnemyCountPerWaveIncrement;
		return FW_Min(count, aMaxEnemyCountPerWave);
	}

	float GetDifficultyWeight(int aWaveIndex, int aDifficultyRampWaveCount)
	{
		if (aDifficultyRampWaveCount <= 0)
			return 1.f;

		float weight = static_cast<float>(aWaveIndex) / static_cast<float>(aDifficultyRampWaveCount);
		return FW_Clamp(weight, 0.f, 1.f);
	}

	int PickUnlockedEnemyIndex(const FW_GrowingArray<EnemyUnlockEntry>& anEnemyUnlocks, int aWaveIndex, float aDifficultyWeight, float aRandomRoll01)
	{
		FW_GrowingArray<int> unlockedIndices;
		for (int i = 0; i < anEnemyUnlocks.Count(); ++i)
		{
			if (anEnemyUnlocks[i].myUnlockWave <= aWaveIndex)
				unlockedIndices.Add(i);
		}

		if (unlockedIndices.IsEmpty())
			return -1;

		// Bias the uniform roll toward the last (hardest) unlocked entry as aDifficultyWeight -> 1.
		// At weight 0 this leaves the roll untouched (uniform); at weight 1 it always lands past the
		// end of the unlocked range, which the clamp below then pins to the last unlocked index.
		float biasedRoll = aRandomRoll01 + (1.f - aRandomRoll01) * aDifficultyWeight;

		int pickedSlot = static_cast<int>(biasedRoll * unlockedIndices.Count());
		pickedSlot = FW_Clamp(pickedSlot, 0, unlockedIndices.Count() - 1);

		return unlockedIndices[pickedSlot];
	}
}
