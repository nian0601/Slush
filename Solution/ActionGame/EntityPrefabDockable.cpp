#include "stdafx.h"
#include "EntityPrefabDockable.h"

#include "Core/Engine.h"
#include "Graphics/Window.h"

EntityPrefabDockable::EntityPrefabDockable(Slush::AssetStorage<EntityPrefab>& aPrefabStorage)
	: myPrefabStorage(aPrefabStorage)
	, myNewPrefabNameStorage("")
{
}

void EntityPrefabDockable::OnBuildUI()
{
	const FW_GrowingArray<EntityPrefab*> prefabs = myPrefabStorage.GetAllAssets();

	if (ImGui::Button("New"))
		ImGui::OpenPopup("New Prefab");

	ImGui::SameLine();

	if (ImGui::Button("Save All"))
	{
		for (EntityPrefab* prefab : prefabs)
			prefab->SaveToDisk();
	}

	if (ImGui::BeginPopup("New Prefab"))
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

	

	ImGui::Separator();

	for (EntityPrefab* prefab : prefabs)
	{
		if (ImGui::CollapsingHeader(prefab->myName.GetBuffer()))
		{
			prefab->BuildUI();
		}
	}
}