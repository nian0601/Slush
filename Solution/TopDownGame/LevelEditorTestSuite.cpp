#include "stdafx.h"

#include "LevelEditorTestSuite.h"
#include "LevelEditorUtils.h"

#include <FW_UnitTestSuite.h>

namespace LevelEditorTestSuite
{
	void TestNavmeshNameReplacesLevelPrefix()
	{
		FW_String navmeshName;
		LevelEditorUtils::GetNavmeshNameForLevel("level_main", navmeshName);
		FW_TEST_CHECK(navmeshName == "navmesh_main", "Expected level_main to map to navmesh_main");

		LevelEditorUtils::GetNavmeshNameForLevel("level_2_copy", navmeshName);
		FW_TEST_CHECK(navmeshName == "navmesh_2_copy", "Expected only the leading level_ to be replaced");
	}

	void TestNavmeshNamePrefixesOtherNames()
	{
		FW_String navmeshName;
		LevelEditorUtils::GetNavmeshNameForLevel("forest", navmeshName);
		FW_TEST_CHECK(navmeshName == "navmesh_forest", "Expected a name without level_ to get navmesh_ prepended");

		LevelEditorUtils::GetNavmeshNameForLevel("my_level_3", navmeshName);
		FW_TEST_CHECK(navmeshName == "navmesh_my_level_3", "Expected level_ in the middle of a name to be left alone");
	}

	void TestValidateRejectsEmptyAndInvalidCharacters()
	{
		FW_GrowingArray<FW_String> noNames;
		FW_TEST_CHECK(LevelEditorUtils::ValidateNewLevelName("", noNames, noNames) == LevelEditorUtils::NameError::Empty, "Expected an empty name to be rejected");
		FW_TEST_CHECK(LevelEditorUtils::ValidateNewLevelName("level 2", noNames, noNames) == LevelEditorUtils::NameError::InvalidCharacters, "Expected a space to be rejected");
		FW_TEST_CHECK(LevelEditorUtils::ValidateNewLevelName("level/2", noNames, noNames) == LevelEditorUtils::NameError::InvalidCharacters, "Expected a path separator to be rejected");
		FW_TEST_CHECK(LevelEditorUtils::ValidateNewLevelName("Level_2-b", noNames, noNames) == LevelEditorUtils::NameError::None, "Expected letters, digits, _ and - to be accepted");
	}

	void TestValidateRejectsExistingLevelOrNavmesh()
	{
		FW_GrowingArray<FW_String> levelNames;
		levelNames.Add("level_main");
		FW_GrowingArray<FW_String> navmeshNames;
		navmeshNames.Add("navmesh_main");
		navmeshNames.Add("navmesh_taken");

		FW_TEST_CHECK(LevelEditorUtils::ValidateNewLevelName("level_main", levelNames, navmeshNames) == LevelEditorUtils::NameError::LevelExists, "Expected an existing level name to be rejected");
		FW_TEST_CHECK(LevelEditorUtils::ValidateNewLevelName("level_taken", levelNames, navmeshNames) == LevelEditorUtils::NameError::NavmeshExists, "Expected a name whose derived navmesh exists to be rejected");
		FW_TEST_CHECK(LevelEditorUtils::ValidateNewLevelName("level_2", levelNames, navmeshNames) == LevelEditorUtils::NameError::None, "Expected a free name to be accepted");
	}

	void RunTests()
	{
		FW_UnitTestSuite::BeginSuite("LevelEditor");
		FW_RUN_TEST(TestNavmeshNameReplacesLevelPrefix);
		FW_RUN_TEST(TestNavmeshNamePrefixesOtherNames);
		FW_RUN_TEST(TestValidateRejectsEmptyAndInvalidCharacters);
		FW_RUN_TEST(TestValidateRejectsExistingLevelOrNavmesh);
	}
}
