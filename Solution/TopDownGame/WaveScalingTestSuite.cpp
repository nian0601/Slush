#include "stdafx.h"

#include "WaveScalingTestSuite.h"
#include "Level/WaveScaling.h"

#include <Core/AssetRepository.h>

#include <FW_UnitTestSuite.h>

namespace WaveScalingTestSuite
{
	struct NamedUnlock
	{
		const char* myPrefabName;
		int myUnlockWave;
		int myWeight;
	};

	// AssetReference only takes a name through Parse() or a real asset, and assets aren't loaded
	// during tests, so named entries round-trip through a scratch file in the Debug folder.
	void MakeNamedUnlocks(const NamedUnlock* someUnlocks, int aCount, FW_GrowingArray<EnemyUnlockEntry>& someOutUnlocks)
	{
		FW_String testFilePath;
		Slush::AssetRepository::GetDebugFilePath("wavescaling_unlocks_test.txt", testFilePath);

		Slush::AssetParser writer;
		Slush::AssetParser::Handle writeHandle = writer.StartWriting("enemyUnlocks");
		for (int i = 0; i < aCount; ++i)
		{
			FW_String prefabName = someUnlocks[i].myPrefabName;
			writeHandle.ParseChildElement("unlock").ParseStringField("enemyPrefab", prefabName);
		}
		writer.FinishWriting(testFilePath.GetBuffer());

		Slush::AssetParser reader;
		Slush::AssetParser::Handle readHandle = reader.Load(testFilePath.GetBuffer());
		someOutUnlocks.RemoveAll();
		for (int i = 0; i < aCount; ++i)
		{
			EnemyUnlockEntry& unlock = someOutUnlocks.Add();
			unlock = EnemyUnlockEntry();
			unlock.myEnemyPrefab.Parse(readHandle.GetChildElementAtIndex(i), "enemyPrefab");
			unlock.myUnlockWave = someUnlocks[i].myUnlockWave;
			unlock.myWeight = someUnlocks[i].myWeight;
		}
	}

	FW_GrowingArray<EnemyUnlockEntry> MakeThreeUnlocks()
	{
		FW_GrowingArray<EnemyUnlockEntry> unlocks;
		unlocks.Add().myUnlockWave = 0;
		unlocks.Add().myUnlockWave = 2;
		unlocks.Add().myUnlockWave = 4;
		return unlocks;
	}

	void TestGetEnemyCountForWaveScalesLinearlyAndRespectsCap()
	{
		FW_TEST_CHECK(WaveScaling::GetEnemyCountForWave(0, 3, 1, 20) == 3, "Expected wave 0 to spawn the base enemy count");
		FW_TEST_CHECK(WaveScaling::GetEnemyCountForWave(1, 3, 1, 20) == 4, "Expected the enemy count to increase linearly with wave index");
		FW_TEST_CHECK(WaveScaling::GetEnemyCountForWave(5, 3, 1, 20) == 8, "Expected the enemy count to keep scaling linearly across several waves");
		FW_TEST_CHECK(WaveScaling::GetEnemyCountForWave(100, 3, 1, 20) == 20, "Expected the enemy count to be capped at the configured maximum");
	}

	void TestPickNeverReturnsNotYetUnlockedEntries()
	{
		FW_GrowingArray<EnemyUnlockEntry> unlocks = MakeThreeUnlocks();
		unlocks[1].myWeight = 100;
		unlocks[2].myWeight = 100;

		// At wave 1, only the wave-0 entry is unlocked, however heavy the later ones are
		FW_TEST_CHECK(WaveScaling::PickUnlockedEnemyIndex(unlocks, 1, 0.f) == 0, "Expected only the wave-0 entry to be pickable before wave 2, with roll 0");
		FW_TEST_CHECK(WaveScaling::PickUnlockedEnemyIndex(unlocks, 1, 0.5f) == 0, "Expected only the wave-0 entry to be pickable before wave 2, with a mid roll");
		FW_TEST_CHECK(WaveScaling::PickUnlockedEnemyIndex(unlocks, 1, 0.9999f) == 0, "Expected only the wave-0 entry to be pickable before wave 2, with a roll near 1");
		FW_TEST_CHECK(WaveScaling::PickUnlockedEnemyIndex(unlocks, 3, 0.9999f) == 1, "Expected the wave-4 entry to stay unpickable at wave 3");
	}

