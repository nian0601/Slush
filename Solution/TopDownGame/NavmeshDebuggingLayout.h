#pragma once

#include <Core/Dockables/IAppLayout.h>

#include "Navmesh.h"

class NavmeshDebuggerDockable;

class NavmeshDebuggingLayout : public Slush::IAppLayout
{
public:
	NavmeshDebuggingLayout();

protected:
	void OnRender() override;

private:
	// Owned outright rather than referencing the real Level's NavmeshData - this layout is a pure
	// diagnostic sandbox for quick pathfinding experiments, kept fully separate from the real,
	// persisted navmesh the Level Editor saves.
	Navmesh myNavmesh;
	NavmeshDebuggerDockable* myNavmeshDebuggerDockable = nullptr;
};
