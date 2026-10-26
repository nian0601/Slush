#include "stdafx.h"

#include "NavmeshTestSuite.h"
#include "Navmesh.h"
#include "Level/NavmeshData.h"

#include <Core/AssetRepository.h>

#include <FW_UnitTestSuite.h>

namespace NavmeshTestSuite
{
	FW_GrowingArray<Vector2f> MakeBox(float aLeft, float aTop, float aRight, float aBottom)
	{
		FW_GrowingArray<Vector2f> box;
		box.Add({ aLeft, aTop });
		box.Add({ aRight, aTop });
		box.Add({ aRight, aBottom });
		box.Add({ aLeft, aBottom });
		return box;
	}

	void CheckMeshGeometry(const Navmesh& aMesh)
	{
		Navmesh::TriangleMesh triangles;
		aMesh.GetAsTriangleMesh(triangles);
		for (int i = 0; i < triangles.myVertices.Count(); i += 3)
		{
			const float area = fabsf(Cross(triangles.myVertices[i + 1] - triangles.myVertices[i],
				triangles.myVertices[i + 2] - triangles.myVertices[i]));
			FW_TEST_CHECK(area > 0.000001f, "Cuts must not create degenerate triangles");
		}
	}

	void TestBoundaryContactAndOverlap()
	{
		Navmesh mesh;
		mesh.GenerateDefaultGrid();
		FW_TEST_CHECK(mesh.IsAreaFullyOnMesh(MakeBox(20.f, 20.f, 100.f, 100.f)), "Outer corner and edges may touch");
		FW_TEST_CHECK(!mesh.IsAreaFullyOnMesh(MakeBox(19.9f, 20.f, 100.f, 100.f)), "Footprint must stay inside outer boundary");
		mesh.CutHole(MakeBox(500.f, 300.f, 700.f, 500.f));
		FW_TEST_CHECK(mesh.IsAreaFullyOnMesh(MakeBox(400.f, 350.f, 500.f, 450.f)), "Partial collinear hole-edge contact is allowed");
		FW_TEST_CHECK(mesh.IsAreaFullyOnMesh(MakeBox(400.f, 200.f, 500.f, 300.f)), "Hole corner contact is allowed");
		FW_TEST_CHECK(!mesh.IsAreaFullyOnMesh(MakeBox(400.f, 350.f, 500.01f, 450.f)), "Small positive overlap is still rejected");
		FW_TEST_CHECK(!mesh.IsAreaFullyOnMesh(MakeBox(500.f, 300.f, 700.f, 500.f)), "Matching a hole is blocked even with all corners on its boundary");
		FW_TEST_CHECK(!mesh.IsAreaFullyOnMesh(MakeBox(450.f, 250.f, 750.f, 550.f)), "Enclosed hole is blocked");

		Navmesh concave;
		concave.GenerateDefaultGrid();
		concave.CutHole(MakeBox(500.f, -20.f, 550.f, 400.f));
		FW_TEST_CHECK(!concave.IsAreaFullyOnMesh(MakeBox(450.f, 300.f, 600.f, 450.f)), "Concave notch is blocked even with open corners and center");

		Navmesh angled;
		angled.GenerateDefaultGrid();
		FW_GrowingArray<Vector2f> slope;
		slope.Add({ 500.f, 300.f });
		slope.Add({ 700.f, 500.f });
		slope.Add({ 500.f, 500.f });
		angled.CutHole(slope);
		FW_TEST_CHECK(angled.IsAreaFullyOnMesh(MakeBox(600.f, 300.f, 650.f, 400.f)), "A box corner may touch an angled boundary");
		FW_TEST_CHECK(!angled.IsAreaFullyOnMesh(MakeBox(600.f, 300.f, 650.f, 400.1f)), "Crossing an angled boundary is rejected");
		angled.CutHole(MakeBox(600.f, 300.f, 650.f, 400.f));
		CheckMeshGeometry(angled);
	}

