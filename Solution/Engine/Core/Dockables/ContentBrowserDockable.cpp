#include "stdafx.h"
#include "ContentBrowserDockable.h"

#include "Core/Engine.h"
#include "Core/Input.h"
#include "Graphics/Window.h"

namespace Slush
{
	ContentBrowserDockable::ContentBrowserDockable()
	{
		myShouldShowDockable = false;
	}

	void ContentBrowserDockable::AddAssetStorage(IAssetStorage* aStorage)
	{
		StorageData& data = myAssetStorages.Add();
		data.myAssetStorage = aStorage;
	}

	void ContentBrowserDockable::OnUpdate()
	{
		if (Engine::GetInstance().GetInput().WasKeyReleased(Input::_F1))
		{
			myShouldShowDockable = !myShouldShowDockable;
		}
	}

	void ContentBrowserDockable::OnBuildUI()
	{
		for (StorageData& storageData : myAssetStorages)
		{
			ImGui::SameLine();
			ImGui::Checkbox(storageData.myAssetStorage->GetAssetTypeName(), &storageData.myShouldShowAssets);
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


			for (StorageData& storageData : myAssetStorages)
			{
				if (!storageData.myShouldShowAssets)
					continue;

				const FW_GrowingArray<Asset*> assets = storageData.myAssetStorage->GetAllAssets();
				for (int i = 0; i < assets.Count(); ++i)
				{
					Slush::Asset* asset = assets[i];

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);

					ImGui::Selectable(asset->GetAssetName().GetBuffer(), false, ImGuiSelectableFlags_SpanAllColumns);

					ImGuiDragDropFlags src_flags = 0;
					if (ImGui::BeginDragDropSource(src_flags))
					{
						if (!(src_flags & ImGuiDragDropFlags_SourceNoPreviewTooltip))
							ImGui::Text("Dragging \"%s\"", asset->GetAssetName().GetBuffer());
						ImGui::SetDragDropPayload(asset->GetTypeName(), &i, sizeof(int));
						ImGui::EndDragDropSource();
					}

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