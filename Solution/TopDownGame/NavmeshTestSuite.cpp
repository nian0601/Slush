#include "stdafx.h"

#include "NavmeshTestSuite.h"
#include "Navmesh.h"
#include "Level/NavmeshData.h"

namespace NavmeshTestSuite
{
	float GetTotalPathLength(const FW_GrowingArray<Vector2f>& aWaypoints)
	{
		float length = 0.f;
		for (int i = 0; i < aWaypoints.Count() - 1; ++i)
			length += Length(aWaypoints[i + 1] - aWaypoints[i]);

		return length;
	}

	void TestFindPathSucceedsBetweenReachablePoints()
	{
		Navmesh mesh;
		mesh.GenerateDefaultGrid();

		Vector2f start{ 50.f, 50.f };
		Vector2f goal{ 1900.f, 850.f };

		FW_GrowingArray<Vector2f> waypoints;
		Navmesh::PathCorridor corridor;
		bool found = mesh.FindPath(start, goal, waypoints, corridor);

		FW_ASSERT(found, "Expected FindPath to succeed between two reachable points");
		FW_ASSERT(waypoints.Count() >= 2, "Expected at least a start and goal waypoint");
		FW_ASSERT(waypoints[0] == start, "Expected first waypoint to be the start position");
		FW_ASSERT(waypoints.GetLast() == goal, "Expected last waypoint to be the goal position");
	}

	void TestFindPathFailsOutsideMesh()
	{
		Navmesh mesh;
		mesh.GenerateDefaultGrid();

		Vector2f start{ -100.f, -100.f };
		Vector2f goal{ 50.f, 50.f };

		FW_GrowingArray<Vector2f> waypoints;
		Navmesh::PathCorridor corridor;
		bool found = mesh.FindPath(start, goal, waypoints, corridor);

		FW_ASSERT(!found, "Expected FindPath to fail when the start position is outside the mesh");
		FW_ASSERT(waypoints.IsEmpty(), "Expected no waypoints for a failed path");
	}

	void TestFindPathFailsWhenCutDisconnectsStartFromGoal()
	{
		Navmesh mesh;
		mesh.GenerateDefaultGrid();

		// A horizontal band spanning wider than the mesh, cutting it into a top half and a bottom half.
		FW_GrowingArray<Vector2f> cutBand;
		cutBand.Add(Vector2f{ -50.f, 450.f });
		cutBand.Add(Vector2f{ 2000.f, 450.f });
		cutBand.Add(Vector2f{ 2000.f, 480.f });
		cutBand.Add(Vector2f{ -50.f, 480.f });
		mesh.CutHole(cutBand);

		Vector2f start{ 50.f, 100.f };
		Vector2f goal{ 1900.f, 800.f };

		FW_GrowingArray<Vector2f> waypoints;
		Navmesh::PathCorridor corridor;
		bool found = mesh.FindPath(start, goal, waypoints, corridor);

		FW_ASSERT(!found, "Expected FindPath to fail once a full-width cut disconnects start from goal");
	}

	void TestFindPathSameTriangleYieldsStraightPath()
	{
		Navmesh mesh;
		mesh.GenerateDefaultGrid();

		Vector2f start{ 30.f, 30.f };
		Vector2f goal{ 40.f, 35.f };

		FW_GrowingArray<Vector2f> waypoints;
		Navmesh::PathCorridor corridor;
		bool found = mesh.FindPath(start, goal, waypoints, corridor);

		FW_ASSERT(found, "Expected FindPath to succeed for a same-triangle start/goal");
		FW_ASSERT(corridor.myPortals.IsEmpty(), "Expected a zero-portal corridor for a same-triangle path");
		FW_ASSERT(waypoints.Count() == 2, "Expected a 2-waypoint straight path for a same-triangle start/goal");
		FW_ASSERT(waypoints[0] == start, "Expected first waypoint to be the start position");
		FW_ASSERT(waypoints[1] == goal, "Expected second waypoint to be the goal position");
	}

