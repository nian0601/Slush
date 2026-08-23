#include "stdafx.h"

#include "NavmeshTestSuite.h"
#include "Navmesh.h"

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

		const int initialTriangleCount = mesh.GetTriangleCount();
		const int initialVertexCount = mesh.GetVertexCount();
		FW_ASSERT(initialTriangleCount == 210, "Expected the default sector grid to produce 210 triangles");
		FW_ASSERT(initialVertexCount == 128, "Expected the default sector grid to produce 128 vertices");

		// A thin band entirely inside one interior quad, crossing its diagonal edge twice (once on
		// each of the band's long sides). All 4 corners sit deep inside triangle interiors - away
		// from any existing vertex or edge - so this exercises the plain edge-crossing cut path only.
		FW_GrowingArray<Vector2f> cutterCorners;
		cutterCorners.Add(Vector2f{ 700.f, 440.f });
		cutterCorners.Add(Vector2f{ 748.f, 440.f });
		cutterCorners.Add(Vector2f{ 748.f, 496.f });
		cutterCorners.Add(Vector2f{ 700.f, 496.f });
		mesh.CutHole(cutterCorners);

		FW_ASSERT(mesh.GetTriangleCount() == initialTriangleCount + 2, "Expected the two diagonal crossings to add exactly 2 triangles");
		FW_ASSERT(mesh.GetVertexCount() == initialVertexCount + 2, "Expected the two diagonal crossings to add exactly 2 vertices");
	}

	void RunTests()
	{
		TestCutAcrossOneQuadProducesExpectedCounts();
		TestFindPathSucceedsBetweenReachablePoints();
		TestFindPathFailsOutsideMesh();
		TestFindPathFailsWhenCutDisconnectsStartFromGoal();
		TestFindPathSameTriangleYieldsStraightPath();
		TestStringPullIsNoLongerThanEdgeCenterPath();
		TestStringPullCanReFunnelAnExistingCorridorFromANewStart();
	}
}
