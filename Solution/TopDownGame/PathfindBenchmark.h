#pragma once

namespace PathfindBenchmark
{
	// Times Navmesh::FindPath on level_main's navmesh, with and without four towers around its first
	// turn, and writes the results to Debug/pathfind_benchmark.txt. Runs before Engine::Initialize,
	// like the unit tests, so it needs no window. Returns false if the navmesh couldn't be loaded or
	// a path that should exist wasn't found.
	bool Run();
}
