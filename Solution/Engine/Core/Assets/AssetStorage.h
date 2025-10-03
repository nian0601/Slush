#pragma once

#include "Core/Assets/Asset.h"
#include "Core/Log.h"

#include "FW_FileSystem.h"
#include "FW_GrowingArray.h"
#include "FW_Hashmap.h"

namespace Slush
{
	class IAssetStorage
	{
	public:
		virtual const char* GetAssetTypeName() const = 0;
		virtual const FW_GrowingArray<Asset*>& GetAllAssets() const = 0;
	};

	template<typename AssetType>
	class AssetStorage : public IAssetStorage
	{
	public:
		~AssetStorage();

		AssetType& CreateNewAsset(const char* aName);

		void Load(const char* aName, const char* aFilePath);
		void LoadAllAssets();

		const AssetType* GetAsset(const char* aName) const;
		const AssetType* GetAsset(const FW_String& aName) const;
		AssetType* GetAsset(const char* aName);
		AssetType* GetAsset(const FW_String& aName);

		const char* GetAssetTypeName() const override { return AssetType::GetAssetTypeName(); }
		const FW_GrowingArray<Asset*>& GetAllAssets() const override { return myAssets; }

	private:
		FW_Hashmap<FW_String, AssetType*> myAssetMap;
		FW_GrowingArray<Asset*> myAssets;
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
			SLUSH_WARNING("%ss: '%s' already exists, returning existing one instead of creating a new Asset", AssetType::GetAssetTypeName(), aName);
			return *myAssetMap[aName];
		}

		AssetType* asset = new AssetType(aName);

		myAssets.Add(asset);
		myAssetMap[aName] = asset;

		SLUSH_INFO("%ss: Created '%s' as new asset", AssetType::GetAssetTypeName(), aName);

		return *asset;
	}

	template<typename AssetType>
	void AssetStorage<AssetType>::Load(const char* aName, const char* aFilePath)
	{
		if (myAssetMap.KeyExists(aName))
		{
			SLUSH_WARNING("%ss: '%s' already exists, loading of '%s' failed.", AssetType::GetAssetTypeName(), aName, aFilePath);
			return;
		}

		AssetType* asset = new AssetType(aName);
		asset->Load(aFilePath);

		myAssets.Add(asset);
		myAssetMap[aName] = asset;

		SLUSH_INFO("%ss: '%s' loaded as '%s'.", AssetType::GetAssetTypeName(), aFilePath, aName);
	}

	template<typename AssetType>
	void AssetStorage<AssetType>::LoadAllAssets()
	{
		SLUSH_INFO("Loading all %ss:", AssetType::GetAssetTypeName())
		FW_GrowingArray<FW_FileSystem::FileInfo> assetFiles;
		FW_FileSystem::GetAllFilesFromRelativeDirectory(AssetType::GetAssetTypeFolder(), assetFiles);

		for (const FW_FileSystem::FileInfo& info : assetFiles)
			Load(info.myFileNameNoExtention.GetBuffer(), info.myRelativeFilePath.GetBuffer());
	}

	template<typename AssetType>
	const AssetType* AssetStorage<AssetType>::GetAsset(const char* aName) const
	{
		if (const AssetType* const* asset = myAssetMap.GetIfExists(aName))
			return *asset;

		SLUSH_WARNING("%ss: '%s' not found.", AssetType::GetAssetTypeName(), aName);
		return nullptr;
	}

	template<typename AssetType>
	const AssetType* AssetStorage<AssetType>::GetAsset(const FW_String& aName) const
	{
		return GetAsset(aName.GetBuffer());
	}

	template<typename AssetType>
	AssetType* AssetStorage<AssetType>::GetAsset(const char* aName)
	{
		if (AssetType* const* asset = myAssetMap.GetIfExists(aName))
			return *asset;

		SLUSH_WARNING("%ss: '%s' not found.", AssetType::GetAssetTypeName(), aName);
		return nullptr;
	}

	template<typename AssetType>
	AssetType* AssetStorage<AssetType>::GetAsset(const FW_String& aName)
	{
		return GetAsset(aName.GetBuffer());
	}
}
