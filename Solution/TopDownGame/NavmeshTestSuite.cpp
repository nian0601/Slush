#include "stdafx.h"

#include "NavmeshTestSuite.h"
#include "Navmesh.h"

namespace NavmeshTestSuite
{
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

	void RunTests()
	{
		TestFindPathSucceedsBetweenReachablePoints();
		TestFindPathFailsOutsideMesh();
		TestFindPathFailsWhenCutDisconnectsStartFromGoal();
		TestFindPathSameTriangleYieldsStraightPath();
	}
}
