#pragma once

#include <FW_GrowingArray.h>
#include <FW_String.h>

// Pure helpers for the Level Editor's New/Duplicate flow - no asset system access, so they're testable
// before Engine::Initialize.
namespace LevelEditorUtils
{
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
