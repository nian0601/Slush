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
		virtual Asset& CreateNewAsset(const char* aName) = 0;
		virtual Asset& CopyAsset(const char* aNewName, const Asset& anOldAsset) = 0;

		virtual void LoadAllAssets() = 0;

		virtual const char* GetAssetTypeName() const = 0;
		unsigned virtual int GetAssetTypeID() const = 0;

		virtual const Asset* GetAsset(const char* aName) const = 0;
		virtual Asset* GetAsset(const char* aName) = 0;

		virtual const FW_GrowingArray<Asset*>& GetAllAssets() const = 0;
	};

	template<typename AssetType>
	class AssetStorage : public IAssetStorage
	{
	public:
		AssetStorage();
		~AssetStorage();

		Asset& CreateNewAsset(const char* aName) override;
		Asset& CopyAsset(const char* aNewName, const Asset& anOldAsset) override;

		void Load(const char* aName, const char* aFilePath);
		void LoadAllAssets();

		const Asset* GetAsset(const char* aName) const;
		const AssetType* GetAsset(const FW_String& aName) const;
		AssetType* GetAsset(const char* aName);
		AssetType* GetAsset(const FW_String& aName);

		const char* GetAssetTypeName() const override { return AssetType::GetAssetTypeName(); }
		unsigned int GetAssetTypeID() const override { return myAssetTypeID; }
		const FW_GrowingArray<Asset*>& GetAllAssets() const override { return myAssets; }

	private:
		FW_Hashmap<FW_String, AssetType*> myAssetMap;
		FW_GrowingArray<Asset*> myAssets;
		unsigned int myAssetTypeID = -1;
	};

	template<typename AssetType>
	AssetStorage<AssetType>::AssetStorage()
	{
		myAssetTypeID = GetAssetID<AssetType>();
	}

	template<typename AssetType>
	AssetStorage<AssetType>::~AssetStorage()
	{
		myAssetMap.Clear();
		myAssets.DeleteAll();
	}

	template<typename AssetType>
	Asset& AssetStorage<AssetType>::CreateNewAsset(const char* aName)
	{
		if (myAssetMap.KeyExists(aName))
		{
			SLUSH_WARNING("%ss: '%s' already exists, returning existing one instead of creating a new Asset", AssetType::GetAssetTypeName(), aName);
			return *myAssetMap[aName];
		}

		AssetType* asset = new AssetType(aName, GetAssetID<AssetType>());

		myAssets.Add(asset);
		myAssetMap[aName] = asset;

		SLUSH_INFO("%ss: Created '%s' as new asset", AssetType::GetAssetTypeName(), aName);

		return *asset;
	}

	template<typename AssetType>
	Asset& AssetStorage<AssetType>::CopyAsset(const char* aNewName, const Asset& anOldAsset)
	{
		if (myAssetMap.KeyExists(aNewName))
		{
			SLUSH_WARNING("%ss: '%s' already exists, returning existing one instead of creating a new copy", AssetType::GetAssetTypeName(), aNewName);
			return *myAssetMap[aNewName];
		}

		AssetType* asset = new AssetType(aNewName, GetAssetID<AssetType>());
		asset->Load(anOldAsset.GetFilePath().GetBuffer());
		asset->Save();

		myAssets.Add(asset);
		myAssetMap[aNewName] = asset;

		SLUSH_INFO("%ss: Created '%s' as copy of %s", AssetType::GetAssetTypeName(), aNewName, anOldAsset.GetAssetName().GetBuffer());

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

		AssetType* asset = new AssetType(aName, GetAssetID<AssetType>());
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
	const Asset* AssetStorage<AssetType>::GetAsset(const char* aName) const
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

	//////////////////////////////////////////////////////////////////////////

	class AssetRegistry
	{
	public:
		static AssetRegistry& GetInstance();
		static void Destroy();

		template <typename AssetType>
		void RegisterAssetType();

		void LoadAllAssets();

		template <typename AssetType>
		const FW_GrowingArray<Asset*>& GetAllAssets() const;
		
		template <typename AssetType>
		AssetType* GetAsset(const char* aAssetName);

		template <typename AssetType>
		const AssetType* GetAsset(const char* aAssetName) const;

		template <typename AssetType>
		const AssetType* GetAsset(int aAssetIndex) const;

		const FW_GrowingArray<IAssetStorage*>& GetAllAssetStorages() const { return myAssetStorages; }
		
		template <typename AssetType>
		IAssetStorage& GetAssetStorage();
		IAssetStorage& GetAssetStorage(unsigned int aAssetTypeID);

	private:
		AssetRegistry();
		~AssetRegistry();
		static AssetRegistry* ourInstance;

		FW_GrowingArray<IAssetStorage*> myAssetStorages;
	};

	template <typename AssetType>
	inline void AssetRegistry::RegisterAssetType()
	{
		int assetID = GetAssetID<AssetType>();
		FW_ASSERT(assetID == myAssetStorages.Count(), "AssetType got registered more than once");
		myAssetStorages.Add(new AssetStorage<AssetType>());
	}

	template <typename AssetType>
	inline const FW_GrowingArray<Asset*>& AssetRegistry::GetAllAssets() const
	{
		int assetID = GetAssetID<AssetType>();
		FW_ASSERT(assetID < myAssetStorages.Count(), "AssetType is not registered");
		return myAssetStorages[assetID]->GetAllAssets();
	}

	template <typename AssetType>
	inline AssetType* AssetRegistry::GetAsset(const char* aAssetName)
	{
		int assetID = GetAssetID<AssetType>();
		FW_ASSERT(assetID < myAssetStorages.Count(), "AssetType is not registered");
		return static_cast<AssetType*>(myAssetStorages[assetID]->GetAsset(aAssetName));
	}

	template <typename AssetType>
	inline const AssetType* AssetRegistry::GetAsset(const char* aAssetName) const
	{
		int assetID = GetAssetID<AssetType>();
		FW_ASSERT(assetID < myAssetStorages.Count(), "AssetType is not registered");
		return static_cast<const AssetType*>(myAssetStorages[assetID]->GetAsset(aAssetName));
	}

	template <typename AssetType>
	inline const AssetType* AssetRegistry::GetAsset(int aAssetIndex) const
	{
		int assetID = GetAssetID<AssetType>();
		FW_ASSERT(assetID < myAssetStorages.Count(), "AssetType is not registered");

		const FW_GrowingArray<Asset*> assets = myAssetStorages[assetID]->GetAllAssets();
		FW_ASSERT(aAssetIndex >= 0 && aAssetIndex < assets.Count(), "Invalid AssetID");

		return static_cast<const AssetType*>(assets[aAssetIndex]);
	}

	template <typename AssetType>
	inline IAssetStorage& AssetRegistry::GetAssetStorage()
	{
		int assetID = GetAssetID<AssetType>();
		FW_ASSERT(assetID < myAssetStorages.Count(), "AssetType is not registered");

		return *myAssetStorages[assetID];
	}
}
