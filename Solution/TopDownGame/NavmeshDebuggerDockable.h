#pragma once

#include <Core/Dockables/Dockable.h>

class Navmesh;

class NavmeshDebuggerDockable : public Slush::DockableBase<NavmeshDebuggerDockable>
{
public:
	NavmeshDebuggerDockable(Navmesh& aNavmesh);

	const char* GetName() const override { return "Navmesh Debugger"; }

protected:
	void OnBuildUI() override;

private:
	Navmesh& myNavmesh;
};