	void TestStringPullIsNoLongerThanEdgeCenterPath()
	{
		Navmesh mesh;
		mesh.GenerateDefaultGrid();

		// A block that leaves room above and below, so the corridor between start and goal
		// has to bend around it rather than running straight through.
		FW_GrowingArray<Vector2f> cutBlock;
		cutBlock.Add(Vector2f{ 860.f, 300.f });
		cutBlock.Add(Vector2f{ 1100.f, 300.f });
		cutBlock.Add(Vector2f{ 1100.f, 636.f });
		cutBlock.Add(Vector2f{ 860.f, 636.f });
		mesh.CutHole(cutBlock);

		Vector2f start{ 50.f, 468.f };
		Vector2f goal{ 1900.f, 468.f };

		FW_GrowingArray<Vector2f> stringPulledWaypoints;
		Navmesh::PathCorridor corridor;
		bool found = mesh.FindPath(start, goal, stringPulledWaypoints, corridor);

		FW_ASSERT(found, "Expected FindPath to succeed by routing around the cut");
		FW_ASSERT(corridor.myPortals.Count() > 1, "Expected a multi-portal corridor bending around the cut");

		FW_GrowingArray<Vector2f> edgeCenterWaypoints;
		mesh.BuildEdgeCenterPath(start, goal, corridor, edgeCenterWaypoints);

		FW_ASSERT(GetTotalPathLength(stringPulledWaypoints) <= GetTotalPathLength(edgeCenterWaypoints),
			"Expected the string-pulled path to be no longer than the edge-center path through the same corridor");
	}

	void TestStringPullCanReFunnelAnExistingCorridorFromANewStart()
	{
		Navmesh mesh;
		mesh.GenerateDefaultGrid();

		FW_GrowingArray<Vector2f> cutBlock;
		cutBlock.Add(Vector2f{ 860.f, 300.f });
		cutBlock.Add(Vector2f{ 1100.f, 300.f });
		cutBlock.Add(Vector2f{ 1100.f, 636.f });
		cutBlock.Add(Vector2f{ 860.f, 636.f });
		mesh.CutHole(cutBlock);

		Vector2f start{ 50.f, 468.f };
		Vector2f goal{ 1900.f, 468.f };

		FW_GrowingArray<Vector2f> waypoints;
		Navmesh::PathCorridor corridor;
		bool found = mesh.FindPath(start, goal, waypoints, corridor);
		FW_ASSERT(found, "Expected the initial FindPath to succeed");

		// Simulate an entity that has moved partway along its route: re-funnel the same,
		// already-computed corridor from a new position without running a new search.
		Vector2f partwayPosition{ 300.f, 468.f };

		FW_GrowingArray<Vector2f> reFunneledWaypoints;
		bool reFunneled = mesh.StringPull(partwayPosition, goal, corridor, reFunneledWaypoints);

		FW_ASSERT(reFunneled, "Expected StringPull to succeed when re-funneling a stored corridor");
		FW_ASSERT(reFunneledWaypoints.Count() >= 2, "Expected at least a start and goal waypoint");
		FW_ASSERT(reFunneledWaypoints[0] == partwayPosition, "Expected first waypoint to be the new start position");
		FW_ASSERT(reFunneledWaypoints.GetLast() == goal, "Expected last waypoint to be the goal position");
	}

	void TestCutAcrossOneQuadProducesExpectedCounts()
	{
		Navmesh mesh;
		mesh.GenerateDefaultGrid();

		const int initialTriangleCount = mesh.GetTriangleCount();
		const int initialVertexCount = mesh.GetVertexCount();
		FW_ASSERT(initialTriangleCount == 210, "Expected the default sector grid to produce 210 triangles");
		FW_ASSERT(initialVertexCount == 128, "Expected the default sector grid to produce 128 vertices");

		// A thin band entirely inside one interior quad, crossing its diagonal edge twice (once on
		// each of the band's long sides). All 4 corners sit deep inside triangle interiors - away
		// from any existing vertex or edge - so besides the 2 diagonal crossings, each of the 4
		// corners now also gets its own precise vertex inserted (the Phase 2 fix - see
		// EnsureCutterVerticesExist). Before that fix landed this produced only +2/+2; the higher
		// counts here are the new, correct behavior, not a regression.
		FW_GrowingArray<Vector2f> cutterCorners;
		cutterCorners.Add(Vector2f{ 700.f, 440.f });
		cutterCorners.Add(Vector2f{ 748.f, 440.f });
		cutterCorners.Add(Vector2f{ 748.f, 496.f });
		cutterCorners.Add(Vector2f{ 700.f, 496.f });
		mesh.CutHole(cutterCorners);

		FW_ASSERT(mesh.GetTriangleCount() == initialTriangleCount + 10, "Expected the 2 diagonal crossings plus 4 precise corner insertions to add exactly 10 triangles");
		FW_ASSERT(mesh.GetVertexCount() == initialVertexCount + 8, "Expected the 2 diagonal crossings plus 4 precise corner insertions to add exactly 8 vertices");
		FW_ASSERT(mesh.HasVertexNear(Vector2f{ 700.f, 440.f }, 0.01f), "Expected a precise vertex at the cutter's own corner position");
	}

