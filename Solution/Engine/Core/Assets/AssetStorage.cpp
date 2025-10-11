#include "stdafx.h"
#include "AssetStorage.h"

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
	}

	AssetRegistry::AssetRegistry()
	{
	}

	AssetRegistry::~AssetRegistry()
	{
		myAssetStorages.DeleteAll();
	}
}