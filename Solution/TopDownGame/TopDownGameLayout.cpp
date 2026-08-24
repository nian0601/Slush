#include "stdafx.h"

#include "TopDownGameLayout.h"

#include "NavmeshDebuggerDockable.h"

TopDownGameLayout::TopDownGameLayout(Navmesh& aNavmesh)
	: Slush::IAppLayout("TopDownGame")
	, myNavmesh(aNavmesh)
{
	AddDockable(new NavmeshDebuggerDockable(myNavmesh));
}
