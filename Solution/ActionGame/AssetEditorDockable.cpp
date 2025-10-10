#include "stdafx.h"

#include "AssetEditorDockable.h"

#include "Core/Engine.h"
#include "Graphics/Window.h"
#include "Core/Input.h"

AssetEditorDockable::AssetEditorDockable(Slush::IAssetStorage& aAssetStorage, const char* aDockableName)
	: Dockable(true)
	, myAssetStorage(aAssetStorage)
	, myDockableName(aDockableName)
	, myNewAssetNameStorage("")
{
}

void AssetEditorDockable::OnUpdate()
{
}

void AssetEditorDockable::OnBuildUI()
{
	const FW_GrowingArray<Slush::Asset*> assets = myAssetStorage.GetAllAssets();
	if (ImGui::BeginMenuBar())
	{
		if (mySelectedAsset)
			ImGui::Text("[%s] ", mySelectedAsset->GetAssetName().GetBuffer());

		if (ImGui::BeginMenu("File.."))
		{
			if (ImGui::MenuItem("New"))
				myWantToOpenNewAssetDialogue = true;
			
			if (mySelectedAsset)
			{
				if (ImGui::MenuItem("Save As"))
					myWantToOpenSaveAsDialogue = true;
			}

			if (ImGui::Selectable("Save All"))
			{
				for (Slush::Asset* asset : assets)
					asset->Save();
			}
	
			if (ImGui::BeginMenu("Open.."))
			{
				for (Slush::Asset* asset : assets)
				{
					if (ImGui::Selectable(asset->GetAssetName().GetBuffer()))
						mySelectedAsset = asset;
				}
	
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
	
		HandleCreatingNewAsset();
		HandleSavingAsNewAsset();

		if (mySelectedAsset && ImGui::MenuItem("Save"))
			mySelectedAsset->Save();
	
		ImGui::EndMenuBar();
	}
	
	ImGui::BeginChild("dragndropdummy");
	
	if (mySelectedAsset)
		mySelectedAsset->BuildUI();
	
	ImGui::EndChild();
	
	if (ImGui::BeginDragDropTarget())
	{
		ImGuiDragDropFlags target_flags = 0;
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(myAssetStorage.GetAssetTypeName(), target_flags))
		{
			int assetIndex = *(const int*)payload->Data;
			mySelectedAsset = myAssetStorage.GetAllAssets()[assetIndex];
		}
		ImGui::EndDragDropTarget();
	}
}

void AssetEditorDockable::HandleCreatingNewAsset()
{
	if (myWantToOpenNewAssetDialogue)
	{
		ImGui::OpenPopup("New_Prefab_Popup");
		myWantToOpenNewAssetDialogue = false;
	}

	if (ImGui::BeginPopupModal("New_Prefab_Popup"))
	{
		if (ImGui::InputText("Name", &myNewAssetNameStorage))
			myHasUniqueAssetName = VerifyUniqueNameForNewAsset();

		if (ImGui::Button("Create"))
		{
			if (!myNewAssetNameStorage.Empty() && myHasUniqueAssetName)
			{
				myAssetStorage.CreateNewAsset(myNewAssetNameStorage.GetBuffer());
				myNewAssetNameStorage.Clear();
				ImGui::CloseCurrentPopup();
			}
		}

		if (ImGui::IsItemHovered())
		{
			if (myNewAssetNameStorage.Empty())
				ImGui::SetTooltip("Invalid name");
			else if (!myHasUniqueAssetName)
				ImGui::SetTooltip("Prefab with that name already exists");
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel")) { ImGui::CloseCurrentPopup(); }

		ImGui::EndPopup();
	}
}

void AssetEditorDockable::HandleSavingAsNewAsset()
{
	if (myWantToOpenSaveAsDialogue)
	{
		ImGui::OpenPopup("Save_As_Popup");
		myWantToOpenSaveAsDialogue = false;
	}

	if (ImGui::BeginPopupModal("Save_As_Popup"))
	{
		if (ImGui::InputText("Name", &myNewAssetNameStorage))
			myHasUniqueAssetName = VerifyUniqueNameForNewAsset();

		if (ImGui::Button("Save"))
		{
			if (!myNewAssetNameStorage.Empty() && myHasUniqueAssetName)
			{
				mySelectedAsset = &myAssetStorage.CopyAsset(myNewAssetNameStorage.GetBuffer(), *mySelectedAsset);
				myNewAssetNameStorage.Clear();
				ImGui::CloseCurrentPopup();
			}
		}

		if (ImGui::IsItemHovered())
		{
			if (myNewAssetNameStorage.Empty())
				ImGui::SetTooltip("Invalid name");
			else if (!myHasUniqueAssetName)
				ImGui::SetTooltip("Prefab with that name already exists");
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel")) { ImGui::CloseCurrentPopup(); }

		ImGui::EndPopup();
	}
}

bool AssetEditorDockable::VerifyUniqueNameForNewAsset() const
{
	const FW_GrowingArray<Slush::Asset*> assets = myAssetStorage.GetAllAssets();
	for (Slush::Asset* asset : assets)
	{
		EntityPrefab* prefab = static_cast<EntityPrefab*>(asset);
		if (prefab->myName == myNewAssetNameStorage.GetBuffer())
			return false;
	}

	return true;
}
