#include "stdafx.h"
#include "DependencyTracker.h"

namespace Slush
{
	DependencyTracker* DependencyTracker::ourInstance = nullptr;

	DependencyTracker& DependencyTracker::GetInstance()
	{
		if (!ourInstance)
			ourInstance = new DependencyTracker();

		return *ourInstance;
	}

	void DependencyTracker::Destroy()
	{
		FW_SAFE_DELETE(ourInstance);
	}

	void DependencyTracker::BeginTrackingAsset(Asset* anAsset)
	{
		FW_ASSERT(myCurrentAsset == nullptr, "[DependencyTracker] BeginTrackingAsset called while already tracking another asset - nesting isn't supported");
		myCurrentAsset = anAsset;
	}

	void DependencyTracker::EndTrackingAsset()
	{
		myCurrentAsset = nullptr;
	}

	const FW_String& DependencyTracker::GetCurrentAssetName() const
	{
		FW_ASSERT(myCurrentAsset != nullptr, "[DependencyTracker] GetCurrentAssetName called outside of a BeginTrackingAsset/EndTrackingAsset pair");
		return myCurrentAsset->GetAssetName();
	}

	void DependencyTracker::RegisterDependency(unsigned int aTypeID, const FW_String& aName)
	{
		FW_ASSERT(myCurrentAsset != nullptr, "[DependencyTracker] RegisterDependency called outside of a BeginTrackingAsset/EndTrackingAsset pair");

		FW_String dependentKey = MakeKey(myCurrentAsset->GetAssetTypeID(), myCurrentAsset->GetAssetName());
		FW_String dependencyKey = MakeKey(aTypeID, aName);

		myDependencies[dependentKey].Add(dependencyKey);
		myDependents[dependencyKey].Add(dependentKey);
	}

	const FW_GrowingArray<FW_String>& DependencyTracker::GetDependencies(unsigned int aTypeID, const FW_String& aName) const
	{
		static FW_GrowingArray<FW_String> ourEmptyArray;
		FW_String key = MakeKey(aTypeID, aName);
		if (const FW_GrowingArray<FW_String>* dependencies = myDependencies.GetIfExists(key))
			return *dependencies;

		return ourEmptyArray;
	}

	const FW_GrowingArray<FW_String>& DependencyTracker::GetDependents(unsigned int aTypeID, const FW_String& aName) const
	{
		static FW_GrowingArray<FW_String> ourEmptyArray;
		FW_String key = MakeKey(aTypeID, aName);
		if (const FW_GrowingArray<FW_String>* dependents = myDependents.GetIfExists(key))
			return *dependents;

		return ourEmptyArray;
	}

	FW_String DependencyTracker::MakeKey(unsigned int aTypeID, const FW_String& aName)
	{
		FW_String key;
		key += static_cast<int>(aTypeID);
		key += ":";
		key += aName;
		return key;
	}

	bool DependencyTracker::ParseKey(const FW_String& aKey, unsigned int& outTypeID, FW_String& outName)
	{
		const int separatorIndex = aKey.Find(":");
		if (separatorIndex == FW_String::NotFound)
			return false;

		const FW_String typeIDStr = aKey.SubStr(0, separatorIndex - 1);
		outTypeID = static_cast<unsigned int>(atoi(typeIDStr.GetBuffer()));
		outName = aKey.SubStr(separatorIndex + 1, aKey.Length() - 1);
		return true;
	}
}
