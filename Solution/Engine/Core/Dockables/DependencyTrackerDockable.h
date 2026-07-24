#pragma once

#include "Dockable.h"

#include <FW_String.h>

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
		void BuildDependentsPane();
		void BuildMiddlePane();
		void BuildDependenciesPane();

		Slush::Asset* ResolveAssetFromKey(const FW_String& aKey) const;
		static FW_String BuildAssetLabel(const Slush::Asset* anAsset);

		Slush::Asset* mySelectedAsset = nullptr;
	};
}
