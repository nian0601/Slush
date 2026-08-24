#pragma once

#include <Core/Dockables/IAppLayout.h>

class Navmesh;
class NavmeshDebuggerDockable;

class TopDownGameLayout : public Slush::IAppLayout
{
public:
	TopDownGameLayout(Navmesh& aNavmesh);

protected:
	void OnRender() override;

private:
	Navmesh& myNavmesh;
	NavmeshDebuggerDockable* myNavmeshDebuggerDockable = nullptr;
};
