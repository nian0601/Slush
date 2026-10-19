#include "stdafx.h"
#include "ContentBrowserDockable.h"

#include "Core/Engine.h"
#include "Core/Input.h"
#include "Core/Dockables/DependencyTrackerDockable.h"
#include "Core/Dockables/IAppLayout.h"
#include "Graphics/Window.h"

namespace Slush
{
	ContentBrowserDockable::ContentBrowserDockable(ViewMode aDefaultViewMode)
		: myViewMode(aDefaultViewMode)
	{
		Slush::AssetRegistry& assetRegistry = Slush::AssetRegistry::GetInstance();
		const FW_GrowingArray<IAssetStorage*>& storages = assetRegistry.GetAllAssetStorages();
		for (int i = 0; i < storages.Count(); ++i)
			myAssetFilters.Add(false);
	}

	void ContentBrowserDockable::OnBuildUI()
	{
		Slush::AssetRegistry& assetRegistry = Slush::AssetRegistry::GetInstance();
		const FW_GrowingArray<IAssetStorage*>& storages = assetRegistry.GetAllAssetStorages();
		FW_ASSERT(myAssetFilters.Count() == storages.Count(), "New AssetType was registered after opening ContentBrowser, we dont handle that yet");

		BuildFilterSidebar(storages);

		ImGui::SameLine();

		ImGui::BeginGroup();
		if (ImGui::RadioButton("Grid", myViewMode == ViewMode::Grid))
			myViewMode = ViewMode::Grid;
		ImGui::SameLine();
		if (ImGui::RadioButton("List", myViewMode == ViewMode::List))
			myViewMode = ViewMode::List;

		if (myViewMode == ViewMode::Grid)
			BuildGridView(storages);
		else
			BuildListView(storages);
		ImGui::EndGroup();
	}

	void ContentBrowserDockable::BuildFilterSidebar(const FW_GrowingArray<IAssetStorage*>& someStorages)
	{
		const float sidebarWidth = 140.0f;
		if (ImGui::BeginChild("FilterSidebar", ImVec2(sidebarWidth, 0.0f), ImGuiChildFlags_Borders))
		{
			ImGui::TextDisabled("Filters");
			for (int i = 0; i < someStorages.Count(); ++i)
				ImGui::Selectable(someStorages[i]->GetAssetTypeName(), &myAssetFilters[i]);
		}
		ImGui::EndChild();
	}

	void ContentBrowserDockable::BuildListView(const FW_GrowingArray<IAssetStorage*>& someStorages)
	{
		static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable;
		if (ImGui::BeginTable("table_scrolly", 4, flags))
		{
			ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("Repository", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("Filepath", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableHeadersRow();

			for (int i = 0; i < someStorages.Count(); ++i)
			{
				if (!IsStorageVisible(i))
					continue;

				const FW_GrowingArray<Asset*>& assets = someStorages[i]->GetAllAssets();
				for (int j = 0; j < assets.Count(); ++j)
				{
					Slush::Asset* asset = assets[j];

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);

					FW_String label = asset->GetTypeIcon();
					label += " ";
					label += asset->GetAssetName();

					ImGui::Selectable(label.GetBuffer(), false, ImGuiSelectableFlags_SpanAllColumns);
					BuildAssetInteraction(*asset, j);

					ImGui::TableSetColumnIndex(1);
					ImGui::Text("%s", asset->GetTypeName());

					ImGui::TableSetColumnIndex(2);
					ImGui::Text("%s", asset->GetRepositoryName().GetBuffer());

					ImGui::TableSetColumnIndex(3);
					ImGui::Text("%s", asset->GetFilePath().GetBuffer());
				}
			}

			ImGui::EndTable();
		}
	}

	void ContentBrowserDockable::BuildGridView(const FW_GrowingArray<IAssetStorage*>& someStorages)
	{
		if (ImGui::BeginChild("AssetGrid", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders))
		{
			const ImVec2 tileSize(130.0f, 64.0f);
			const float spacing = ImGui::GetStyle().ItemSpacing.x;
			const float availableWidth = ImGui::GetContentRegionAvail().x;
			int columns = static_cast<int>((availableWidth + spacing) / (tileSize.x + spacing));
			if (columns < 1)
				columns = 1;

			ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));

			int tileIndex = 0;
			for (int i = 0; i < someStorages.Count(); ++i)
			{
				if (!IsStorageVisible(i))
					continue;

				const FW_GrowingArray<Asset*>& assets = someStorages[i]->GetAllAssets();
				for (int j = 0; j < assets.Count(); ++j)
				{
					Slush::Asset* asset = assets[j];

					if (tileIndex % columns != 0)
						ImGui::SameLine();
					++tileIndex;

					FW_String label = asset->GetTypeIcon();
					label += "\n";
					label += asset->GetAssetName();

					ImGui::PushID(asset);
					ImGui::Selectable(label.GetBuffer(), false, ImGuiSelectableFlags_None, tileSize);
					const bool isHovered = ImGui::IsItemHovered();
					BuildAssetInteraction(*asset, j);

					// Long names get clipped by the tile, so show the full name on hover
					if (isHovered && !ImGui::GetDragDropPayload())
						ImGui::SetTooltip("%s", asset->GetAssetName().GetBuffer());
					ImGui::PopID();
				}
			}

			ImGui::PopStyleVar();
		}
		ImGui::EndChild();
	}

	void ContentBrowserDockable::BuildAssetInteraction(Asset& anAsset, int anAssetIndex)
	{
		ImGui::BeingDraggedAsset(anAsset, anAssetIndex);

		ImGui::PushID(&anAsset);
		if (ImGui::BeginPopupContextItem("AssetContextMenu"))
		{
			if (ImGui::MenuItem("Show Dependencies"))
				myOwnerLayout->OpenOrCreateDockable<DependencyTrackerDockable>()->SetSelectedAsset(&anAsset);

			ImGui::EndPopup();
		}
		ImGui::PopID();
	}

	bool ContentBrowserDockable::IsStorageVisible(int aStorageIndex) const
	{
		// No filter selected means show everything
		bool isFilterEnabled = false;
		for (const bool& filter : myAssetFilters)
			isFilterEnabled |= filter;

		return !isFilterEnabled || myAssetFilters[aStorageIndex];
	}
}
