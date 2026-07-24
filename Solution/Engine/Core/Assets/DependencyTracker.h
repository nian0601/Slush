#pragma once

#include "Asset.h"
#include <FW_String.h>
#include <FW_GrowingArray.h>
#include <FW_Hashmap.h>

namespace Slush
{
	// Queryable graph of which assets reference which. Populated by AssetReference<T>::ResolveDependency()
	// during AssetRegistry::LoadAllAssets()'s ResolveDependencies pass - not persisted, rebuilt every load.
	class DependencyTracker
	{
	public:
		static DependencyTracker& GetInstance();
		static void Destroy();

		void BeginTrackingAsset(Asset* anAsset);
		void EndTrackingAsset();
		const FW_String& GetCurrentAssetName() const;

		void RegisterDependency(unsigned int aTypeID, const FW_String& aName);

		const FW_GrowingArray<FW_String>& GetDependencies(unsigned int aTypeID, const FW_String& aName) const;
		const FW_GrowingArray<FW_String>& GetDependents(unsigned int aTypeID, const FW_String& aName) const;

		// Inverse of MakeKey - splits a "<typeID>:<name>" key back into its parts. Returns false if aKey isn't a well-formed key.
		static bool ParseKey(const FW_String& aKey, unsigned int& outTypeID, FW_String& outName);

	private:
		DependencyTracker() {}
		~DependencyTracker() {}
		static DependencyTracker* ourInstance;

		static FW_String MakeKey(unsigned int aTypeID, const FW_String& aName);

		Asset* myCurrentAsset = nullptr; // single current tracked asset; Begin/End are not nested today
		FW_Hashmap<FW_String, FW_GrowingArray<FW_String>> myDependencies; // asset key -> keys it depends on
		FW_Hashmap<FW_String, FW_GrowingArray<FW_String>> myDependents;   // asset key -> keys that depend on it
	};
}
