#pragma once

#include "Core/Dockables/Dockable.h"

class FilterColorHelperDockable : public Slush::Dockable
{
public:
	FilterColorHelperDockable();
	const char* GetName() const override { return "Filter Color Helper"; }

protected:
	void OnBuildUI() override;

	float myColor[4] = { 1.f, 1.f, 1.f, 1.f };
};