	void TestCutterCornerInsideTriangleInteriorProducesVertexThere()
	{
		Navmesh mesh;
		mesh.GenerateDefaultGrid();

		Vector2f cornerDeepInsideATriangle{ 45.f, 45.f };
		FW_ASSERT(!mesh.HasVertexNear(cornerDeepInsideATriangle, 0.01f), "Test setup: expected no existing vertex at this position");

		FW_GrowingArray<Vector2f> cutterCorners;
		cutterCorners.Add(cornerDeepInsideATriangle);
		cutterCorners.Add(Vector2f{ 60.f, 45.f });
		cutterCorners.Add(Vector2f{ 45.f, 60.f });
		mesh.CutHole(cutterCorners);

		FW_ASSERT(mesh.HasVertexNear(cornerDeepInsideATriangle, 0.01f), "Expected a precise vertex at the cutter corner that landed inside a triangle's interior");
	}

	void TestCutterCornerNearExistingVertexSnapsInsteadOfDuplicating()
	{
		Navmesh mesh;
		mesh.GenerateDefaultGrid();

		// The exact 3 vertices of one interior quad's own upper-left triangle - already real navmesh
		// vertices, so EnsureCutterVerticesExist should snap to each rather than inserting a
		// near-duplicate for any of them.
		Vector2f topLeft{ 660.f, 404.f };
		Vector2f topRight{ 788.f, 404.f };
		Vector2f bottomLeft{ 660.f, 532.f };
		FW_ASSERT(mesh.HasVertexNear(topLeft, 0.01f) && mesh.HasVertexNear(topRight, 0.01f) && mesh.HasVertexNear(bottomLeft, 0.01f),
			"Test setup: expected these to already be real navmesh vertices");

		const int initialVertexCount = mesh.GetVertexCount();

		FW_GrowingArray<Vector2f> cutterCorners;
		cutterCorners.Add(topLeft);
		cutterCorners.Add(topRight);
		cutterCorners.Add(bottomLeft);
		mesh.CutHole(cutterCorners);

		FW_ASSERT(mesh.GetVertexCount() == initialVertexCount, "Expected snapping to already-existing vertices to add no new vertices");
	}

	void TestCutterCornerNearExistingEdgeSnapsOntoThatEdge()
	{
		Navmesh mesh;
		mesh.GenerateDefaultGrid();

		// A point close to (but not exactly on, and well clear of either endpoint of) one interior
		// quad's own top edge - should snap onto that edge via SplitEdgeAtPosition rather than
		// falling into the triangle-interior insertion path.
		Vector2f cornerNearEdge{ 724.f, 404.3f };
		FW_ASSERT(!mesh.HasVertexNear(cornerNearEdge, 0.01f), "Test setup: expected no existing vertex at this position");

		FW_GrowingArray<Vector2f> cutterCorners;
		cutterCorners.Add(cornerNearEdge);
		cutterCorners.Add(Vector2f{ 700.f, 460.f });
		cutterCorners.Add(Vector2f{ 748.f, 460.f });
		mesh.CutHole(cutterCorners);

		FW_ASSERT(mesh.HasVertexNear(cornerNearEdge, 0.01f), "Expected a vertex snapped onto the nearby edge at the cutter's own corner position");
	}