	void TestTouchingCutsBlockPathsWithoutSeams()
	{
		Navmesh mesh;
		mesh.GenerateDefaultGrid();
		// Cover the complete height with accepted, edge-touching rectangles.
		for (int i = 0; i < 7; ++i)
		{
			const auto box = MakeBox(700.f, 20.f + i * 128.f, 748.f, 148.f + i * 128.f);
			FW_TEST_REQUIRE(mesh.IsAreaFullyOnMesh(box), "Every adjacent wall piece must be placeable");
			mesh.CutHole(box);
			FW_TEST_CHECK(!mesh.IsAreaFullyOnMesh(box), "Cut footprint must become blocked");
			CheckMeshGeometry(mesh);
		}
		FW_GrowingArray<Vector2f> path;
		FW_TEST_CHECK(!mesh.FindPath({ 600.f, 468.f }, { 850.f, 468.f }, path), "A touching wall must have no traversable seam");

		Navmesh corners;
		corners.GenerateDefaultGrid();
		corners.CutHole(MakeBox(500.f, 300.f, 550.f, 350.f));
		const auto touching = MakeBox(550.f, 350.f, 600.f, 400.f);
		FW_TEST_REQUIRE(corners.IsAreaFullyOnMesh(touching), "Corner-touching cut must be placeable");
		corners.CutHole(touching);
		FW_TEST_CHECK(!corners.IsAreaFullyOnMesh(touching), "Corner-touching cut must remove its full footprint");
		CheckMeshGeometry(corners);

		Navmesh precision;
		precision.GenerateDefaultGrid();
		precision.CutHole(MakeBox(500.f, 300.f, 550.f, 350.f));
		precision.CutHole(MakeBox(550.f, 300.3f, 600.f, 349.7f));
		FW_TEST_CHECK(precision.HasVertexNear({ 550.f, 300.3f }, 0.001f), "Sub-unit partial contact must keep the exact new endpoint");
		FW_TEST_CHECK(precision.HasVertexNear({ 550.f, 300.f }, 0.001f), "Sub-unit contact must preserve the previous cut endpoint");
		CheckMeshGeometry(precision);
	}

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
		bool found = mesh.FindPath(start, goal, waypoints, &corridor);

