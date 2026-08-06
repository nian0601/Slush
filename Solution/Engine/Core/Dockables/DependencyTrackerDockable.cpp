#include "stdafx.h"
#include "DependencyTrackerDockable.h"

#include "Core/Assets/AssetStorage.h"
#include "Core/Assets/DependencyTracker.h"

namespace Slush
{
	DependencyTrackerDockable::DependencyTrackerDockable()
		: Slush::DockableBase<DependencyTrackerDockable>(false, true, Vector2f(1200.0f, 700.0f))
	{
	}

	void DependencyTrackerDockable::SetSelectedAsset(Slush::Asset* anAsset)
	{
		mySelectedAsset = anAsset;
		myWantsFocus = true;
	}

	void DependencyTrackerDockable::OnBuildUI()
	{
		if (myWantsFocus)
		{
			ImGui::SetWindowFocus(myUniqueIDName.GetBuffer());
			myWantsFocus = false;
		}

		Slush::Asset* pendingSelection = nullptr;

		const ImVec2 avail = ImGui::GetContentRegionAvail();
		const float middleWidth = avail.x * 0.35f;
		const float leftWidth = (avail.x - middleWidth) * 0.5f;

		// NoSavedSettings: this dockable is only ever opened on-demand, never present in the default
		// layout, so persisting a manually-resized pane split across sessions isn't worth the ini clutter
		// it leaves behind once the tracker's closed.
		ImGui::BeginChild("DependencyTracker_Dependents", ImVec2(leftWidth, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX, ImGuiWindowFlags_NoSavedSettings);
		BuildDependentsPane(pendingSelection);
		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild("DependencyTracker_AssetList", ImVec2(middleWidth, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX, ImGuiWindowFlags_NoSavedSettings);
		BuildMiddlePane(pendingSelection);
		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild("DependencyTracker_Dependencies", ImVec2(0, 0), ImGuiChildFlags_Borders, ImGuiWindowFlags_NoSavedSettings);
		BuildDependenciesPane(pendingSelection);
		ImGui::EndChild();

		if (pendingSelection)
			mySelectedAsset = pendingSelection;
	}

	void DependencyTrackerDockable::BuildDependentsPane(Slush::Asset*& outPendingSelection)
	{
		ImGui::TextDisabled("Dependents");
		ImGui::Separator();

		if (!mySelectedAsset)
		{
			ImGui::TextDisabled("<no asset selected>");
			return;
		}

		const FW_GrowingArray<FW_String>& dependents = DependencyTracker::GetInstance().GetDependents(mySelectedAsset->GetAssetTypeID(), mySelectedAsset->GetAssetName());
		bool anyRendered = false;
		for (const FW_String& key : dependents)
		{
			Slush::Asset* asset = ResolveAssetFromKey(key);
			if (!asset)
				continue;

			anyRendered = true;
			if (ImGui::Selectable(BuildAssetLabel(asset).GetBuffer(), asset == mySelectedAsset))
				outPendingSelection = asset;
		}

		if (!anyRendered)
			ImGui::TextDisabled("<no dependents>");
	}

	void DependencyTrackerDockable::BuildMiddlePane(Slush::Asset*& outPendingSelection)
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
					outPendingSelection = asset;
			}
		}
		ImGui::EndChild();
	}

	void DependencyTrackerDockable::BuildDependenciesPane(Slush::Asset*& outPendingSelection)
	{
		ImGui::TextDisabled("Dependencies");
		ImGui::Separator();

		if (!mySelectedAsset)
		{
			ImGui::TextDisabled("<no asset selected>");
			return;
		}

		const FW_GrowingArray<FW_String>& dependencies = DependencyTracker::GetInstance().GetDependencies(mySelectedAsset->GetAssetTypeID(), mySelectedAsset->GetAssetName());
		bool anyRendered = false;
		for (const FW_String& key : dependencies)
		{
			Slush::Asset* asset = ResolveAssetFromKey(key);
			if (!asset)
				continue;

			anyRendered = true;
			if (ImGui::Selectable(BuildAssetLabel(asset).GetBuffer(), asset == mySelectedAsset))
				outPendingSelection = asset;
		}

		if (!anyRendered)
			ImGui::TextDisabled("<no dependencies>");
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
		label += anAsset->GetTypeIcon();
		label += " ";
		label += anAsset->GetAssetName();
		label += " (";
		label += anAsset->GetTypeName();
		label += ")";
		return label;
	}
}
