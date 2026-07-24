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
		static ImGuiTextFilter assetFilter;

		// Fixed-height, non-scrolling row for the search field so it stays put while the list below scrolls.
		ImGui::BeginChild("DependencyTracker_AssetSearch", ImVec2(0, ImGui::GetFrameHeightWithSpacing()), ImGuiChildFlags_None);
		const float clearButtonWidth = ImGui::GetFrameHeight();
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - clearButtonWidth - ImGui::GetStyle().ItemSpacing.x);
		if (ImGui::InputText("##AssetSearch", assetFilter.InputBuf, IM_ARRAYSIZE(assetFilter.InputBuf)))
			assetFilter.Build();
		ImGui::SameLine();
		if (ImGui::Button("X", ImVec2(clearButtonWidth, 0)))
			assetFilter.Clear();
		ImGui::EndChild();

		ImGui::BeginChild("DependencyTracker_AssetListScroll", ImVec2(0, 0), ImGuiChildFlags_None);
		Slush::AssetRegistry& assetRegistry = Slush::AssetRegistry::GetInstance();
		const FW_GrowingArray<IAssetStorage*>& storages = assetRegistry.GetAllAssetStorages();
		for (IAssetStorage* storage : storages)
		{
			const FW_GrowingArray<Asset*>& assets = storage->GetAllAssets();
			for (Asset* asset : assets)
			{
				if (!assetFilter.PassFilter(asset->GetAssetName().GetBuffer()))
					continue;

				if (ImGui::Selectable(BuildAssetLabel(asset).GetBuffer(), asset == mySelectedAsset))
					mySelectedAsset = asset;
			}
		}
		ImGui::EndChild();
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
