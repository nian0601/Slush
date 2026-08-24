#pragma once

#include <Core/Dockables/IAppLayout.h>

class Navmesh;

class TopDownGameLayout : public Slush::IAppLayout
{
public:
	TopDownGameLayout(Navmesh& aNavmesh);

private:
	Navmesh& myNavmesh;
};