	void TestNavmeshSaveLoadRoundTrip()
	{
		NavmeshData original("navmesh_roundtrip_test", 0);
		original.myNavmesh.GenerateDefaultGrid();

		FW_GrowingArray<Vector2f> cutBlockA;
		cutBlockA.Add(Vector2f{ 100.f, 100.f });
		cutBlockA.Add(Vector2f{ 200.f, 100.f });
		cutBlockA.Add(Vector2f{ 200.f, 200.f });
		cutBlockA.Add(Vector2f{ 100.f, 200.f });
		original.myNavmesh.CutHole(cutBlockA);

		FW_GrowingArray<Vector2f> cutBlockB;
		cutBlockB.Add(Vector2f{ 900.f, 300.f });
		cutBlockB.Add(Vector2f{ 1000.f, 300.f });
		cutBlockB.Add(Vector2f{ 1000.f, 400.f });
		cutBlockB.Add(Vector2f{ 900.f, 400.f });
		original.myNavmesh.CutHole(cutBlockB);

		// Drive NavmeshData::OnParse() directly (bypassing DataAsset::Save()/Load()'s
		// folder-derived paths) so this test writes to its own scratch file under temp/ instead
		// of the real data/navmeshes/ folder - that folder is scanned by AssetStorage<NavmeshData>
		// on every launch, so writing there would leave this fixture behind as a permanent, visible
		// asset in the editor UI, not just a test artifact.
		// "temp/" already exists (FW_UnitTestSuite's own file-processor test lives there) -
		// CreateFolderIfNecessary() can't be used here anyway, since it assumes every path
		// contains a "data" segment.
		const char* testFilePath = "temp/navmesh_roundtrip_test.navmesh";

		Slush::AssetParser writer;
		Slush::AssetParser::Handle writeHandle = writer.StartWriting("NavmeshData");
		original.OnParse(writeHandle, 1);
		writer.FinishWriting(testFilePath);

		Slush::AssetParser reader;
		Slush::AssetParser::Handle readHandle = reader.Load(testFilePath);
		NavmeshData loaded("navmesh_roundtrip_test", 0);
		loaded.OnParse(readHandle, 1);

		FW_ASSERT(loaded.myNavmesh.GetVertexCount() == original.myNavmesh.GetVertexCount(), "Expected matching vertex counts after a save/load round-trip");
		FW_ASSERT(loaded.myNavmesh.GetTriangleCount() == original.myNavmesh.GetTriangleCount(), "Expected matching triangle counts after a save/load round-trip");

		Vector2f start{ 50.f, 50.f };
		Vector2f goal{ 1900.f, 850.f };

		FW_GrowingArray<Vector2f> originalWaypoints;
		Navmesh::PathCorridor originalCorridor;
		bool originalFound = original.myNavmesh.FindPath(start, goal, originalWaypoints, originalCorridor);

		FW_GrowingArray<Vector2f> loadedWaypoints;
		Navmesh::PathCorridor loadedCorridor;
		bool loadedFound = loaded.myNavmesh.FindPath(start, goal, loadedWaypoints, loadedCorridor);

		FW_ASSERT(originalFound && loadedFound, "Expected FindPath to succeed on both the original and the round-tripped navmesh");
		FW_ASSERT(loadedWaypoints.Count() == originalWaypoints.Count(), "Expected matching waypoint counts after a save/load round-trip");

		// Epsilon compare, not ==: field values round-trip through AssetParser's "%.3f" float
		// formatting, so a waypoint landing on a non-exact-decimal position would otherwise make
		// this fail on a correct round-trip purely from serialization precision loss.
		const float PositionEpsilon = 0.01f;
		for (int i = 0; i < originalWaypoints.Count(); ++i)
			FW_ASSERT(Length2(loadedWaypoints[i] - originalWaypoints[i]) <= PositionEpsilon * PositionEpsilon, "Expected matching waypoints after a save/load round-trip");
	}

	void RunTests()
	{
		TestNavmeshSaveLoadRoundTrip();
		TestCutAcrossOneQuadProducesExpectedCounts();
		TestCutterCornerInsideTriangleInteriorProducesVertexThere();
		TestCutterCornerNearExistingVertexSnapsInsteadOfDuplicating();
		TestCutterCornerNearExistingEdgeSnapsOntoThatEdge();
		TestFindPathSucceedsBetweenReachablePoints();
		TestFindPathFailsOutsideMesh();
		TestFindPathFailsWhenCutDisconnectsStartFromGoal();
		TestFindPathSameTriangleYieldsStraightPath();
		TestStringPullIsNoLongerThanEdgeCenterPath();
		TestStringPullCanReFunnelAnExistingCorridorFromANewStart();
	}
}
