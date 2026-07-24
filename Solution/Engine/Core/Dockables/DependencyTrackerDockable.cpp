#include "stdafx.h"
#include "DependencyTrackerDockable.h"

#include "Core/Assets/AssetStorage.h"
#include "Core/Assets/DependencyTracker.h"

namespace Slush
{
	DependencyTrackerDockable::DependencyTrackerDockable()
	{
	}

	void DependencyTrackerDockable::OnBuildUI()
	{
		BuildMiddlePane();
	}

	void DependencyTrackerDockable::BuildMiddlePane()
	{
		Slush::AssetRegistry& assetRegistry = Slush::AssetRegistry::GetInstance();
		const FW_GrowingArray<IAssetStorage*>& storages = assetRegistry.GetAllAssetStorages();
		for (IAssetStorage* storage : storages)
		{
			const FW_GrowingArray<Asset*>& assets = storage->GetAllAssets();
			for (Asset* asset : assets)
			{
				FW_String label;
				label += asset->GetAssetName();
				label += " (";
				label += asset->GetTypeName();
				label += ")";

				if (ImGui::Selectable(label.GetBuffer(), asset == mySelectedAsset))
					mySelectedAsset = asset;
			}
		}
	}
}
