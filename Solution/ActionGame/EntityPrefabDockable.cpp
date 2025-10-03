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
	//const FW_GrowingArray<Slush::Asset*> prefabs = myPrefabStorage.GetAllAssets();
	//if (!prefabs.IsEmpty())
	//	mySelectedPrefab = static_cast<EntityPrefab*>(prefabs[0]);

	myShouldShowDockable = true;
}

void EntityPrefabDockable::OnUpdate()
{
	//if (Slush::Engine::GetInstance().GetInput().WasKeyReleased(Slush::Input::_F2))
	//{
	//	myShouldShowDockable = !myShouldShowDockable;
	//}
}

void EntityPrefabDockable::OnBuildUI()
{
	const FW_GrowingArray<Slush::Asset*> assets = myPrefabStorage.GetAllAssets();
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File.."))
		{
			if (ImGui::Selectable("New"))
				ImGui::OpenPopup("New_Prefab_Popup");

			if (ImGui::BeginPopup("New_Prefab_Popup"))
			{
				if (ImGui::InputText("Name", &myNewPrefabNameStorage))
				{
					myHasUniquePrefabName = true;
					for (Slush::Asset* asset : assets)
					{
						EntityPrefab* prefab = static_cast<EntityPrefab*>(asset);
						if (prefab->myName == myNewPrefabNameStorage.GetBuffer())
						{
							myHasUniquePrefabName = false;
							break;
						}
					}
				}

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