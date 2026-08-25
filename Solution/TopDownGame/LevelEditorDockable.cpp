#include "stdafx.h"

#include "LevelEditorDockable.h"

#include "Level/Level.h"

LevelEditorDockable::LevelEditorDockable(Level& aLevel)
	: Slush::DockableBase<LevelEditorDockable>()
	, myLevel(aLevel)
{
}

void LevelEditorDockable::OnUpdate()
{
}

void LevelEditorDockable::OnBuildUI()
{
	ImGui::Text("Level Editor");
}
