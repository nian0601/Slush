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
		const ImVec2 avail = ImGui::GetContentRegionAvail();
		const float leftWidth = avail.x * 0.25f;
		const float middleWidth = avail.x * 0.35f;

		ImGui::BeginChild("DependencyTracker_Dependents", ImVec2(leftWidth, 0), ImGuiChildFlags_Borders);
		BuildDependentsPane();
		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild("DependencyTracker_AssetList", ImVec2(middleWidth, 0), ImGuiChildFlags_Borders);
		BuildMiddlePane();
		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild("DependencyTracker_Dependencies", ImVec2(0, 0), ImGuiChildFlags_Borders);
		BuildDependenciesPane();
		ImGui::EndChild();
	}

	void DependencyTrackerDockable::BuildDependentsPane()
	{
		ImGui::TextDisabled("Dependents");
		ImGui::Separator();

		if (!mySelectedAsset)
		{
			ImGui::TextDisabled("<no asset selected>");
			return;
		}

		const FW_GrowingArray<FW_String>& dependents = DependencyTracker::GetInstance().GetDependents(mySelectedAsset->GetAssetTypeID(), mySelectedAsset->GetAssetName());
		for (const FW_String& key : dependents)
		{
			Slush::Asset* asset = ResolveAssetFromKey(key);
			if (!asset)
				continue;

			ImGui::Selectable(BuildAssetLabel(asset).GetBuffer(), false);
		}
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
				if (ImGui::Selectable(BuildAssetLabel(asset).GetBuffer(), asset == mySelectedAsset))
					mySelectedAsset = asset;
			}
		}
	}

	void DependencyTrackerDockable::BuildDependenciesPane()
	{
		ImGui::TextDisabled("Dependencies");
		ImGui::Separator();

		if (!mySelectedAsset)
		{
			ImGui::TextDisabled("<no asset selected>");
			return;
		}

		const FW_GrowingArray<FW_String>& dependencies = DependencyTracker::GetInstance().GetDependencies(mySelectedAsset->GetAssetTypeID(), mySelectedAsset->GetAssetName());
		for (const FW_String& key : dependencies)
		{
			Slush::Asset* asset = ResolveAssetFromKey(key);
			if (!asset)
				continue;

			ImGui::Selectable(BuildAssetLabel(asset).GetBuffer(), false);
		}
	}

	Slush::Asset* DependencyTrackerDockable::ResolveAssetFromKey(const FW_String& aKey) const
	{
		unsigned int typeID = 0;
		FW_String name;
		if (!DependencyTracker::ParseKey(aKey, typeID, name))
			return nullptr;

		Slush::IAssetStorage& storage = Slush::AssetRegistry::GetInstance().GetAssetStorage(typeID);
		return storage.GetAsset(name.GetBuffer());
	}

	FW_String DependencyTrackerDockable::BuildAssetLabel(const Slush::Asset* anAsset)
	{
		FW_String label;
		label += anAsset->GetAssetName();
		label += " (";
		label += anAsset->GetTypeName();
		label += ")";
		return label;
	}
}
