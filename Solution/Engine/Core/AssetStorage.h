#pragma once
#include "Log.h"
#include "FW_Hashmap.h"
#include "FW_GrowingArray.h"

namespace Slush
{
	template<typename AssetType>
	class AssetStorage
	{
	public:
		~AssetStorage();

		AssetType& CreateNewAsset(const char* aName);
		void Load(const char* aName, const char* aFilePath, bool aIsAbsolutePath = false);
		const AssetType* GetAsset(const char* aName) const;

		const FW_GrowingArray<AssetType*>& GetAllAssets() const { return myAssets; }
	private:
		FW_Hashmap<FW_String, AssetType*> myAssetMap;
		FW_GrowingArray<AssetType*> myAssets;
	};

	template<typename AssetType>
	AssetStorage<AssetType>::~AssetStorage()
	{
		myAssetMap.Clear();
		myAssets.DeleteAll();
	}

	template<typename AssetType>
	AssetType& AssetStorage<AssetType>::CreateNewAsset(const char* aName)
	{
		if (myAssetMap.KeyExists(aName))
		{
			SLUSH_WARNING("AssetStorage: '%s' already exists, returning existing one instead of creating a new Asset", aName);
			return *myAssetMap[aName];
		}

		AssetType* asset = new AssetType(aName);

		myAssets.Add(asset);
		myAssetMap[aName] = asset;

		SLUSH_INFO("AssetStorage: Created '%s' as new asset", aName);

		return *asset;
	}

	template<typename AssetType>
	void AssetStorage<AssetType>::Load(const char* aName, const char* aFilePath, bool aIsAbsolutePath /*= false*/)
	{
		if (myAssetMap.KeyExists(aName))
		{
			SLUSH_WARNING("AssetStorage: '%s' already exists, loading of '%s' failed.", aName, aFilePath);
			return;
		}

		AssetType* asset = new AssetType(aName);
		asset->Load(aFilePath, aIsAbsolutePath);

		myAssets.Add(asset);
		myAssetMap[aName] = asset;

		SLUSH_INFO("AssetStorage: Stored '%s' as '%s'.", aFilePath, aName);
	}

	template<typename AssetType>
	const AssetType* AssetStorage<AssetType>::GetAsset(const char* aName) const
	{
		if (const AssetType* const* asset = myAssetMap.GetIfExists(aName))
			return *asset;

		SLUSH_WARNING("AssetStorage: '%s' not found.", aName);
		return nullptr;
	}
}
