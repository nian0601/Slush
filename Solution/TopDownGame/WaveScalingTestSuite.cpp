#include "stdafx.h"

#include "WaveScalingTestSuite.h"
#include "Level/WaveScaling.h"

namespace WaveScalingTestSuite
{
	void TestGetEnemyCountForWaveScalesLinearlyAndRespectsCap()
	{
		FW_ASSERT(WaveScaling::GetEnemyCountForWave(0, 3, 1, 20) == 3, "Expected wave 0 to spawn the base enemy count");
		FW_ASSERT(WaveScaling::GetEnemyCountForWave(1, 3, 1, 20) == 4, "Expected the enemy count to increase linearly with wave index");
		FW_ASSERT(WaveScaling::GetEnemyCountForWave(5, 3, 1, 20) == 8, "Expected the enemy count to keep scaling linearly across several waves");
		FW_ASSERT(WaveScaling::GetEnemyCountForWave(100, 3, 1, 20) == 20, "Expected the enemy count to be capped at the configured maximum");
	}

	void TestGetDifficultyWeightRampsAndClamps()
	{
		FW_ASSERT(WaveScaling::GetDifficultyWeight(0, 10) == 0.f, "Expected wave 0 to have zero difficulty weight");
		FW_ASSERT(WaveScaling::GetDifficultyWeight(5, 10) == 0.5f, "Expected the difficulty weight to ramp linearly partway through the ramp window");
		FW_ASSERT(WaveScaling::GetDifficultyWeight(10, 10) == 1.f, "Expected the difficulty weight to reach 1 exactly at the end of the ramp window");
		FW_ASSERT(WaveScaling::GetDifficultyWeight(20, 10) == 1.f, "Expected the difficulty weight to clamp at 1 past the ramp window");
	}

	FW_GrowingArray<EnemyUnlockEntry> MakeThreeUnlocks()
	{
		FW_GrowingArray<EnemyUnlockEntry> unlocks;
		unlocks.Add().myUnlockWave = 0;
		unlocks.Add().myUnlockWave = 2;
		unlocks.Add().myUnlockWave = 4;
		return unlocks;
	}

	void TestPickUnlockedEnemyIndexNeverReturnsAboveUnlockThreshold()
	{
		FW_GrowingArray<EnemyUnlockEntry> unlocks = MakeThreeUnlocks();

		// At wave 1, only the wave-0 entry (index 0) is unlocked - the wave-2 and wave-4 entries aren't yet.
		FW_ASSERT(WaveScaling::PickUnlockedEnemyIndex(unlocks, 1, 0.f, 0.f) == 0, "Expected only the wave-0 entry to be pickable before its wave-2 sibling unlocks");
		FW_ASSERT(WaveScaling::PickUnlockedEnemyIndex(unlocks, 1, 1.f, 0.99f) == 0, "Expected only the wave-0 entry to be pickable regardless of weight/roll before wave 2");

		// At wave -1 (below every unlock threshold), nothing is unlocked.
		FW_ASSERT(WaveScaling::PickUnlockedEnemyIndex(unlocks, -1, 0.f, 0.5f) == -1, "Expected no pickable entry below every unlock threshold");
	}

	void TestPickUnlockedEnemyIndexUniformAtZeroWeightAndLastAtOneWeight()
	{
		FW_GrowingArray<EnemyUnlockEntry> unlocks = MakeThreeUnlocks();

		// At wave 4, all three entries are unlocked (indices 0, 1, 2).
		FW_ASSERT(WaveScaling::PickUnlockedEnemyIndex(unlocks, 4, 0.f, 0.0f) == 0, "Expected a zero roll at zero weight to pick the first unlocked entry");
		FW_ASSERT(WaveScaling::PickUnlockedEnemyIndex(unlocks, 4, 0.f, 0.4f) == 1, "Expected a mid roll at zero weight to pick the middle unlocked entry");
		FW_ASSERT(WaveScaling::PickUnlockedEnemyIndex(unlocks, 4, 0.f, 0.8f) == 2, "Expected a high roll at zero weight to pick the last unlocked entry");
		FW_ASSERT(WaveScaling::PickUnlockedEnemyIndex(unlocks, 4, 0.f, 0.999f) == 2, "Expected a roll near 1 at zero weight to still pick the last unlocked entry rather than overrun");

		FW_ASSERT(WaveScaling::PickUnlockedEnemyIndex(unlocks, 4, 1.f, 0.0f) == 2, "Expected weight 1 to always pick the last unlocked entry, even with a zero roll");
		FW_ASSERT(WaveScaling::PickUnlockedEnemyIndex(unlocks, 4, 1.f, 0.5f) == 2, "Expected weight 1 to always pick the last unlocked entry, even with a mid roll");
		FW_ASSERT(WaveScaling::PickUnlockedEnemyIndex(unlocks, 4, 1.f, 0.999f) == 2, "Expected weight 1 to always pick the last unlocked entry, even with a near-1 roll");
	}

	void RunTests()
	{
		TestGetEnemyCountForWaveScalesLinearlyAndRespectsCap();
		TestGetDifficultyWeightRampsAndClamps();
		TestPickUnlockedEnemyIndexNeverReturnsAboveUnlockThreshold();
		TestPickUnlockedEnemyIndexUniformAtZeroWeightAndLastAtOneWeight();
	}
}
