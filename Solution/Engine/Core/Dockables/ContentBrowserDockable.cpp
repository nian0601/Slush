#include "stdafx.h"
#include "ContentBrowserDockable.h"

#include "Core/Engine.h"
#include "Core/Input.h"
#include "Core/Dockables/DependencyTrackerDockable.h"
#include "Core/Dockables/IAppLayout.h"
#include "Graphics/Window.h"

namespace Slush
{
	ContentBrowserDockable::ContentBrowserDockable()
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

		if (ImGui::CollapsingHeader("Filters"))
		{
			for (int i = 0; i < storages.Count(); ++i)
				ImGui::Checkbox(storages[i]->GetAssetTypeName(), &myAssetFilters[i]);
		}

		ImGui::Separator();

		static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable;
		if (ImGui::BeginTable("table_scrolly", 3, flags))
		{
			ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("Filepath", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableHeadersRow();

			bool isFilterEnabled = false;
			for (bool& filter : myAssetFilters)
				isFilterEnabled |= filter;

			for (int i = 0; i < storages.Count(); ++i)
			{
				IAssetStorage* storage = storages[i];

				if (isFilterEnabled && !myAssetFilters[i])
					continue;

				const FW_GrowingArray<Asset*> assets = storage->GetAllAssets();
				for (int j = 0; j < assets.Count(); ++j)
				{
					Slush::Asset* asset = assets[j];

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);

					FW_String label = asset->GetTypeIcon();
					label += " ";
					label += asset->GetAssetName();

					ImGui::Selectable(label.GetBuffer(), false, ImGuiSelectableFlags_SpanAllColumns);
					ImGui::BeingDraggedAsset(*asset, j);

					ImGui::PushID(asset);
					if (ImGui::BeginPopupContextItem("AssetContextMenu"))
					{
						if (ImGui::MenuItem("Show Dependencies"))
							myOwnerLayout->OpenOrCreateDockable<DependencyTrackerDockable>()->SetSelectedAsset(asset);

						ImGui::EndPopup();
					}
					ImGui::PopID();

					ImGui::TableSetColumnIndex(1);
					ImGui::Text("%s", asset->GetTypeName());

					ImGui::TableSetColumnIndex(2);
					ImGui::Text("%s", asset->GetFilePath().GetBuffer());
				}
			}

			ImGui::EndTable();
		}
	}
}