		FW_TEST_REQUIRE(found, "Expected FindPath to succeed between two reachable points");
		FW_TEST_REQUIRE(waypoints.Count() >= 2, "Expected at least a start and goal waypoint");
		FW_TEST_CHECK(waypoints[0] == start, "Expected first waypoint to be the start position");
		FW_TEST_CHECK(waypoints.GetLast() == goal, "Expected last waypoint to be the goal position");
	}

	void TestFindPathFailsOutsideMesh()
	{
		Navmesh mesh;
		mesh.GenerateDefaultGrid();

		Vector2f start{ -100.f, -100.f };
		Vector2f goal{ 50.f, 50.f };

		FW_GrowingArray<Vector2f> waypoints;
		Navmesh::PathCorridor corridor;
		bool found = mesh.FindPath(start, goal, waypoints, &corridor);

		FW_TEST_CHECK(!found, "Expected FindPath to fail when the start position is outside the mesh");
		FW_TEST_CHECK(waypoints.IsEmpty(), "Expected no waypoints for a failed path");
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
		bool found = mesh.FindPath(start, goal, waypoints, &corridor);

		FW_TEST_CHECK(!found, "Expected FindPath to fail once a full-width cut disconnects start from goal");
	}

	void TestIsAreaFullyOnMesh()
	{
		Navmesh mesh;
		mesh.GenerateDefaultGrid();

		FW_GrowingArray<Vector2f> openArea;
		openArea.Add(Vector2f{ 100.f, 100.f });
		openArea.Add(Vector2f{ 200.f, 100.f });
		openArea.Add(Vector2f{ 200.f, 200.f });
		openArea.Add(Vector2f{ 100.f, 200.f });
		FW_TEST_CHECK(mesh.IsAreaFullyOnMesh(openArea), "Expected an area entirely on open mesh to be accepted");

		FW_GrowingArray<Vector2f> cutArea;
		cutArea.Add(Vector2f{ 500.f, 300.f });
		cutArea.Add(Vector2f{ 700.f, 300.f });
		cutArea.Add(Vector2f{ 700.f, 500.f });
		cutArea.Add(Vector2f{ 500.f, 500.f });
		mesh.CutHole(cutArea);

		FW_TEST_CHECK(!mesh.IsAreaFullyOnMesh(cutArea), "Expected an area entirely overlapping a cut hole to be rejected");

		FW_GrowingArray<Vector2f> straddlingArea;
		straddlingArea.Add(Vector2f{ 450.f, 350.f });
		straddlingArea.Add(Vector2f{ 550.f, 350.f });
		straddlingArea.Add(Vector2f{ 550.f, 450.f });
		straddlingArea.Add(Vector2f{ 450.f, 450.f });
		FW_TEST_CHECK(!mesh.IsAreaFullyOnMesh(straddlingArea), "Expected an area straddling a cut-hole boundary to be rejected");

		FW_GrowingArray<Vector2f> secondCutArea;
		secondCutArea.Add(Vector2f{ 650.f, 400.f });
		secondCutArea.Add(Vector2f{ 800.f, 400.f });
		secondCutArea.Add(Vector2f{ 800.f, 550.f });
		secondCutArea.Add(Vector2f{ 650.f, 550.f });
		FW_TEST_CHECK(!mesh.IsAreaFullyOnMesh(secondCutArea), "Expected an area overlapping a previously cut area to be rejected");

		Navmesh nestedHoleMesh;
		nestedHoleMesh.GenerateDefaultGrid();
		FW_GrowingArray<Vector2f> nestedHole;
		nestedHole.Add(Vector2f{ 600.f, 400.f });
		nestedHole.Add(Vector2f{ 650.f, 400.f });
		nestedHole.Add(Vector2f{ 650.f, 450.f });
		nestedHole.Add(Vector2f{ 600.f, 450.f });
		nestedHoleMesh.CutHole(nestedHole);

		FW_GrowingArray<Vector2f> enclosingArea;
		enclosingArea.Add(Vector2f{ 550.f, 350.f });
		enclosingArea.Add(Vector2f{ 700.f, 350.f });
		enclosingArea.Add(Vector2f{ 700.f, 500.f });
		enclosingArea.Add(Vector2f{ 550.f, 500.f });
		FW_TEST_CHECK(!nestedHoleMesh.IsAreaFullyOnMesh(enclosingArea), "Expected an area enclosing a cut hole to be rejected");
	}

	void TestFindPathSameTriangleYieldsStraightPath()
	{
		Navmesh mesh;
		mesh.GenerateDefaultGrid();

		Vector2f start{ 30.f, 30.f };
		Vector2f goal{ 40.f, 35.f };

		FW_GrowingArray<Vector2f> waypoints;
		Navmesh::PathCorridor corridor;
		bool found = mesh.FindPath(start, goal, waypoints, &corridor);

		FW_TEST_REQUIRE(found, "Expected FindPath to succeed for a same-triangle start/goal");
		FW_TEST_CHECK(corridor.myPortals.IsEmpty(), "Expected a zero-portal corridor for a same-triangle path");
		FW_TEST_REQUIRE(waypoints.Count() == 2, "Expected a 2-waypoint straight path for a same-triangle start/goal");
		FW_TEST_CHECK(waypoints[0] == start, "Expected first waypoint to be the start position");
		FW_TEST_CHECK(waypoints[1] == goal, "Expected second waypoint to be the goal position");
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
		bool found = mesh.FindPath(start, goal, stringPulledWaypoints, &corridor);

		FW_TEST_REQUIRE(found, "Expected FindPath to succeed by routing around the cut");
		FW_TEST_CHECK(corridor.myPortals.Count() > 1, "Expected a multi-portal corridor bending around the cut");

		FW_GrowingArray<Vector2f> edgeCenterWaypoints;
		mesh.BuildEdgeCenterPath(start, goal, corridor, edgeCenterWaypoints);

		FW_TEST_CHECK(GetTotalPathLength(stringPulledWaypoints) <= GetTotalPathLength(edgeCenterWaypoints),
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
		bool found = mesh.FindPath(start, goal, waypoints, &corridor);
		FW_TEST_REQUIRE(found, "Expected the initial FindPath to succeed");

		// Simulate an entity that has moved partway along its route: re-funnel the same,
		// already-computed corridor from a new position without running a new search.
		Vector2f partwayPosition{ 300.f, 468.f };

		FW_GrowingArray<Vector2f> reFunneledWaypoints;
		bool reFunneled = mesh.StringPull(partwayPosition, goal, corridor, reFunneledWaypoints);

		FW_TEST_REQUIRE(reFunneled, "Expected StringPull to succeed when re-funneling a stored corridor");
		FW_TEST_REQUIRE(reFunneledWaypoints.Count() >= 2, "Expected at least a start and goal waypoint");
		FW_TEST_CHECK(reFunneledWaypoints[0] == partwayPosition, "Expected first waypoint to be the new start position");
		FW_TEST_CHECK(reFunneledWaypoints.GetLast() == goal, "Expected last waypoint to be the goal position");
	}

	void TestCutAcrossOneQuadProducesExpectedCounts()
	{
		Navmesh mesh;
		mesh.GenerateDefaultGrid();

		const int initialTriangleCount = mesh.GetTriangleCount();
		const int initialVertexCount = mesh.GetVertexCount();
		FW_TEST_CHECK(initialTriangleCount == 210, "Expected the default sector grid to produce 210 triangles");
		FW_TEST_CHECK(initialVertexCount == 128, "Expected the default sector grid to produce 128 vertices");

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

		FW_TEST_CHECK(mesh.GetTriangleCount() == initialTriangleCount + 10, "Expected the 2 diagonal crossings plus 4 precise corner insertions to add exactly 10 triangles");
		FW_TEST_CHECK(mesh.GetVertexCount() == initialVertexCount + 8, "Expected the 2 diagonal crossings plus 4 precise corner insertions to add exactly 8 vertices");
		FW_TEST_CHECK(mesh.HasVertexNear(Vector2f{ 700.f, 440.f }, 0.01f), "Expected a precise vertex at the cutter's own corner position");
	}

	void TestCutterCornerInsideTriangleInteriorProducesVertexThere()
	{
		Navmesh mesh;
		mesh.GenerateDefaultGrid();

		Vector2f cornerDeepInsideATriangle{ 45.f, 45.f };
		FW_TEST_CHECK(!mesh.HasVertexNear(cornerDeepInsideATriangle, 0.01f), "Test setup: expected no existing vertex at this position");

		FW_GrowingArray<Vector2f> cutterCorners;
		cutterCorners.Add(cornerDeepInsideATriangle);
		cutterCorners.Add(Vector2f{ 60.f, 45.f });
		cutterCorners.Add(Vector2f{ 45.f, 60.f });
		mesh.CutHole(cutterCorners);

		FW_TEST_CHECK(mesh.HasVertexNear(cornerDeepInsideATriangle, 0.01f), "Expected a precise vertex at the cutter corner that landed inside a triangle's interior");
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
		FW_TEST_CHECK(mesh.HasVertexNear(topLeft, 0.01f) && mesh.HasVertexNear(topRight, 0.01f) && mesh.HasVertexNear(bottomLeft, 0.01f),
			"Test setup: expected these to already be real navmesh vertices");

		const int initialVertexCount = mesh.GetVertexCount();

		FW_GrowingArray<Vector2f> cutterCorners;
		cutterCorners.Add(topLeft);
		cutterCorners.Add(topRight);
		cutterCorners.Add(bottomLeft);
		mesh.CutHole(cutterCorners);

		FW_TEST_CHECK(mesh.GetVertexCount() == initialVertexCount, "Expected snapping to already-existing vertices to add no new vertices");
	}

	void TestCutterCornerNearExistingEdgeSnapsOntoThatEdge()
	{
		Navmesh mesh;
		mesh.GenerateDefaultGrid();

		// A point close to (but not exactly on, and well clear of either endpoint of) one interior
		// quad's own top edge - should snap onto that edge via SplitEdgeAtPosition rather than
		// falling into the triangle-interior insertion path.
		Vector2f cornerNearEdge{ 724.f, 404.3f };
		FW_TEST_CHECK(!mesh.HasVertexNear(cornerNearEdge, 0.01f), "Test setup: expected no existing vertex at this position");

		FW_GrowingArray<Vector2f> cutterCorners;
		cutterCorners.Add(cornerNearEdge);
		cutterCorners.Add(Vector2f{ 700.f, 460.f });
		cutterCorners.Add(Vector2f{ 748.f, 460.f });
		mesh.CutHole(cutterCorners);

		FW_TEST_CHECK(mesh.HasVertexNear(cornerNearEdge, 0.01f), "Expected a vertex snapped onto the nearby edge at the cutter's own corner position");
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
		// folder-derived paths) so this test writes to its own scratch file in the Debug folder instead
		// of the real navmeshes/ folder - that folder is scanned by AssetStorage<NavmeshData>
		// on every launch, so writing there would leave this fixture behind as a permanent, visible
		// asset in the editor UI, not just a test artifact.
		FW_String testFilePathString;
		Slush::AssetRepository::GetDebugFilePath("navmesh_roundtrip_test.navmesh", testFilePathString);
		const char* testFilePath = testFilePathString.GetBuffer();

		Slush::AssetParser writer;
		Slush::AssetParser::Handle writeHandle = writer.StartWriting("NavmeshData");
		original.OnParse(writeHandle, 1);
		writer.FinishWriting(testFilePath);

		Slush::AssetParser reader;
		Slush::AssetParser::Handle readHandle = reader.Load(testFilePath);
		NavmeshData loaded("navmesh_roundtrip_test", 0);
		loaded.OnParse(readHandle, 1);

		FW_TEST_CHECK(loaded.myNavmesh.GetVertexCount() == original.myNavmesh.GetVertexCount(), "Expected matching vertex counts after a save/load round-trip");
		FW_TEST_CHECK(loaded.myNavmesh.GetTriangleCount() == original.myNavmesh.GetTriangleCount(), "Expected matching triangle counts after a save/load round-trip");

		Vector2f start{ 50.f, 50.f };
		Vector2f goal{ 1900.f, 850.f };

		FW_GrowingArray<Vector2f> originalWaypoints;
		Navmesh::PathCorridor originalCorridor;
		bool originalFound = original.myNavmesh.FindPath(start, goal, originalWaypoints, &originalCorridor);

		FW_GrowingArray<Vector2f> loadedWaypoints;
		Navmesh::PathCorridor loadedCorridor;
		bool loadedFound = loaded.myNavmesh.FindPath(start, goal, loadedWaypoints, &loadedCorridor);

		FW_TEST_REQUIRE(originalFound && loadedFound, "Expected FindPath to succeed on both the original and the round-tripped navmesh");
		FW_TEST_REQUIRE(loadedWaypoints.Count() == originalWaypoints.Count(), "Expected matching waypoint counts after a save/load round-trip");

		// Epsilon compare, not ==: field values round-trip through AssetParser's "%.3f" float
		// formatting, so a waypoint landing on a non-exact-decimal position would otherwise make
		// this fail on a correct round-trip purely from serialization precision loss.
		const float PositionEpsilon = 0.01f;
		for (int i = 0; i < originalWaypoints.Count(); ++i)
			FW_TEST_CHECK(Length2(loadedWaypoints[i] - originalWaypoints[i]) <= PositionEpsilon * PositionEpsilon, "Expected matching waypoints after a save/load round-trip");
	}

	void TestNavmeshCopyIsIndependent()
	{
		Navmesh original;
		original.GenerateDefaultGrid();

		FW_GrowingArray<Vector2f> firstHole;
		firstHole.Add(Vector2f{ 700.f, 300.f });
		firstHole.Add(Vector2f{ 800.f, 300.f });
		firstHole.Add(Vector2f{ 800.f, 400.f });
		firstHole.Add(Vector2f{ 700.f, 400.f });
		original.CutHole(firstHole);

		Navmesh copy;
		copy.CopyFrom(original);
		FW_TEST_CHECK(copy.GetVertexCount() == original.GetVertexCount(), "Expected copied navmesh vertex count to match");
		FW_TEST_CHECK(copy.GetTriangleCount() == original.GetTriangleCount(), "Expected copied navmesh triangle count to match");

		FW_GrowingArray<Vector2f> originalWaypoints;
		FW_GrowingArray<Vector2f> copyWaypoints;
		const Vector2f start{ 50.f, 50.f };
		const Vector2f goal{ 1900.f, 850.f };
		FW_TEST_CHECK(original.FindPath(start, goal, originalWaypoints) == copy.FindPath(start, goal, copyWaypoints),
			"Expected copied navmesh path query to agree with the original");
		FW_TEST_REQUIRE(copyWaypoints.Count() == originalWaypoints.Count(), "Expected matching paths on copied navmesh");
		for (int i = 0; i < originalWaypoints.Count(); ++i)
			FW_TEST_CHECK(copyWaypoints[i] == originalWaypoints[i], "Expected exact waypoint positions on copied navmesh");

		const int originalVertexCount = original.GetVertexCount();
		const int originalTriangleCount = original.GetTriangleCount();
		FW_GrowingArray<Vector2f> secondHole;
		secondHole.Add(Vector2f{ 900.f, 300.f });
		secondHole.Add(Vector2f{ 1000.f, 300.f });
		secondHole.Add(Vector2f{ 1000.f, 400.f });
		secondHole.Add(Vector2f{ 900.f, 400.f });
		copy.CutHole(secondHole);
		FW_TEST_CHECK(original.GetVertexCount() == originalVertexCount, "Expected copied navmesh cut to leave original vertices unchanged");
		FW_TEST_CHECK(original.GetTriangleCount() == originalTriangleCount, "Expected copied navmesh cut to leave original triangles unchanged");
	}

	void RunTests()
	{
		FW_UnitTestSuite::BeginSuite("Navmesh");

		FW_RUN_TEST(TestNavmeshSaveLoadRoundTrip);
		FW_RUN_TEST(TestNavmeshCopyIsIndependent);
		FW_RUN_TEST(TestCutAcrossOneQuadProducesExpectedCounts);
		FW_RUN_TEST(TestCutterCornerInsideTriangleInteriorProducesVertexThere);
		FW_RUN_TEST(TestCutterCornerNearExistingVertexSnapsInsteadOfDuplicating);
		FW_RUN_TEST(TestCutterCornerNearExistingEdgeSnapsOntoThatEdge);
		FW_RUN_TEST(TestFindPathSucceedsBetweenReachablePoints);
		FW_RUN_TEST(TestFindPathFailsOutsideMesh);
		FW_RUN_TEST(TestFindPathFailsWhenCutDisconnectsStartFromGoal);
		FW_RUN_TEST(TestIsAreaFullyOnMesh);
		FW_RUN_TEST(TestBoundaryContactAndOverlap);
		FW_RUN_TEST(TestTouchingCutsBlockPathsWithoutSeams);
		FW_RUN_TEST(TestFindPathSameTriangleYieldsStraightPath);
		FW_RUN_TEST(TestStringPullIsNoLongerThanEdgeCenterPath);
		FW_RUN_TEST(TestStringPullCanReFunnelAnExistingCorridorFromANewStart);
	}
}
