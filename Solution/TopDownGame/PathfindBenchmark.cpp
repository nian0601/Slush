#include "stdafx.h"

#include "PathfindBenchmark.h"
#include "Navmesh.h"
#include "Level/NavmeshData.h"

#include <Core/AssetRepository.h>
#include <Core/Time.h>

#include <stdio.h>

namespace PathfindBenchmark
{
	namespace
	{
		// level_main's start and goal, at either end of its U-shaped route.
		const Vector2f StartPosition{ 136.14f, 85.67f };
		const Vector2f GoalPosition{ 1811.93f, 101.118f };

		const float TowerHalfSize = 24.f;
		const int RunsPerCase = 20;

		struct Timing
		{
			float myAverageMs = 0.f;
			float myMaxMs = 0.f;
		};

		struct CaseResult
		{
			// Cold: the first query after the mesh changed (a tower was placed), so nothing is cached yet.
			// Warm: later queries on the same mesh with the same radius, like every following enemy spawn.
			Timing myCold;
			Timing myWarm;
			int myWaypointCount = 0;
			bool myFoundPath = true;
		};

		void PlaceTower(Navmesh& aNavmesh, const Vector2f& aPosition)
		{
			FW_GrowingArray<Vector2f> footprint;
			footprint.Add(Vector2f{ aPosition.x - TowerHalfSize, aPosition.y - TowerHalfSize });
			footprint.Add(Vector2f{ aPosition.x + TowerHalfSize, aPosition.y - TowerHalfSize });
			footprint.Add(Vector2f{ aPosition.x + TowerHalfSize, aPosition.y + TowerHalfSize });
			footprint.Add(Vector2f{ aPosition.x - TowerHalfSize, aPosition.y + TowerHalfSize });
			aNavmesh.CutHole(footprint);
		}

		void AddSample(Slush::Time::TimeUnit anElapsed, Timing& ioTiming)
		{
			const float elapsedMs = anElapsed / 1000.f;
			ioTiming.myAverageMs += elapsedMs / RunsPerCase;
			ioTiming.myMaxMs = FW_Max(ioTiming.myMaxMs, elapsedMs);
		}

		CaseResult TimeFindPath(const Navmesh& aNavmesh, float anAgentRadius)
		{
			CaseResult result;
			FW_GrowingArray<Vector2f> waypoints;
			Navmesh copy;
			for (int i = 0; i < RunsPerCase; ++i)
			{
				// A fresh copy has nothing cached, like a navmesh that just had a tower cut into it.
				copy.CopyFrom(aNavmesh);

				Slush::Time::TimeUnit startTime = Slush::Time::GetCurrentExactTime();
				result.myFoundPath &= copy.FindPath(StartPosition, GoalPosition, anAgentRadius, waypoints);
				AddSample(Slush::Time::GetCurrentExactTime() - startTime, result.myCold);

				startTime = Slush::Time::GetCurrentExactTime();
				result.myFoundPath &= copy.FindPath(StartPosition, GoalPosition, anAgentRadius, waypoints);
				AddSample(Slush::Time::GetCurrentExactTime() - startTime, result.myWarm);
			}

			result.myWaypointCount = waypoints.Count();
			return result;
		}

		void WriteCase(FILE* aFile, const char* aName, const Navmesh& aNavmesh, float anAgentRadius, bool& ioAllFound)
		{
			const CaseResult result = TimeFindPath(aNavmesh, anAgentRadius);
			ioAllFound = ioAllFound && result.myFoundPath;
			fprintf(aFile, "%-12s radius %4.1f  triangles %4d  cold avg %7.3f ms  max %7.3f ms  warm avg %7.3f ms  max %7.3f ms  waypoints %d%s\n",
				aName, anAgentRadius, aNavmesh.GetTriangleCount(), result.myCold.myAverageMs, result.myCold.myMaxMs,
				result.myWarm.myAverageMs, result.myWarm.myMaxMs, result.myWaypointCount, result.myFoundPath ? "" : "  NO PATH");
		}
	}

	bool Run()
	{
		Slush::Time::Init();

		FW_String navmeshPath;
		if (!Slush::AssetRepository::FindAssetFile("navmeshes/navmesh_main.navmesh", navmeshPath))
			return false;

		Slush::AssetParser reader;
		Slush::AssetParser::Handle readHandle = reader.Load(navmeshPath.GetBuffer());
		NavmeshData navmeshData("navmesh_main", 0);
		navmeshData.OnParse(readHandle, 1);
		const Navmesh& openNavmesh = navmeshData.myNavmesh;

		// Four towers around the inner corner of the first turn, where the left column meets the bottom band.
		Navmesh towerNavmesh;
		towerNavmesh.CopyFrom(openNavmesh);
		PlaceTower(towerNavmesh, Vector2f{ 200.f, 480.f });
		PlaceTower(towerNavmesh, Vector2f{ 230.f, 620.f });
		PlaceTower(towerNavmesh, Vector2f{ 350.f, 700.f });
		PlaceTower(towerNavmesh, Vector2f{ 420.f, 600.f });

		FW_String reportPath;
		Slush::AssetRepository::GetDebugFilePath("pathfind_benchmark.txt", reportPath);
		FILE* file = nullptr;
		fopen_s(&file, reportPath.GetBuffer(), "w");
		if (file == nullptr)
			return false;

		fprintf(file, "FindPath on navmesh_main, start to goal, %d runs per case\n", RunsPerCase);
		bool allFound = true;
		WriteCase(file, "open", openNavmesh, 0.f, allFound);
		WriteCase(file, "open", openNavmesh, 16.f, allFound);
		WriteCase(file, "four towers", towerNavmesh, 0.f, allFound);
		WriteCase(file, "four towers", towerNavmesh, 16.f, allFound);
		fclose(file);

		return allFound;
	}
}
