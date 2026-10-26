#include "stdafx.h"

#include "LevelEditorTestSuite.h"
#include "LevelEditorUtils.h"
#include "Navmesh.h"

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

	// The default grid spans 20..1940 x 20..916; this hole leaves its left edge 20 from the point below.
	void BuildMeshWithHole(Navmesh& aMesh)
	{
		aMesh.GenerateDefaultGrid();

		FW_GrowingArray<Vector2f> hole;
		hole.Add(Vector2f{ 860.f, 300.f });
		hole.Add(Vector2f{ 1100.f, 300.f });
		hole.Add(Vector2f{ 1100.f, 636.f });
		hole.Add(Vector2f{ 860.f, 636.f });
		aMesh.CutHole(hole);
	}

	void TestEndpointProblemsAcceptClearPoints()
	{
		Navmesh mesh;
		BuildMeshWithHole(mesh);

		FW_GrowingArray<LevelEditorUtils::RosterRadius> noRoster;
		FW_GrowingArray<FW_String> problems;
		LevelEditorUtils::GetEndpointProblems(mesh, Vector2f{ 400.f, 468.f }, Vector2f{ 1500.f, 468.f }, 48.f, noRoster, problems);
		FW_TEST_CHECK(problems.IsEmpty(), "Expected start and goal far from every wall to have no problems");
		FW_TEST_CHECK(LevelEditorUtils::IsEndpointClear(mesh, Vector2f{ 400.f, 468.f }, 48.f), "Expected the start to be clear");
	}

	void TestEndpointProblemsFlagPointsTooCloseOrOffMesh()
	{
		Navmesh mesh;
		BuildMeshWithHole(mesh);

		FW_GrowingArray<LevelEditorUtils::RosterRadius> noRoster;
		FW_GrowingArray<FW_String> problems;
		LevelEditorUtils::GetEndpointProblems(mesh, Vector2f{ 840.f, 468.f }, Vector2f{ 1000.f, 468.f }, 48.f, noRoster, problems);
		FW_TEST_REQUIRE(problems.Count() == 2, "Expected one problem each for a start 20 from the hole and a goal inside it");
		FW_TEST_CHECK(problems[0] == "Start is closer than 48 to the navmesh edge. Move it.", "Expected the too-close message for the start");
		FW_TEST_CHECK(problems[1] == "Goal is off the navmesh. Move it.", "Expected the off-mesh message for the goal");
		FW_TEST_CHECK(!LevelEditorUtils::IsEndpointClear(mesh, Vector2f{ 840.f, 468.f }, 48.f), "Expected the start not to be clear");
	}

	void TestEndpointProblemsFollowClearanceChanges()
	{
		Navmesh mesh;
		BuildMeshWithHole(mesh);

		// 100 from the hole and 380 from the left edge: clear at 48, not once the clearance passes 100.
		const Vector2f start{ 760.f, 468.f };
		const Vector2f goal{ 1500.f, 468.f };

		FW_GrowingArray<LevelEditorUtils::RosterRadius> noRoster;
		FW_GrowingArray<FW_String> problems;
		LevelEditorUtils::GetEndpointProblems(mesh, start, goal, 48.f, noRoster, problems);
		FW_TEST_CHECK(problems.IsEmpty(), "Expected the start to be clear at clearance 48");

		LevelEditorUtils::GetEndpointProblems(mesh, start, goal, 120.f, noRoster, problems);
		FW_TEST_REQUIRE(problems.Count() == 1, "Expected only the start to be flagged once the clearance is raised past it");
		FW_TEST_CHECK(problems[0] == "Start is closer than 120 to the navmesh edge. Move it.", "Expected the too-close message with the raised clearance");
	}

	void TestEndpointProblemsFlagRosterRadiusOverClearance()
	{
		Navmesh mesh;
		BuildMeshWithHole(mesh);

		FW_GrowingArray<LevelEditorUtils::RosterRadius> roster;
		LevelEditorUtils::RosterRadius& small = roster.Add();
		small.myEnemyName = "Enemy_Normal";
		small.myRadius = 16.f;
		LevelEditorUtils::RosterRadius& big = roster.Add();
		big.myEnemyName = "Enemy_Big";
		big.myRadius = 60.f;

		FW_GrowingArray<FW_String> problems;
		LevelEditorUtils::GetEndpointProblems(mesh, Vector2f{ 400.f, 468.f }, Vector2f{ 1500.f, 468.f }, 48.f, roster, problems);
		FW_TEST_REQUIRE(problems.Count() == 1, "Expected only the enemy wider than the clearance to be flagged");
		FW_TEST_CHECK(problems[0] == "Enemy_Big's radius 60 is larger than the endpoint clearance 48.", "Expected the roster radius message");
	}

	void RunTests()
	{
		FW_UnitTestSuite::BeginSuite("LevelEditor");
		FW_RUN_TEST(TestNavmeshNameReplacesLevelPrefix);
		FW_RUN_TEST(TestNavmeshNamePrefixesOtherNames);
		FW_RUN_TEST(TestValidateRejectsEmptyAndInvalidCharacters);
		FW_RUN_TEST(TestValidateRejectsExistingLevelOrNavmesh);
		FW_RUN_TEST(TestEndpointProblemsAcceptClearPoints);
		FW_RUN_TEST(TestEndpointProblemsFlagPointsTooCloseOrOffMesh);
		FW_RUN_TEST(TestEndpointProblemsFollowClearanceChanges);
		FW_RUN_TEST(TestEndpointProblemsFlagRosterRadiusOverClearance);
	}
}