	void TestPickWithNoUnlocksReturnsMinusOne()
	{
		FW_GrowingArray<EnemyUnlockEntry> unlocks;
		FW_TEST_CHECK(WaveScaling::PickUnlockedEnemyIndex(unlocks, 3, 0.5f) == -1, "Expected -1 with an empty roster");

		unlocks = MakeThreeUnlocks();
		FW_TEST_CHECK(WaveScaling::PickUnlockedEnemyIndex(unlocks, -1, 0.5f) == -1, "Expected -1 below every unlock threshold");
	}

	void TestPickEqualWeightsSplitRollEvenly()
	{
		FW_GrowingArray<EnemyUnlockEntry> unlocks = MakeThreeUnlocks();

		FW_TEST_CHECK(WaveScaling::PickUnlockedEnemyIndex(unlocks, 4, 0.f) == 0, "Expected roll 0 to pick the first unlocked entry");
		FW_TEST_CHECK(WaveScaling::PickUnlockedEnemyIndex(unlocks, 4, 0.32f) == 0, "Expected a roll in the first third to pick the first entry");
		FW_TEST_CHECK(WaveScaling::PickUnlockedEnemyIndex(unlocks, 4, 0.34f) == 1, "Expected a roll in the middle third to pick the middle entry");
		FW_TEST_CHECK(WaveScaling::PickUnlockedEnemyIndex(unlocks, 4, 0.65f) == 1, "Expected a roll in the middle third to pick the middle entry");
		FW_TEST_CHECK(WaveScaling::PickUnlockedEnemyIndex(unlocks, 4, 0.67f) == 2, "Expected a roll in the last third to pick the last entry");
		FW_TEST_CHECK(WaveScaling::PickUnlockedEnemyIndex(unlocks, 4, 0.9999f) == 2, "Expected a roll just under 1 to pick the last unlocked entry rather than overrun");
		FW_TEST_CHECK(WaveScaling::PickUnlockedEnemyIndex(unlocks, 4, 1.f) == 2, "Expected a roll of exactly 1 to clamp to the last unlocked entry");
	}

	void TestPickHeavierEntryClaimsItsShare()
	{
		// Weights 1 and 3: the second entry owns the roll range [0.25, 1)
		FW_GrowingArray<EnemyUnlockEntry> unlocks;
		unlocks.Add().myWeight = 1;
		unlocks.Add().myWeight = 3;

		FW_TEST_CHECK(WaveScaling::PickUnlockedEnemyIndex(unlocks, 0, 0.24f) == 0, "Expected a roll under 1/4 to pick the weight-1 entry");
		FW_TEST_CHECK(WaveScaling::PickUnlockedEnemyIndex(unlocks, 0, 0.26f) == 1, "Expected a roll over 1/4 to pick the weight-3 entry");
		FW_TEST_CHECK(WaveScaling::PickUnlockedEnemyIndex(unlocks, 0, 0.9999f) == 1, "Expected a roll just under 1 to pick the weight-3 entry");
	}

