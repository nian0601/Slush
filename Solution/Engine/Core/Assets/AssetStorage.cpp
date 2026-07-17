#include "stdafx.h"
#include "AssetStorage.h"
#include "DependencyTracker.h"

namespace Slush
{
	AssetRegistry* AssetRegistry::ourInstance = nullptr;

	AssetRegistry& AssetRegistry::GetInstance()
	{
		if (!ourInstance)
			ourInstance = new AssetRegistry();

		return *ourInstance;
	}

	void AssetRegistry::Destroy()
	{
		FW_SAFE_DELETE(ourInstance);
	}

	void AssetRegistry::LoadAllAssets()
	{
		for (IAssetStorage* storage : myAssetStorages)
			storage->LoadAllAssets();

		DependencyTracker& dependencyTracker = DependencyTracker::GetInstance();
		for (IAssetStorage* storage : myAssetStorages)
		{
			for (Asset* asset : storage->GetAllAssets())
			{
				dependencyTracker.BeginTrackingAsset(asset);
				asset->ResolveDependencies();
				dependencyTracker.EndTrackingAsset();
			}
		}
	}

	Slush::IAssetStorage& AssetRegistry::GetAssetStorage(unsigned int aAssetTypeID)
	{
		for (IAssetStorage* storage : myAssetStorages)
		{
			if (storage->GetAssetTypeID() == aAssetTypeID)
				return *storage;
		}
			
		FW_ASSERT("Failed to find storage for assettype");
		return *myAssetStorages[0];
	}

	AssetRegistry::AssetRegistry()
	{
	}

	AssetRegistry::~AssetRegistry()
	{
		myAssetStorages.DeleteAll();
	}
}