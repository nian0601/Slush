#include "stdafx.h"

#include "EntityPrefabDockable.h"

#include "Core/Engine.h"
#include "Graphics/Window.h"
#include "Core/Input.h"

EntityPrefabDockable::EntityPrefabDockable(Slush::AssetStorage<EntityPrefab>& aPrefabStorage)
	: Dockable(true)
	, myPrefabStorage(aPrefabStorage)
	, myNewPrefabNameStorage("")
{
}

void EntityPrefabDockable::OnUpdate()
{
}

void EntityPrefabDockable::OnBuildUI()
{
	const FW_GrowingArray<Slush::Asset*> assets = myPrefabStorage.GetAllAssets();
	if (ImGui::BeginMenuBar())
	{
		if (mySelectedPrefab)
			ImGui::Text("[%s] ", mySelectedPrefab->GetAssetName().GetBuffer());

		if (ImGui::BeginMenu("File.."))
		{
			if (ImGui::MenuItem("New"))
				myWantToOpenNewAssetDialogue = true;
			
			if (mySelectedPrefab)
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
					EntityPrefab* prefab = static_cast<EntityPrefab*>(asset);
					if (ImGui::Selectable(prefab->myName.GetBuffer()))
						mySelectedPrefab = prefab;
				}
	
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
	
		HandleCreatingNewAsset();
		HandleSavingAsNewAsset();

		if (mySelectedPrefab && ImGui::MenuItem("Save"))
			mySelectedPrefab->Save();
	
		ImGui::EndMenuBar();
	}
	
	ImGui::BeginChild("dragndropdummy");
	
	if (mySelectedPrefab)
		mySelectedPrefab->BuildUI();
	
	ImGui::EndChild();
	
	if (ImGui::BeginDragDropTarget())
	{
		ImGuiDragDropFlags target_flags = 0;
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(EntityPrefab::GetAssetTypeName(), target_flags))
		{
			int assetIndex = *(const int*)payload->Data;
			mySelectedPrefab = static_cast<EntityPrefab*>(myPrefabStorage.GetAllAssets()[assetIndex]);
		}
		ImGui::EndDragDropTarget();
	}
}

void EntityPrefabDockable::HandleCreatingNewAsset()
{
	if (myWantToOpenNewAssetDialogue)
	{
		ImGui::OpenPopup("New_Prefab_Popup");
		myWantToOpenNewAssetDialogue = false;
	}

	if (ImGui::BeginPopupModal("New_Prefab_Popup"))
	{
		if (ImGui::InputText("Name", &myNewPrefabNameStorage))
			myHasUniquePrefabName = VerifyUniqueNameForNewAsset();

		if (ImGui::Button("Create"))
		{
			if (!myNewPrefabNameStorage.Empty() && myHasUniquePrefabName)
			{
				myPrefabStorage.CreateNewAsset(myNewPrefabNameStorage.GetBuffer());
				myNewPrefabNameStorage.Clear();
				ImGui::CloseCurrentPopup();
			}
		}

		if (ImGui::IsItemHovered())
		{
			if (myNewPrefabNameStorage.Empty())
				ImGui::SetTooltip("Invalid name");
			else if (!myHasUniquePrefabName)
				ImGui::SetTooltip("Prefab with that name already exists");
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel")) { ImGui::CloseCurrentPopup(); }

		ImGui::EndPopup();
	}
}

void EntityPrefabDockable::HandleSavingAsNewAsset()
{
	if (myWantToOpenSaveAsDialogue)
	{
		ImGui::OpenPopup("Save_As_Popup");
		myWantToOpenSaveAsDialogue = false;
	}

	if (ImGui::BeginPopupModal("Save_As_Popup"))
	{
		if (ImGui::InputText("Name", &myNewPrefabNameStorage))
			myHasUniquePrefabName = VerifyUniqueNameForNewAsset();

		if (ImGui::Button("Save"))
		{
			if (!myNewPrefabNameStorage.Empty() && myHasUniquePrefabName)
			{
				mySelectedPrefab = &myPrefabStorage.CopyAsset(myNewPrefabNameStorage.GetBuffer(), *mySelectedPrefab);
				myNewPrefabNameStorage.Clear();
				ImGui::CloseCurrentPopup();
			}
		}

		if (ImGui::IsItemHovered())
		{
			if (myNewPrefabNameStorage.Empty())
				ImGui::SetTooltip("Invalid name");
			else if (!myHasUniquePrefabName)
				ImGui::SetTooltip("Prefab with that name already exists");
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel")) { ImGui::CloseCurrentPopup(); }

		ImGui::EndPopup();
	}
}

bool EntityPrefabDockable::VerifyUniqueNameForNewAsset() const
{
	const FW_GrowingArray<Slush::Asset*> assets = myPrefabStorage.GetAllAssets();
	for (Slush::Asset* asset : assets)
	{
		EntityPrefab* prefab = static_cast<EntityPrefab*>(asset);
		if (prefab->myName == myNewPrefabNameStorage.GetBuffer())
			return false;
	}

	return true;
}