	void TestPickStackedEntriesMatchSummedWeight()
	{
		// A, B, B (each weight 1) should split the roll exactly like A 1, B 2
		const NamedUnlock stacked[] = { { "A", 0, 1 }, { "B", 0, 1 }, { "B", 0, 1 } };
		FW_GrowingArray<EnemyUnlockEntry> stackedUnlocks;
		MakeNamedUnlocks(stacked, 3, stackedUnlocks);

		const NamedUnlock summed[] = { { "A", 0, 1 }, { "B", 0, 2 } };
		FW_GrowingArray<EnemyUnlockEntry> summedUnlocks;
		MakeNamedUnlocks(summed, 2, summedUnlocks);

		const float rolls[] = { 0.f, 0.2f, 0.32f, 0.34f, 0.5f, 0.7f, 0.9999f };
		for (float roll : rolls)
		{
			int stackedPick = WaveScaling::PickUnlockedEnemyIndex(stackedUnlocks, 0, roll);
			int summedPick = WaveScaling::PickUnlockedEnemyIndex(summedUnlocks, 0, roll);
			FW_TEST_REQUIRE(stackedPick >= 0 && summedPick >= 0, "Expected a pick with everything unlocked");
			FW_TEST_CHECK(stackedUnlocks[stackedPick].myEnemyPrefab.GetName() == summedUnlocks[summedPick].myEnemyPrefab.GetName(), "Expected two stacked entries to pick like one entry with the summed weight");
		}
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

	void TestWavePreviewGroupsStackedEntriesPerPrefab()
	{
		const NamedUnlock named[] = { { "Slow", 0, 1 }, { "Fast", 2, 1 }, { "Normal", 2, 1 }, { "Fast", 4, 2 } };
		FW_GrowingArray<EnemyUnlockEntry> unlocks;
		MakeNamedUnlocks(named, 4, unlocks);
		WaveScaling::WavePreview preview;

		WaveScaling::GetWavePreview(1, 3, 1, 20, unlocks, preview);
		FW_TEST_REQUIRE(preview.myPrefabWeights.Count() == 1, "Expected one prefab in the pool before wave 2");
		FW_TEST_CHECK(preview.myTotalWeight == 1, "Expected the total weight of the single unlock");

		WaveScaling::GetWavePreview(4, 3, 1, 20, unlocks, preview);
		FW_TEST_REQUIRE(preview.myPrefabWeights.Count() == 3, "Expected the second Fast entry to group with the first, not add a fourth prefab");
		FW_TEST_CHECK(preview.myTotalWeight == 5, "Expected the total weight to sum every unlocked entry");
		FW_TEST_CHECK(preview.myPrefabWeights[0].myPrefabName == "Slow" && preview.myPrefabWeights[0].myWeight == 1, "Expected Slow first, weight 1");
		FW_TEST_CHECK(preview.myPrefabWeights[1].myPrefabName == "Fast" && preview.myPrefabWeights[1].myWeight == 3, "Expected Fast in first-unlocked order, its two entries summed to 3");
		FW_TEST_CHECK(preview.myPrefabWeights[2].myPrefabName == "Normal" && preview.myPrefabWeights[2].myWeight == 1, "Expected Normal last, weight 1");
	}

	void TestWavePreviewWithNoUnlocksHasEmptyPool()
	{
		FW_GrowingArray<EnemyUnlockEntry> unlocks;
		WaveScaling::WavePreview preview;
		preview.myUnlockedIndices.Add(42);
		preview.myPrefabWeights.Add().myWeight = 42;

		WaveScaling::GetWavePreview(3, 3, 1, 20, unlocks, preview);
		FW_TEST_CHECK(preview.myUnlockedIndices.IsEmpty(), "Expected an empty pool, with stale entries cleared, when there are no unlocks");
		FW_TEST_CHECK(preview.myPrefabWeights.IsEmpty(), "Expected no grouped prefabs, with stale entries cleared, when there are no unlocks");
		FW_TEST_CHECK(preview.myTotalWeight == 0, "Expected zero total weight when there are no unlocks");
		FW_TEST_CHECK(preview.myEnemyCount == 6, "Expected the count to be computed even with no unlocks");
	}

	void RunTests()
	{
		FW_UnitTestSuite::BeginSuite("WaveScaling");
		FW_RUN_TEST(TestGetEnemyCountForWaveScalesLinearlyAndRespectsCap);
		FW_RUN_TEST(TestPickNeverReturnsNotYetUnlockedEntries);
		FW_RUN_TEST(TestPickWithNoUnlocksReturnsMinusOne);
		FW_RUN_TEST(TestPickEqualWeightsSplitRollEvenly);
		FW_RUN_TEST(TestPickHeavierEntryClaimsItsShare);
		FW_RUN_TEST(TestPickStackedEntriesMatchSummedWeight);
		FW_RUN_TEST(TestWavePreviewCountIsCappedAtMax);
		FW_RUN_TEST(TestWavePreviewPoolGrowsAtEachUnlockWave);
		FW_RUN_TEST(TestWavePreviewGroupsStackedEntriesPerPrefab);
		FW_RUN_TEST(TestWavePreviewWithNoUnlocksHasEmptyPool);
	}
}
