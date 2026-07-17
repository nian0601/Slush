#pragma once

#include "Asset.h"
#include "AssetParser.h"
#include "AssetStorage.h"
#include "DependencyTracker.h"

namespace Slush
{
	// Pairs a saved asset name with its resolved pointer. Parse() reads/writes the name only;
	// ResolveDependency() does the actual lookup once, during AssetRegistry::LoadAllAssets()'s
	// second pass - not on every use (e.g. not on every entity spawn).
	template <typename AssetType>
	class AssetReference
	{
	public:
		AssetReference() = default;

		void Parse(AssetParser::Handle aHandle, const char* aFieldName)
		{
			aHandle.ParseStringField(aFieldName, myAssetName);
		}

		void ResolveDependency()
		{
			myAsset = nullptr;
			if (!myAssetName.Empty())
			{
				myAsset = AssetRegistry::GetInstance().GetAsset<AssetType>(myAssetName);
				DependencyTracker::GetInstance().RegisterDependency(GetAssetID<AssetType>(), myAssetName);
			}
		}

		// Editor convenience: set directly from a picked/drag-dropped asset, no round-trip needed.
		void Set(const AssetType* anAsset)
		{
			myAsset = const_cast<AssetType*>(anAsset);
			myAssetName = anAsset ? anAsset->GetAssetName() : "";
		}

		bool IsSet() const { return myAsset != nullptr; }
		AssetType* Get() const { return myAsset; }
		AssetType* operator->() const { FW_ASSERT(myAsset, "AssetReference: dereferenced before ResolveDependency() or unset"); return myAsset; }
		operator AssetType*() const { return myAsset; }
		const FW_String& GetName() const { return myAssetName; }

	private:
		FW_String myAssetName;
		AssetType* myAsset = nullptr;
	};
}
