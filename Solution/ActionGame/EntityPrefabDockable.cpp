#include "stdafx.h"

#include "EntityPrefabDockable.h"

#include "Core/Engine.h"
#include "Graphics/Window.h"

EntityPrefabDockable::EntityPrefabDockable(Slush::AssetStorage<EntityPrefab>& aPrefabStorage)
	: Dockable(true)
	, myPrefabStorage(aPrefabStorage)
	, myNewPrefabNameStorage("")
{
	const FW_GrowingArray<EntityPrefab*> prefabs = myPrefabStorage.GetAllAssets();
	if (!prefabs.IsEmpty())
		mySelectedPrefab = prefabs[0];
}

void EntityPrefabDockable::OnBuildUI()
{
	const FW_GrowingArray<EntityPrefab*> prefabs = myPrefabStorage.GetAllAssets();
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
					for (EntityPrefab* prefab : prefabs)
					{
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
				for (EntityPrefab* prefab : prefabs)
					prefab->Save();
			}

			if (ImGui::BeginMenu("Open.."))
			{
				for (EntityPrefab* prefab : prefabs)
				{
					if (ImGui::Selectable(prefab->myName.GetBuffer()))
						mySelectedPrefab = prefab;
				}

				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	if (mySelectedPrefab)
		mySelectedPrefab->BuildUI();
}