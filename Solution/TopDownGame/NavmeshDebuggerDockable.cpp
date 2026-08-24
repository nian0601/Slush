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
}
