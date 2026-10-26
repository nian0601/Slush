#include "stdafx.h"

#include "NavmeshTestSuite.h"
#include "Navmesh.h"
#include "Level/NavmeshData.h"

#include <Core/AssetRepository.h>

#include <FW_UnitTestSuite.h>

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
		bool found = mesh.FindPath(start, goal, 0.f, waypoints, &corridor);

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
		bool found = mesh.FindPath(start, goal, 0.f, waypoints, &corridor);

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
		bool found = mesh.FindPath(start, goal, 0.f, waypoints, &corridor);

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
		bool found = mesh.FindPath(start, goal, 0.f, waypoints, &corridor);

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
		bool found = mesh.FindPath(start, goal, 0.f, stringPulledWaypoints, &corridor);

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
		bool found = mesh.FindPath(start, goal, 0.f, waypoints, &corridor);
		FW_TEST_REQUIRE(found, "Expected the initial FindPath to succeed");

		// Simulate an entity that has moved partway along its route: re-funnel the same,
		// already-computed corridor from a new position without running a new search.
		Vector2f partwayPosition{ 300.f, 468.f };

		FW_GrowingArray<Vector2f> reFunneledWaypoints;
		bool reFunneled = mesh.StringPull(partwayPosition, goal, corridor, 0.f, reFunneledWaypoints);

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
		bool originalFound = original.myNavmesh.FindPath(start, goal, 0.f, originalWaypoints, &originalCorridor);

		FW_GrowingArray<Vector2f> loadedWaypoints;
		Navmesh::PathCorridor loadedCorridor;
		bool loadedFound = loaded.myNavmesh.FindPath(start, goal, 0.f, loadedWaypoints, &loadedCorridor);

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
		FW_TEST_CHECK(original.FindPath(start, goal, 0.f, originalWaypoints) == copy.FindPath(start, goal, 0.f, copyWaypoints),
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

	void AddRectHole(Navmesh& aMesh, float aLeft, float aTop, float aRight, float aBottom)
	{
		FW_GrowingArray<Vector2f> hole;
		hole.Add(Vector2f{ aLeft, aTop });
		hole.Add(Vector2f{ aRight, aTop });
		hole.Add(Vector2f{ aRight, aBottom });
		hole.Add(Vector2f{ aLeft, aBottom });
		aMesh.CutHole(hole);
	}

	// Writes a mesh with exactly these vertices and triangles to a scratch file and loads it, for
	// layouts GenerateDefaultGrid + CutHole can't produce.
	void LoadHandBuiltMesh(Navmesh& aMesh, const FW_GrowingArray<Vector2f>& someVertices, const FW_GrowingArray<int>& someTriangleIndices)
	{
		FW_String filePath;
		Slush::AssetRepository::GetDebugFilePath("navmesh_handbuilt_test.navmesh", filePath);

		Slush::AssetParser writer;
		Slush::AssetParser::Handle rootHandle = writer.StartWriting("NavmeshData");
		Slush::AssetParser::Handle verticesHandle = rootHandle.ParseChildElement("vertices");
		for (Vector2f vertex : someVertices)
			verticesHandle.ParseChildElement("vertex").ParseVec2fField("pos", vertex);

		Slush::AssetParser::Handle trianglesHandle = rootHandle.ParseChildElement("triangles");
		for (int i = 0; i + 2 < someTriangleIndices.Count(); i += 3)
		{
			Slush::AssetParser::Handle triangleHandle = trianglesHandle.ParseChildElement("triangle");
			int v0 = someTriangleIndices[i];
			int v1 = someTriangleIndices[i + 1];
			int v2 = someTriangleIndices[i + 2];
			triangleHandle.ParseIntField("v0", v0);
			triangleHandle.ParseIntField("v1", v1);
			triangleHandle.ParseIntField("v2", v2);
		}
		writer.FinishWriting(filePath.GetBuffer());

		Slush::AssetParser reader;
		aMesh.Load(reader.Load(filePath.GetBuffer()));
	}

	void TestFindPathRadiusBlocksNarrowDiagonalPinch()
	{
		Navmesh mesh;
		mesh.GenerateDefaultGrid();

		// Two holes spanning the full height between them, touching only diagonally: the upper
		// hole's bottom-right corner (900, 450) sits about 28 from the lower hole's top-left corner
		// (920, 470), and the pinch between them is the only way from left to right.
		AddRectHole(mesh, 600.f, -50.f, 900.f, 450.f);
		AddRectHole(mesh, 920.f, 470.f, 1200.f, 1000.f);

		const Vector2f start{ 100.f, 468.f };
		const Vector2f goal{ 1800.f, 468.f };

		FW_GrowingArray<Vector2f> waypoints;
		FW_TEST_CHECK(mesh.FindPath(start, goal, 0.f, waypoints), "Expected a point agent to pass the diagonal pinch");
		FW_TEST_CHECK(mesh.FindPath(start, goal, 8.f, waypoints), "Expected radius 8 to fit through the ~28 wide pinch");
		FW_TEST_CHECK(!mesh.FindPath(start, goal, 16.f, waypoints), "Expected radius 16 to be blocked by the ~28 wide pinch");
	}

	void TestFindPathRadiusBlocksNarrowCorridor()
	{
		Navmesh mesh;
		mesh.GenerateDefaultGrid();

		// A 24 wide, 700 long horizontal corridor between two holes that each reach past the mesh edge.
		AddRectHole(mesh, 600.f, -50.f, 1300.f, 450.f);
		AddRectHole(mesh, 600.f, 474.f, 1300.f, 1000.f);

		const Vector2f start{ 100.f, 462.f };
		const Vector2f goal{ 1800.f, 462.f };

		FW_GrowingArray<Vector2f> narrowWaypoints;
		FW_TEST_CHECK(mesh.FindPath(start, goal, 8.f, narrowWaypoints), "Expected radius 8 to fit through the 24 wide corridor");

		FW_GrowingArray<Vector2f> wideWaypoints;
		FW_TEST_CHECK(!mesh.FindPath(start, goal, 16.f, wideWaypoints), "Expected radius 16 to find no path through the 24 wide corridor");
		FW_TEST_CHECK(wideWaypoints.IsEmpty(), "Expected no waypoints for a path blocked by its radius");
	}

	void TestFindPathRadiusPassesShortOpenFloorEdge()
	{
		// A 400x400 square whose middle has two open-floor vertices only 10 apart, so every route
		// across it goes over short edges that touch no wall.
		FW_GrowingArray<Vector2f> vertices;
		vertices.Add(Vector2f{ 0.f, 0.f });
		vertices.Add(Vector2f{ 400.f, 0.f });
		vertices.Add(Vector2f{ 400.f, 400.f });
		vertices.Add(Vector2f{ 0.f, 400.f });
		vertices.Add(Vector2f{ 200.f, 195.f });
		vertices.Add(Vector2f{ 200.f, 205.f });

		const int indices[] = { 0, 1, 4,  1, 2, 5,  1, 5, 4,  0, 4, 3,  4, 5, 3,  5, 2, 3 };
		FW_GrowingArray<int> triangleIndices;
		for (int index : indices)
			triangleIndices.Add(index);

		Navmesh mesh;
		LoadHandBuiltMesh(mesh, vertices, triangleIndices);
		FW_TEST_REQUIRE(mesh.GetTriangleCount() == 6, "Test setup: expected the hand-built mesh to load");

		FW_GrowingArray<Vector2f> waypoints;
		Navmesh::PathCorridor corridor;
		FW_TEST_CHECK(mesh.FindPath(Vector2f{ 100.f, 200.f }, Vector2f{ 300.f, 200.f }, 16.f, waypoints, &corridor), "Expected radius 16 to cross short edges between open-floor vertices");
		FW_TEST_CHECK(!corridor.myPortals.IsEmpty(), "Test setup: expected the path to cross at least one edge");
	}

	void TestFindPathRadiusMatchesPointPathOnOpenGrid()
	{
		Navmesh mesh;
		mesh.GenerateDefaultGrid();

		const Vector2f start{ 100.f, 100.f };
		const Vector2f goal{ 1800.f, 800.f };

		FW_GrowingArray<Vector2f> pointWaypoints;
		FW_GrowingArray<Vector2f> radiusWaypoints;
		FW_TEST_REQUIRE(mesh.FindPath(start, goal, 0.f, pointWaypoints), "Expected a point path across the open grid");
		FW_TEST_REQUIRE(mesh.FindPath(start, goal, 16.f, radiusWaypoints), "Expected a radius 16 path across the open grid");
		FW_TEST_REQUIRE(radiusWaypoints.Count() == pointWaypoints.Count(), "Expected nothing on the open grid to block radius 16");
		for (int i = 0; i < pointWaypoints.Count(); ++i)
			FW_TEST_CHECK(radiusWaypoints[i] == pointWaypoints[i], "Expected the same waypoints as the point path");
	}

	// The default grid's outer boundary, matching GenerateDefaultGrid's offset and sector grid.
	const float GridLeft = 20.f;
	const float GridTop = 20.f;
	const float GridRight = 20.f + 15.f * 128.f;
	const float GridBottom = 20.f + 7.f * 128.f;


	// Waypoints where the path really changes direction. The funnel can also emit duplicates and
	// points along a straight run, which these tests don't count as corners.
	int CountTurns(const FW_GrowingArray<Vector2f>& someWaypoints)
	{
		const float MinTurnSin = 0.01f;

		int turnCount = 0;
		Vector2f previous = someWaypoints.IsEmpty() ? Vector2f() : someWaypoints[0];
		for (int i = 1; i < someWaypoints.Count() - 1; ++i)
		{
			const Vector2f incoming = someWaypoints[i] - previous;
			const Vector2f outgoing = someWaypoints[i + 1] - someWaypoints[i];
			if (Length(incoming) < 0.01f || Length(outgoing) < 0.01f)
				continue;

			if (fabsf(Cross(GetNormalized(incoming), GetNormalized(outgoing))) > MinTurnSin || Dot(incoming, outgoing) < 0.f)
				++turnCount;

			previous = someWaypoints[i];
		}

		return turnCount;
	}

	float GetDistanceToRect(const Vector2f& aPoint, float aLeft, float aTop, float aRight, float aBottom)
	{
		const float dx = FW_Max(FW_Max(aLeft - aPoint.x, aPoint.x - aRight), 0.f);
		const float dy = FW_Max(FW_Max(aTop - aPoint.y, aPoint.y - aBottom), 0.f);
		return Length(Vector2f{ dx, dy });
	}

	float GetDistanceToGridBoundary(const Vector2f& aPoint)
	{
		return FW_Min(FW_Min(aPoint.x - GridLeft, GridRight - aPoint.x), FW_Min(aPoint.y - GridTop, GridBottom - aPoint.y));
	}

	// Smallest distance from any point along the path (sampled every unit) to the rect hole or the
	// grid boundary.
	float GetPathClearance(const FW_GrowingArray<Vector2f>& someWaypoints, float aLeft, float aTop, float aRight, float aBottom)
	{
		float clearance = 1000000.f;
		for (int i = 0; i < someWaypoints.Count() - 1; ++i)
		{
			const Vector2f segment = someWaypoints[i + 1] - someWaypoints[i];
			const int sampleCount = FW_Max(1, static_cast<int>(Length(segment)));
			for (int sample = 0; sample <= sampleCount; ++sample)
			{
				const Vector2f point = someWaypoints[i] + segment * (static_cast<float>(sample) / sampleCount);
				clearance = FW_Min(clearance, GetDistanceToRect(point, aLeft, aTop, aRight, aBottom));
				clearance = FW_Min(clearance, GetDistanceToGridBoundary(point));
			}
		}

		return clearance;
	}

	void TestStringPullKeepsRadiusAroundHoleCorners()
	{
		Navmesh mesh;
		mesh.GenerateDefaultGrid();
		AddRectHole(mesh, 860.f, 300.f, 1100.f, 636.f);

		const Vector2f start{ 50.f, 468.f };
		const Vector2f goal{ 1900.f, 468.f };
		const float radius = 16.f;

		FW_GrowingArray<Vector2f> pointWaypoints;
		FW_GrowingArray<Vector2f> radiusWaypoints;
		FW_TEST_REQUIRE(mesh.FindPath(start, goal, 0.f, pointWaypoints), "Expected a point path around the hole");
		FW_TEST_REQUIRE(mesh.FindPath(start, goal, radius, radiusWaypoints), "Expected a radius path around the hole");

		FW_TEST_CHECK(CountTurns(pointWaypoints) > 0, "Test setup: expected the point path to turn around the hole's corners");
		FW_TEST_CHECK(CountTurns(radiusWaypoints) == CountTurns(pointWaypoints), "Expected one corner waypoint per corner the point path turns around");
		FW_TEST_CHECK(radiusWaypoints[0] == start, "Expected the start waypoint not to move");
		FW_TEST_CHECK(radiusWaypoints.GetLast() == goal, "Expected the goal waypoint not to move");
		FW_TEST_CHECK(GetPathClearance(radiusWaypoints, 860.f, 300.f, 1100.f, 636.f) >= radius - 0.05f, "Expected every point on the path to keep the radius from the hole and the boundary");
	}

	void TestStringPullKeepsRadiusAlongStraightWall()
	{
		Navmesh mesh;
		mesh.GenerateDefaultGrid();

		// A hole hanging from the top edge: the point path runs along its 700 long bottom wall.
		AddRectHole(mesh, 600.f, -50.f, 1300.f, 450.f);

		const Vector2f start{ 100.f, 300.f };
		const Vector2f goal{ 1800.f, 300.f };
		const float radius = 16.f;

		FW_GrowingArray<Vector2f> pointWaypoints;
		FW_GrowingArray<Vector2f> radiusWaypoints;
		FW_TEST_REQUIRE(mesh.FindPath(start, goal, 0.f, pointWaypoints), "Expected a point path under the hole");
		FW_TEST_REQUIRE(mesh.FindPath(start, goal, radius, radiusWaypoints), "Expected a radius path under the hole");

		FW_TEST_CHECK(GetPathClearance(pointWaypoints, 600.f, -50.f, 1300.f, 450.f) < 1.f, "Test setup: expected the point path to touch the hole's bottom wall");
		FW_TEST_CHECK(CountTurns(radiusWaypoints) == CountTurns(pointWaypoints), "Expected one corner waypoint per corner the point path turns around");
		FW_TEST_CHECK(GetPathClearance(radiusWaypoints, 600.f, -50.f, 1300.f, 450.f) >= radius - 0.05f, "Expected the path along the wall to keep the radius from it");
	}

	void TestStringPullFallsBackWhenCornerOffsetLeavesMesh()
	{
		Navmesh mesh;
		mesh.GenerateDefaultGrid();

		// A thin spike whose tip is 40 below the top edge. The path turns over the tip, and the tip's
		// offset (straight up, clamped to 3 radii = 48) would land above the mesh.
		FW_GrowingArray<Vector2f> spike;
		spike.Add(Vector2f{ 600.f, 60.f });
		spike.Add(Vector2f{ 640.f, 500.f });
		spike.Add(Vector2f{ 560.f, 500.f });
		mesh.CutHole(spike);

		const Vector2f start{ 100.f, 200.f };
		const Vector2f goal{ 1100.f, 200.f };

		FW_GrowingArray<Vector2f> pointWaypoints;
		FW_GrowingArray<Vector2f> radiusWaypoints;
		FW_TEST_REQUIRE(mesh.FindPath(start, goal, 0.f, pointWaypoints), "Expected a point path over the spike");
		FW_TEST_REQUIRE(mesh.FindPath(start, goal, 16.f, radiusWaypoints), "Expected a radius 16 path through the 40 wide gap over the spike");

		FW_TEST_CHECK(CountTurns(radiusWaypoints) == CountTurns(pointWaypoints), "Expected the same corner count as the point path");
		for (const Vector2f& waypoint : radiusWaypoints)
			FW_TEST_CHECK(GetDistanceToGridBoundary(waypoint) > 0.f, "Expected every waypoint to stay on the mesh");
	}

	void TestDistanceToNearestWall()
	{
		Navmesh mesh;
		mesh.GenerateDefaultGrid();
		AddRectHole(mesh, 860.f, 300.f, 1100.f, 636.f);

		const float Epsilon = 0.01f;
		FW_TEST_CHECK(fabsf(mesh.GetDistanceToNearestWall(Vector2f{ 400.f, 468.f }) - 380.f) < Epsilon, "Expected an open point's distance to be to the nearest outer edge (left, 380 away)");
		FW_TEST_CHECK(fabsf(mesh.GetDistanceToNearestWall(Vector2f{ 1500.f, 30.f }) - 10.f) < Epsilon, "Expected a point 10 below the top edge to be 10 from a wall");
		FW_TEST_CHECK(fabsf(mesh.GetDistanceToNearestWall(Vector2f{ 840.f, 468.f }) - 20.f) < Epsilon, "Expected a point 20 left of the hole to be 20 from a wall");
		FW_TEST_CHECK(fabsf(mesh.GetDistanceToNearestWall(Vector2f{ 1110.f, 646.f }) - Length(Vector2f{ 10.f, 10.f })) < Epsilon, "Expected a point diagonally off the hole's corner to measure to that corner");
		FW_TEST_CHECK(mesh.GetDistanceToNearestWall(Vector2f{ 1000.f, 468.f }) < 0.f, "Expected a point inside the cut hole to be off the mesh");
		FW_TEST_CHECK(mesh.GetDistanceToNearestWall(Vector2f{ -50.f, 468.f }) < 0.f, "Expected a point outside the grid to be off the mesh");
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
		FW_RUN_TEST(TestFindPathSameTriangleYieldsStraightPath);
		FW_RUN_TEST(TestStringPullIsNoLongerThanEdgeCenterPath);
		FW_RUN_TEST(TestStringPullCanReFunnelAnExistingCorridorFromANewStart);
		FW_RUN_TEST(TestFindPathRadiusBlocksNarrowDiagonalPinch);
		FW_RUN_TEST(TestFindPathRadiusBlocksNarrowCorridor);
		FW_RUN_TEST(TestFindPathRadiusPassesShortOpenFloorEdge);
		FW_RUN_TEST(TestFindPathRadiusMatchesPointPathOnOpenGrid);
		FW_RUN_TEST(TestStringPullKeepsRadiusAroundHoleCorners);
		FW_RUN_TEST(TestStringPullKeepsRadiusAlongStraightWall);
		FW_RUN_TEST(TestStringPullFallsBackWhenCornerOffsetLeavesMesh);
		FW_RUN_TEST(TestDistanceToNearestWall);
	}
}
