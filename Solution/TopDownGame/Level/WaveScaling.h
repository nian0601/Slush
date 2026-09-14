#pragma once

#include "LevelData.h"

// Pure, stateless wave-scaling math - no dependency on Level/entities, so it's testable in isolation.
namespace WaveScaling
{
	int GetEnemyCountForWave(int aWaveIndex, int aBaseEnemyCount, int anEnemyCountPerWaveIncrement, int aMaxEnemyCountPerWave);
	float GetDifficultyWeight(int aWaveIndex, int aDifficultyRampWaveCount);

	// Returns the index into anEnemyUnlocks of the picked entry, or -1 if none are unlocked yet at aWaveIndex.
	// Takes the random roll as a parameter (rather than generating it internally) so callers can drive it
	// deterministically in tests instead of relying on flaky statistical assertions.
	int PickUnlockedEnemyIndex(const FW_GrowingArray<EnemyUnlockEntry>& anEnemyUnlocks, int aWaveIndex, float aDifficultyWeight, float aRandomRoll01);
}
