#pragma once

#include "Dockable.h"

#include <FW_String.h>

namespace Slush
{
	class Asset;

	class DependencyTrackerDockable : public Slush::DockableBase<DependencyTrackerDockable>
	{
	public:
		DependencyTrackerDockable();
		const char* GetName() const override { return "Dependency Tracker"; }

		void SetSelectedAsset(Slush::Asset* anAsset);

	protected:
		void OnBuildUI() override;

	private:
		void BuildDependentsPane(Slush::Asset*& outPendingSelection);
		void BuildMiddlePane(Slush::Asset*& outPendingSelection);
		void BuildDependenciesPane(Slush::Asset*& outPendingSelection);

		Slush::Asset* ResolveAssetFromKey(const FW_String& aKey) const;
		static FW_String BuildAssetLabel(const Slush::Asset* anAsset);

		Slush::Asset* mySelectedAsset = nullptr;
		bool myWantsFocus = false;
	};
}
