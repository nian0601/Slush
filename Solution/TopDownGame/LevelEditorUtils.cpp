#include "stdafx.h"

#include "LevelEditorUtils.h"

namespace LevelEditorUtils
{
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
