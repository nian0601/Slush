#include "stdafx.h"

#include "LevelEditorUtils.h"
#include "Navmesh.h"

namespace LevelEditorUtils
{
	namespace
	{
		void AppendNumber(FW_String& aString, float aValue)
		{
			char buffer[32];
			_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, "%g", aValue);
			aString += buffer;
		}
	}

	bool IsEndpointClear(const Navmesh& aNavmesh, const Vector2f& aPoint, float aClearance)
	{
		const float distance = aNavmesh.GetDistanceToNearestWall(aPoint);
		return distance >= 0.f && distance >= aClearance;
	}

	void AddEndpointProblem(const Navmesh& aNavmesh, const char* aPointLabel, const Vector2f& aPoint, float aClearance, FW_GrowingArray<FW_String>& anOutProblems)
	{
		const float distance = aNavmesh.GetDistanceToNearestWall(aPoint);
		if (distance >= aClearance)
			return;

		FW_String& problem = anOutProblems.Add();
		problem = aPointLabel;
		if (distance < 0.f)
		{
			problem += " is off the navmesh. Move it.";
			return;
		}

		problem += " is closer than ";
		AppendNumber(problem, aClearance);
		problem += " to the navmesh edge. Move it.";
	}

	void AddRosterRadiusProblems(const FW_GrowingArray<RosterRadius>& someRosterRadii, float aClearance, FW_GrowingArray<FW_String>& anOutProblems)
	{
		for (const RosterRadius& rosterRadius : someRosterRadii)
		{
			if (rosterRadius.myRadius <= aClearance)
				continue;

			FW_String& problem = anOutProblems.Add();
			problem = rosterRadius.myEnemyName;
			problem += "'s radius ";
			AppendNumber(problem, rosterRadius.myRadius);
			problem += " is larger than the endpoint clearance ";
			AppendNumber(problem, aClearance);
			problem += ".";
		}
	}

	void GetEndpointProblems(const Navmesh& aNavmesh, const Vector2f& aStart, const Vector2f& aGoal, float aClearance,
		const FW_GrowingArray<RosterRadius>& someRosterRadii, FW_GrowingArray<FW_String>& anOutProblems)
	{
		anOutProblems.RemoveAll();
		AddEndpointProblem(aNavmesh, "Start", aStart, aClearance, anOutProblems);
		AddEndpointProblem(aNavmesh, "Goal", aGoal, aClearance, anOutProblems);
		AddRosterRadiusProblems(someRosterRadii, aClearance, anOutProblems);
	}

	void GetNavmeshNameForLevel(const FW_String& aLevelName, FW_String& anOutNavmeshName)
	{
		const char* levelPrefix = "level_";
		const int levelPrefixLength = 6;

		anOutNavmeshName = "navmesh_";
		if (aLevelName.Find(levelPrefix) == 0)
			anOutNavmeshName += aLevelName.SubStr(levelPrefixLength, aLevelName.Length());
		else
			anOutNavmeshName += aLevelName;
	}

	NameError ValidateNewLevelName(const FW_String& aLevelName, const FW_GrowingArray<FW_String>& someExistingLevelNames,
		const FW_GrowingArray<FW_String>& someExistingNavmeshNames)
	{
		if (aLevelName.Empty())
			return NameError::Empty;

		for (int i = 0; i < aLevelName.Length(); ++i)
		{
			const char character = aLevelName[i];
			const bool isValid = (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z')
				|| (character >= '0' && character <= '9') || character == '_' || character == '-';
			if (!isValid)
				return NameError::InvalidCharacters;
		}

		if (someExistingLevelNames.Contains(aLevelName))
			return NameError::LevelExists;

		FW_String navmeshName;
		GetNavmeshNameForLevel(aLevelName, navmeshName);
		if (someExistingNavmeshNames.Contains(navmeshName))
			return NameError::NavmeshExists;

		return NameError::None;
	}

	const char* GetNameErrorMessage(NameError aNameError)
	{
		switch (aNameError)
		{
		case NameError::Empty: return "Enter a name.";
		case NameError::InvalidCharacters: return "Use only A-Z, a-z, 0-9, _ and -.";
		case NameError::LevelExists: return "A level with this name already exists.";
		case NameError::NavmeshExists: return "The matching navmesh name already exists.";
		default: return "";
		}
	}
}
