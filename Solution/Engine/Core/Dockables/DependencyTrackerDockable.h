#pragma once

#include "Dockable.h"

namespace Slush
{
	class Asset;

	class DependencyTrackerDockable : public Slush::Dockable
	{
	public:
		DependencyTrackerDockable();
		const char* GetName() const override { return "Dependency Tracker"; }

	protected:
		void OnBuildUI() override;

	private:
		void BuildMiddlePane();

		Slush::Asset* mySelectedAsset = nullptr;
	};
}
