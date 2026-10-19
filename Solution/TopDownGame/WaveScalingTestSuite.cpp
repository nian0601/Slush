#include "stdafx.h"

#include "WaveScalingTestSuite.h"
#include "Level/WaveScaling.h"

#include <FW_UnitTestSuite.h>

namespace WaveScalingTestSuite
{
	void TestEnemyUnlockOrderAllowsEmptySingleAscendingAndTies()
	{
		FW_GrowingArray<EnemyUnlockEntry> unlocks;
		FW_TEST_CHECK(WaveScaling::AreEnemyUnlocksAscending(unlocks), "An empty roster is ordered");
		unlocks.Add().myUnlockWave = 0;
		FW_TEST_CHECK(WaveScaling::AreEnemyUnlocksAscending(unlocks), "A single entry is ordered");
		unlocks.Add().myUnlockWave = 2;
		unlocks.Add().myUnlockWave = 4;
		FW_TEST_CHECK(WaveScaling::AreEnemyUnlocksAscending(unlocks), "Increasing unlock waves are ordered");
		unlocks.Add().myUnlockWave = 4;
		FW_TEST_CHECK(WaveScaling::AreEnemyUnlocksAscending(unlocks), "Equal unlock waves retain authored order");
	}

	void TestEnemyUnlockOrderRejectsAnyDescendingPair()
	{
		FW_GrowingArray<EnemyUnlockEntry> unlocks;
		unlocks.Add().myUnlockWave = 0;
		unlocks.Add().myUnlockWave = 4;
		unlocks.Add().myUnlockWave = 2;
		FW_TEST_CHECK(!WaveScaling::AreEnemyUnlocksAscending(unlocks), "A descending final pair must warn");
		unlocks[0].myUnlockWave = 5;
		unlocks[2].myUnlockWave = 6;
		FW_TEST_CHECK(!WaveScaling::AreEnemyUnlocksAscending(unlocks), "A descending first pair must warn even when the last pair is ordered");
	}

	void TestGetEnemyCountForWaveScalesLinearlyAndRespectsCap()
	{
		FW_TEST_CHECK(WaveScaling::GetEnemyCountForWave(0, 3, 1, 20) == 3, "Expected wave 0 to spawn the base enemy count");
		FW_TEST_CHECK(WaveScaling::GetEnemyCountForWave(1, 3, 1, 20) == 4, "Expected the enemy count to increase linearly with wave index");
		FW_TEST_CHECK(WaveScaling::GetEnemyCountForWave(5, 3, 1, 20) == 8, "Expected the enemy count to keep scaling linearly across several waves");
		FW_TEST_CHECK(WaveScaling::GetEnemyCountForWave(100, 3, 1, 20) == 20, "Expected the enemy count to be capped at the configured maximum");
	}

	void TestGetDifficultyWeightRampsAndClamps()
	{
		FW_TEST_CHECK(WaveScaling::GetDifficultyWeight(0, 10) == 0.f, "Expected wave 0 to have zero difficulty weight");
		FW_TEST_CHECK(WaveScaling::GetDifficultyWeight(5, 10) == 0.5f, "Expected the difficulty weight to ramp linearly partway through the ramp window");
		FW_TEST_CHECK(WaveScaling::GetDifficultyWeight(10, 10) == 1.f, "Expected the difficulty weight to reach 1 exactly at the end of the ramp window");
		FW_TEST_CHECK(WaveScaling::GetDifficultyWeight(20, 10) == 1.f, "Expected the difficulty weight to clamp at 1 past the ramp window");
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
		FW_TEST_CHECK(WaveScaling::PickUnlockedEnemyIndex(unlocks, 1, 0.f, 0.f) == 0, "Expected only the wave-0 entry to be pickable before its wave-2 sibling unlocks");
		FW_TEST_CHECK(WaveScaling::PickUnlockedEnemyIndex(unlocks, 1, 1.f, 0.99f) == 0, "Expected only the wave-0 entry to be pickable regardless of weight/roll before wave 2");

		// At wave -1 (below every unlock threshold), nothing is unlocked.
		FW_TEST_CHECK(WaveScaling::PickUnlockedEnemyIndex(unlocks, -1, 0.f, 0.5f) == -1, "Expected no pickable entry below every unlock threshold");
	}

