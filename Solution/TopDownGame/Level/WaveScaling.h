#pragma once

#include "LevelData.h"

// Pure, stateless wave-scaling math - no dependency on Level/entities, so it's testable in isolation.
namespace WaveScaling
{
	// One distinct prefab in a wave's pool, with the summed weight of every unlocked entry for it.
	struct PrefabWeight
	{
		FW_String myPrefabName;
		int myWeight = 0;
	};

	// What one wave will send, for the editor's read-only preview.
	struct WavePreview
	{
		int myEnemyCount = 0;
		int myTotalWeight = 0;
		FW_GrowingArray<int> myUnlockedIndices; // into the unlock list, in list order
		FW_GrowingArray<PrefabWeight> myPrefabWeights; // grouped per prefab name, in first-unlocked order
	};

	int GetEnemyCountForWave(int aWaveIndex, int aBaseEnemyCount, int anEnemyCountPerWaveIncrement, int aMaxEnemyCountPerWave);

	// Indices into anEnemyUnlocks of every entry unlocked at aWaveIndex, in list order.
	void GetUnlockedEnemyIndices(const FW_GrowingArray<EnemyUnlockEntry>& anEnemyUnlocks, int aWaveIndex, FW_GrowingArray<int>& someOutIndices);

	void GetWavePreview(int aWaveIndex, int aBaseEnemyCount, int anEnemyCountPerWaveIncrement, int aMaxEnemyCountPerWave,
		const FW_GrowingArray<EnemyUnlockEntry>& anEnemyUnlocks, WavePreview& anOutPreview);

	// Weighted pick over the entries unlocked at aWaveIndex. Returns the index into anEnemyUnlocks of the
	// picked entry, or -1 if none are unlocked yet. Several entries for the same prefab add up, so picking
	// per entry gives the summed-per-prefab odds without any grouping.
	// Takes the random roll as a parameter (rather than generating it internally) so callers can drive it
	// deterministically in tests instead of relying on flaky statistical assertions.
	int PickUnlockedEnemyIndex(const FW_GrowingArray<EnemyUnlockEntry>& anEnemyUnlocks, int aWaveIndex, float aRandomRoll01);
}
