#include "stdafx.h"

#include "ActionGameGlobals.h"
#include "LevelData.h"
#include "LevelDataDockable.h"

LevelDataDockable::LevelDataDockable()
	: Dockable(true)
	//, myNewPrefabNameStorage("")
{
	myShouldShowDockable = true;
}

void LevelDataDockable::OnUpdate()
{
}

void LevelDataDockable::OnBuildUI()
{
	ActionGameGlobals& globals = ActionGameGlobals::GetInstance();
	LevelData* testLevel = globals.GetLevelDataStorage().GetAsset("testLevel");

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::MenuItem("Save"))
			testLevel->Save();

		ImGui::EndMenuBar();
	}

	testLevel->BuildUI();
}