	void TestPickUnlockedEnemyIndexUniformAtZeroWeightAndLastAtOneWeight()
	{
		FW_GrowingArray<EnemyUnlockEntry> unlocks = MakeThreeUnlocks();

		// At wave 4, all three entries are unlocked (indices 0, 1, 2).
		FW_TEST_CHECK(WaveScaling::PickUnlockedEnemyIndex(unlocks, 4, 0.f, 0.0f) == 0, "Expected a zero roll at zero weight to pick the first unlocked entry");
		FW_TEST_CHECK(WaveScaling::PickUnlockedEnemyIndex(unlocks, 4, 0.f, 0.4f) == 1, "Expected a mid roll at zero weight to pick the middle unlocked entry");
		FW_TEST_CHECK(WaveScaling::PickUnlockedEnemyIndex(unlocks, 4, 0.f, 0.8f) == 2, "Expected a high roll at zero weight to pick the last unlocked entry");
		FW_TEST_CHECK(WaveScaling::PickUnlockedEnemyIndex(unlocks, 4, 0.f, 0.999f) == 2, "Expected a roll near 1 at zero weight to still pick the last unlocked entry rather than overrun");

		FW_TEST_CHECK(WaveScaling::PickUnlockedEnemyIndex(unlocks, 4, 1.f, 0.0f) == 2, "Expected weight 1 to always pick the last unlocked entry, even with a zero roll");
		FW_TEST_CHECK(WaveScaling::PickUnlockedEnemyIndex(unlocks, 4, 1.f, 0.5f) == 2, "Expected weight 1 to always pick the last unlocked entry, even with a mid roll");
		FW_TEST_CHECK(WaveScaling::PickUnlockedEnemyIndex(unlocks, 4, 1.f, 0.999f) == 2, "Expected weight 1 to always pick the last unlocked entry, even with a near-1 roll");
	}

	void TestWavePreviewCountIsCappedAtMax()
	{
		FW_GrowingArray<EnemyUnlockEntry> unlocks = MakeThreeUnlocks();
		WaveScaling::WavePreview preview;

		WaveScaling::GetWavePreview(0, 3, 2, 8, unlocks, preview);
		FW_TEST_CHECK(preview.myEnemyCount == 3, "Expected wave 0 to preview the base enemy count");
		WaveScaling::GetWavePreview(2, 3, 2, 8, unlocks, preview);
		FW_TEST_CHECK(preview.myEnemyCount == 7, "Expected the preview count to scale with the increment");
		WaveScaling::GetWavePreview(9, 3, 2, 8, unlocks, preview);
		FW_TEST_CHECK(preview.myEnemyCount == 8, "Expected the preview count to be capped at the max");
	}

	void TestWavePreviewPoolGrowsAtEachUnlockWave()
	{
		FW_GrowingArray<EnemyUnlockEntry> unlocks = MakeThreeUnlocks();
		WaveScaling::WavePreview preview;

		WaveScaling::GetWavePreview(1, 3, 1, 20, unlocks, preview);
		FW_TEST_REQUIRE(preview.myUnlockedIndices.Count() == 1, "Expected only the wave-0 unlock in the pool before wave 2");
		FW_TEST_CHECK(preview.myUnlockedIndices[0] == 0, "Expected the wave-0 unlock to be the pool");

		WaveScaling::GetWavePreview(2, 3, 1, 20, unlocks, preview);
		FW_TEST_REQUIRE(preview.myUnlockedIndices.Count() == 2, "Expected the wave-2 unlock to join the pool at wave 2");
		FW_TEST_CHECK(preview.myUnlockedIndices[1] == 1, "Expected the pool to stay in list order");

		WaveScaling::GetWavePreview(4, 3, 1, 20, unlocks, preview);
		FW_TEST_CHECK(preview.myUnlockedIndices.Count() == 3, "Expected every unlock in the pool from wave 4");
	}

	void TestWavePreviewWithNoUnlocksHasEmptyPool()
	{
		FW_GrowingArray<EnemyUnlockEntry> unlocks;
		WaveScaling::WavePreview preview;
		preview.myUnlockedIndices.Add(42);

		WaveScaling::GetWavePreview(3, 3, 1, 20, unlocks, preview);
		FW_TEST_CHECK(preview.myUnlockedIndices.IsEmpty(), "Expected an empty pool, with stale entries cleared, when there are no unlocks");
		FW_TEST_CHECK(preview.myEnemyCount == 6, "Expected the count to be computed even with no unlocks");
	}

	void RunTests()
	{
		FW_UnitTestSuite::BeginSuite("WaveScaling");
		FW_RUN_TEST(TestEnemyUnlockOrderAllowsEmptySingleAscendingAndTies);
		FW_RUN_TEST(TestEnemyUnlockOrderRejectsAnyDescendingPair);

		FW_RUN_TEST(TestGetEnemyCountForWaveScalesLinearlyAndRespectsCap);
		FW_RUN_TEST(TestGetDifficultyWeightRampsAndClamps);
		FW_RUN_TEST(TestPickUnlockedEnemyIndexNeverReturnsAboveUnlockThreshold);
		FW_RUN_TEST(TestPickUnlockedEnemyIndexUniformAtZeroWeightAndLastAtOneWeight);
		FW_RUN_TEST(TestWavePreviewCountIsCappedAtMax);
		FW_RUN_TEST(TestWavePreviewPoolGrowsAtEachUnlockWave);
		FW_RUN_TEST(TestWavePreviewWithNoUnlocksHasEmptyPool);
	}
}
