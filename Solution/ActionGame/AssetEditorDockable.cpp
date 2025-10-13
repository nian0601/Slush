#include "stdafx.h"

#include "AssetEditorDockable.h"

#include "Core/Engine.h"
#include "Graphics/Window.h"
#include "Core/Input.h"
#include <Graphics/Animation/Animation.h>

AssetEditorDockable::AssetEditorDockable()
	: Dockable(false)
	, myNewAssetNameStorage("")
{
	Slush::Animation* anim = Slush::AssetRegistry::GetInstance().GetAsset<Slush::Animation>("Dash");
	myAssets.Add({ anim, true });
}

void AssetEditorDockable::OnUpdate()
{
	for (int i = 0; i < myAssets.Count();)
	{
		if (!myAssets[i].myShouldKeep)
		{
			myAssets.RemoveNonCyclicAtIndex(i);
		}
		else
		{
			++i;
		}
	}
}

void AssetEditorDockable::OnBuildUI()
{
	ImGui::BeginChild("dragndropdummy", ImVec2(0, 0), false, ImGuiWindowFlags_MenuBar);

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New"))
				myWantToOpenNewAssetDialogue = true;

			if (ImGui::MenuItem("Save All"))
			{
				for (AssetData& assetData : myAssets)
					assetData.myAsset->Save();
			}

			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();	
	}

	if (!myAssets.IsEmpty())
	{
		if (ImGui::BeginTabBar("AssetTabs", ImGuiTabBarFlags_AutoSelectNewTabs))
		{
			for (AssetData& assetData : myAssets)
			{
				if (ImGui::BeginTabItem(assetData.myAsset->GetAssetName().GetBuffer(), &assetData.myShouldKeep))
				{
					if (ImGui::BeginMenuBar())
					{
						if (ImGui::MenuItem("Save"))
							assetData.myAsset->Save();

						if (ImGui::MenuItem("Save As"))
						{
							myWantToOpenSaveAsDialogue = true;
							mySelectedAsset = assetData.myAsset;
						}

						ImGui::EndMenuBar();
					}

					assetData.myAsset->BuildUI();

					ImGui::EndTabItem();
				}
			}

			ImGui::EndTabBar();
		}
	}

	ImGui::EndChild();

	HandleCreatingNewAsset();
	HandleSavingAsNewAsset();

	if (ImGui::BeginDragDropTarget())
	{
		const FW_GrowingArray<Slush::IAssetStorage*>& assetStorages = Slush::AssetRegistry::GetInstance().GetAllAssetStorages();
		for (Slush::IAssetStorage* storage : assetStorages)
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(storage->GetAssetTypeName(), 0))
			{
				bool alreadyOpenedAsset = false;
				int assetIndex = *(const int*)payload->Data;
				Slush::Asset* asset = storage->GetAllAssets()[assetIndex];
				for (AssetData& data : myAssets)
				{
					if (data.myAsset == asset)
					{
						alreadyOpenedAsset = true;
						break;
					}
				}

				if (!alreadyOpenedAsset)
				{
					AssetData& data = myAssets.Add();
					data.myAsset = asset;
					data.myShouldKeep = true;
				}
			}
		}

		ImGui::EndDragDropTarget();
	}

}

void AssetEditorDockable::HandleCreatingNewAsset()
{
	if (myWantToOpenNewAssetDialogue)
	{
		//ImGui::OpenPopup("New_Prefab_Popup");
		SLUSH_ERROR("'New' is not implemented yet!");
		myWantToOpenNewAssetDialogue = false;
	}
	//
	//if (ImGui::BeginPopupModal("New_Prefab_Popup"))
	//{
	//	if (ImGui::InputText("Name", &myNewAssetNameStorage))
	//		myHasUniqueAssetName = VerifyUniqueNameForNewAsset();
	//
	//	if (ImGui::Button("Create"))
	//	{
	//		if (!myNewAssetNameStorage.Empty() && myHasUniqueAssetName)
	//		{
	//			myAssetStorage.CreateNewAsset(myNewAssetNameStorage.GetBuffer());
	//			myNewAssetNameStorage.Clear();
	//			ImGui::CloseCurrentPopup();
	//		}
	//	}
	//
	//	if (ImGui::IsItemHovered())
	//	{
	//		if (myNewAssetNameStorage.Empty())
	//			ImGui::SetTooltip("Invalid name");
	//		else if (!myHasUniqueAssetName)
	//			ImGui::SetTooltip("Prefab with that name already exists");
	//	}
	//
	//	ImGui::SameLine();
	//
	//	if (ImGui::Button("Cancel")) { ImGui::CloseCurrentPopup(); }
	//
	//	ImGui::EndPopup();
	//}
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
				Slush::IAssetStorage& assetStorage = Slush::AssetRegistry::GetInstance().GetAssetStorage(mySelectedAsset->GetAssetTypeID());
				AssetData& newAsset = myAssets.Add();
				newAsset.myAsset = &assetStorage.CopyAsset(myNewAssetNameStorage.GetBuffer(), *mySelectedAsset);
				newAsset.myShouldKeep = true;

				myNewAssetNameStorage.Clear();
				mySelectedAsset = nullptr;
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
	// This will only work for 'Save As', but not for 'New'.
	// The entire 'New'-flow needs updating now that this dockable handles all AssetTypes; the user needs to select what kind of asset that should be created etc
	Slush::IAssetStorage& assetStorage = Slush::AssetRegistry::GetInstance().GetAssetStorage(mySelectedAsset->GetAssetTypeID());
	const FW_GrowingArray<Slush::Asset*> assets = assetStorage.GetAllAssets();
	for (Slush::Asset* asset : assets)
	{
		if (asset->GetAssetName() == myNewAssetNameStorage.GetBuffer())
			return false;
	}

	return true;
}
