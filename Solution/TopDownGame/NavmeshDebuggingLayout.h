#pragma once

#include <Core/Dockables/IAppLayout.h>

class Navmesh;
class NavmeshDebuggerDockable;

class NavmeshDebuggingLayout : public Slush::IAppLayout
{
public:
	NavmeshDebuggingLayout(Navmesh& aNavmesh);

protected:
	void OnRender() override;

private:
	Navmesh& myNavmesh;
	NavmeshDebuggerDockable* myNavmeshDebuggerDockable = nullptr;
};
