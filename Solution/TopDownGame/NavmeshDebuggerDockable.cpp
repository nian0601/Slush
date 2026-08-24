#include "stdafx.h"

#include "NavmeshDebuggerDockable.h"

#include "Navmesh.h"

NavmeshDebuggerDockable::NavmeshDebuggerDockable(Navmesh& aNavmesh)
	: Slush::DockableBase<NavmeshDebuggerDockable>()
	, myNavmesh(aNavmesh)
{
}

void NavmeshDebuggerDockable::OnBuildUI()
{
	ImGui::Text("Navmesh Debugger");

	if (myIsBoxCutModeActive)
	{
		if (ImGui::Button("Disable Box Cut"))
		{
			myIsBoxCutModeActive = false;
			myNavmesh.SetBoxCutModeActive(false);
		}
	}
	else
	{
		if (ImGui::Button("Enable Box Cut"))
		{
			myIsBoxCutModeActive = true;
			myNavmesh.SetBoxCutModeActive(true);
		}
	}
}
