#include "stdafx.h"

#include "LevelTestSuite.h"
#include "Level/Level.h"

#include <FW_UnitTestSuite.h>
#include <Core/AssetRepository.h>

namespace LevelTestSuite
{
	void TestRectVsEndpointClearance()
	{
		// A 48x48 tower footprint from (100, 100) to (148, 148), against a clearance of 48.
		const Vector2f rectMin{ 100.f, 100.f };
		const Vector2f rectMax{ 148.f, 148.f };
		const float clearance = 48.f;

		FW_TEST_CHECK(!TowerPlacementUtils::DoesRectOverlapCircle(rectMin, rectMax, Vector2f{ 300.f, 124.f }, clearance), "Expected a point far from the rect not to overlap");
		FW_TEST_CHECK(!TowerPlacementUtils::DoesRectOverlapCircle(rectMin, rectMax, Vector2f{ 196.f, 124.f }, clearance), "Expected a circle exactly touching the rect's side to be allowed");
		FW_TEST_CHECK(TowerPlacementUtils::DoesRectOverlapCircle(rectMin, rectMax, Vector2f{ 195.f, 124.f }, clearance), "Expected a circle reaching 1 into the rect's side to overlap");
		FW_TEST_CHECK(!TowerPlacementUtils::DoesRectOverlapCircle(rectMin, rectMax, Vector2f{ 148.f + 34.f, 148.f + 34.f }, clearance), "Expected a point diagonally off the corner, 48+ away, not to overlap");
		FW_TEST_CHECK(TowerPlacementUtils::DoesRectOverlapCircle(rectMin, rectMax, Vector2f{ 148.f + 33.f, 148.f + 33.f }, clearance), "Expected a point diagonally off the corner, under 48 away, to overlap");
		FW_TEST_CHECK(TowerPlacementUtils::DoesRectOverlapCircle(rectMin, rectMax, Vector2f{ 120.f, 130.f }, clearance), "Expected a point inside the rect to overlap");
	}

	void TestLevelEndpointsLoadV7AndRoundTripV8()
	{
		FW_String path;
		Slush::AssetRepository::GetDebugFilePath("level_endpoints_test.tdlevel", path);
		{
			Slush::AssetParser writer;
			Slush::AssetParser::Handle root = writer.StartWriting("LevelData");
			int version = 7;
			root.ParseIntField("version", version);
			FW_String navmeshName = "navmesh_test";
			root.ParseStringField("navmeshData", navmeshName);
			int count = 1;
			const char* intFields[] = { "startingResources", "startingLives", "totalWaveCount", "baseEnemyCount", "enemyCountPerWaveIncrement", "maxEnemyCountPerWave" };
			for (const char* field : intFields)
				root.ParseIntField(field, count);
			float timing = 1.f;
			const char* floatFields[] = { "endpointClearance", "enemySpawnInterval", "interWaveRestDuration", "preFirstWaveGraceDuration" };
			for (const char* field : floatFields)
				root.ParseFloatField(field, timing);
			root.ParseChildElement("enemyUnlocks");
			Vector2f spawn{ 100.f, 200.f };
			Vector2f goal{ 1500.f, 600.f };
			root.ParseVec2fField("startPosition", spawn);
			root.ParseVec2fField("goalPosition", goal);
			writer.FinishWriting(path.GetBuffer());
		}

		LevelData level("test", 0);
		for (int reload = 0; reload < 2; ++reload)
		{
			Slush::AssetParser reader;
			Slush::AssetParser::Handle root = reader.Load(path.GetBuffer());
			FW_TEST_REQUIRE(root.IsValid(), "Legacy fixture loads");
			int version = 0;
			root.ParseIntField("version", version);
			level.OnParse(root, version);
			FW_TEST_REQUIRE(level.mySpawnPositions.Count() == 1 && level.myGoalPositions.Count() == 1, "V7 loads one endpoint of each kind without accumulating on reload");
			FW_TEST_CHECK(level.mySpawnPositions[0] == Vector2f(100.f, 200.f), "Legacy spawn preserved");
			FW_TEST_CHECK(level.myGoalPositions[0] == Vector2f(1500.f, 600.f), "Legacy goal preserved");
		}
		level.mySpawnPositions.Add(Vector2f{ 300.f, 400.f });
		level.myGoalPositions.Add(Vector2f{ 1700.f, 700.f });
		{
			Slush::AssetParser writer;
			Slush::AssetParser::Handle root = writer.StartWriting("LevelData");
			int version = 8;
			root.ParseIntField("version", version);
			level.OnParse(root, version);
			FW_TEST_CHECK(!root.HasChildElement("startPosition") && !root.HasChildElement("goalPosition"), "V8 writes only the lists");
			writer.FinishWriting(path.GetBuffer());
		}
		for (int reload = 0; reload < 2; ++reload)
		{
			Slush::AssetParser reader;
			Slush::AssetParser::Handle root = reader.Load(path.GetBuffer());
			FW_TEST_REQUIRE(root.IsValid(), "V8 fixture loads");
			int version = 0;
			root.ParseIntField("version", version);
			level.OnParse(root, version);
			FW_TEST_REQUIRE(level.mySpawnPositions.Count() == 2 && level.myGoalPositions.Count() == 2, "Both lists round-trip without accumulating on reload");
			FW_TEST_CHECK(level.mySpawnPositions[0] == Vector2f(100.f, 200.f) && level.mySpawnPositions[1] == Vector2f(300.f, 400.f), "Both spawn positions preserved in order");
			FW_TEST_CHECK(level.myGoalPositions[0] == Vector2f(1500.f, 600.f) && level.myGoalPositions[1] == Vector2f(1700.f, 700.f), "Both goal positions preserved in order");
		}
	}

	void RunTests()
	{
		FW_UnitTestSuite::BeginSuite("Level");
		FW_RUN_TEST(TestRectVsEndpointClearance);
		FW_RUN_TEST(TestLevelEndpointsLoadV7AndRoundTripV8);
	}
}
