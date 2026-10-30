#pragma once

#include <FW_GrowingArray.h>
#include <FW_String.h>
#include <FW_Vector2.h>

class Navmesh;

// Pure helpers for the Level Editor - no asset system access, so they're testable before Engine::Initialize.
namespace LevelEditorUtils
{
	// A roster enemy's movement radius, looked up by the caller so these helpers stay asset-free.
	struct RosterRadius
	{
		FW_String myEnemyName;
		float myRadius = 0.f;
	};

	// A spawn or goal point is clear when it's on the navmesh and at least aClearance from every wall.
	// Per point, so a level with several spawns or goals can loop over it.
	bool IsEndpointClear(const Navmesh& aNavmesh, const Vector2f& aPoint, float aClearance);
	void AddEndpointProblem(const Navmesh& aNavmesh, const char* aPointLabel, const Vector2f& aPoint, float aClearance, FW_GrowingArray<FW_String>& anOutProblems);
	// One problem per roster enemy wider than the clearance: it could fail to path out of its spawn.
	void AddRosterRadiusProblems(const FW_GrowingArray<RosterRadius>& someRosterRadii, float aClearance, FW_GrowingArray<FW_String>& anOutProblems);
	void GetEndpointProblems(const Navmesh& aNavmesh, const FW_GrowingArray<Vector2f>& someSpawns, const FW_GrowingArray<Vector2f>& someGoals, float aClearance,
		const FW_GrowingArray<RosterRadius>& someRosterRadii, FW_GrowingArray<FW_String>& anOutProblems);

	enum class NameError
	{
		None,
		Empty,
		InvalidCharacters,
		LevelExists,
		NavmeshExists,
	};

	// level_<x> -> navmesh_<x>, anything else -> navmesh_<name> (matches level_main/navmesh_main).
	void GetNavmeshNameForLevel(const FW_String& aLevelName, FW_String& anOutNavmeshName);

	// Takes the existing names as input so the "already exists" checks stay testable without the asset system.
	NameError ValidateNewLevelName(const FW_String& aLevelName, const FW_GrowingArray<FW_String>& someExistingLevelNames,
		const FW_GrowingArray<FW_String>& someExistingNavmeshNames);

	const char* GetNameErrorMessage(NameError aNameError);
}
