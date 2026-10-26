#include "stdafx.h"

#include "LevelTestSuite.h"
#include "Level/Level.h"

#include <FW_UnitTestSuite.h>

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

	void RunTests()
	{
		FW_UnitTestSuite::BeginSuite("Level");
		FW_RUN_TEST(TestRectVsEndpointClearance);
	}
}
