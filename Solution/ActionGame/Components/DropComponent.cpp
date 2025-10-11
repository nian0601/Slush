#include "stdafx.h"

#include "DropComponent.h"
#include <EntitySystem\EntityPrefab.h>
#include <ActionGameGlobals.h>
#include <EntitySystem\EntityManager.h>

void DropComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	if (aComponentHandle.IsReading())
	{
		int numDrops = aComponentHandle.GetNumChildElements();
		myDrops.Reserve(numDrops);
	}

	for (int i = 0; i < myDrops.Count(); ++i)
	{
		Slush::AssetParser::Handle dropHandle;
		if (aComponentHandle.IsReading())
			dropHandle = aComponentHandle.GetChildElementAtIndex(i);
		else
			dropHandle = aComponentHandle.ParseChildElement("drop");

		dropHandle.ParseIntField("weight", myDrops[i].myWeight);
		dropHandle.ParseStringField("prefab", myDrops[i].myPrefabName);
	}
}

void DropComponent::Data::OnBuildUI()
{
	for (int i = 0; i < myDrops.Count(); ++i)
	{
		ImGui::PushID(i);

		DropItem& drop = myDrops[i];

		if (ImGui::Button(" X "))
		{
			myDrops.RemoveNonCyclicAtIndex(i);
			--i;
		}
		else
		{
			ImGui::SameLine();
			ImGui::SetNextItemWidth(50.f);
			ImGui::SliderInt("", &drop.myWeight, 0, 100);

			ImGui::SameLine();
			ImGui::SetNextItemWidth(200.f);
			ImGui::InputText("Prefab", &drop.myPrefabName);
			if (ImGui::BeginDragDropTarget())
			{
				ImGuiDragDropFlags target_flags = 0;
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(EntityPrefab::GetAssetTypeName(), target_flags))
				{
					int assetIndex = *(const int*)payload->Data;
					Slush::AssetRegistry& assets = Slush::AssetRegistry::GetInstance();
					if (const EntityPrefab* prefab = assets.GetAsset<EntityPrefab>(assetIndex))
						drop.myPrefabName = prefab->GetAssetName();
				}
				ImGui::EndDragDropTarget();
			}
		}

		ImGui::PopID();
	}

	if (ImGui::Button("Add.."))
		myDrops.Add();
}

//////////////////////////////////////////////////////////////////////////

DropComponent::DropComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab)
	: Component(anEntity, anEntityPrefab)
	, myData(*static_cast<const DropComponent::Data*>(anEntityPrefab.GetComponentBaseData<DropComponent>()))
{
}

void DropComponent::OnDeath()
{
	int totalWeight = 0;
	for (const Data::DropItem& item : myData.myDrops)
		totalWeight += item.myWeight;

	int pick = FW_RandInt(0, totalWeight);
	for (const Data::DropItem& item : myData.myDrops)
	{
		if (item.myWeight > pick)
		{
			EntityManager& entityManager = ActionGameGlobals::GetInstance().GetEntityManager();
			entityManager.CreateEntity(myEntity.myPosition, item.myPrefabName);
			return;
		}

		pick -= item.myWeight;
	}
}