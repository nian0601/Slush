#pragma once

#include "LevelData.h"

// Pure, stateless wave-scaling math - no dependency on Level/entities, so it's testable in isolation.
namespace WaveScaling
{
	// What one wave will send, for the editor's read-only preview. The pool is the gated set of unlocks;
	// GetDifficultyWeight() only biases which of them gets picked, so it isn't part of the preview.
	struct WavePreview
	{
		int myEnemyCount = 0;
		FW_GrowingArray<int> myUnlockedIndices; // into the unlock list, in list order
	};

	bool AreEnemyUnlocksAscending(const FW_GrowingArray<EnemyUnlockEntry>& anEnemyUnlocks);

	int GetEnemyCountForWave(int aWaveIndex, int aBaseEnemyCount, int anEnemyCountPerWaveIncrement, int aMaxEnemyCountPerWave);
	float GetDifficultyWeight(int aWaveIndex, int aDifficultyRampWaveCount);

	// Indices into anEnemyUnlocks of every entry unlocked at aWaveIndex, in list order.
	void GetUnlockedEnemyIndices(const FW_GrowingArray<EnemyUnlockEntry>& anEnemyUnlocks, int aWaveIndex, FW_GrowingArray<int>& someOutIndices);

	void GetWavePreview(int aWaveIndex, int aBaseEnemyCount, int anEnemyCountPerWaveIncrement, int aMaxEnemyCountPerWave,
		const FW_GrowingArray<EnemyUnlockEntry>& anEnemyUnlocks, WavePreview& anOutPreview);

	// Returns the index into anEnemyUnlocks of the picked entry, or -1 if none are unlocked yet at aWaveIndex.
	// Takes the random roll as a parameter (rather than generating it internally) so callers can drive it
	// deterministically in tests instead of relying on flaky statistical assertions.
	int PickUnlockedEnemyIndex(const FW_GrowingArray<EnemyUnlockEntry>& anEnemyUnlocks, int aWaveIndex, float aDifficultyWeight, float aRandomRoll01);
}
