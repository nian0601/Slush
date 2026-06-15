#include "stdafx.h"

#include "EntityPrefab.h"

#include <FW_FileSystem.h>
#include "ComponentRegistry.h"

void EntityPrefab::ComponentData::Parse(Slush::AssetParser::Handle aRootHandle)
{
	if (aRootHandle.IsReading())
	{
		Slush::AssetParser::Handle componentHandle = aRootHandle.ParseChildElement(myComponentDataName);
		if (componentHandle.IsValid())
		{
			myEnabled = true;
			OnParse(componentHandle);
		}
	}
	else
	{
		if (myEnabled)
		{
			Slush::AssetParser::Handle componentHandle = aRootHandle.ParseChildElement(myComponentDataName);
			if (componentHandle.IsValid())
				OnParse(componentHandle);
		}
	}
}

void EntityPrefab::ComponentData::BuildUI()
{
	FW_String label = myComponentLabel;
	label += " Component";

	if (myEnabled)
	{
		if (ImGui::CollapsingHeader(label.GetBuffer(), &myEnabled))
		{
			ImGui::Indent();
			OnBuildUI();
			ImGui::Unindent();
		}
	}
}

EntityPrefab::EntityPrefab(const char* aName, unsigned int aAssetID)
	: DataAsset(aName, aAssetID)
	, myName(aName)
{
	myComponentBaseDatas.Fill(nullptr);

	for (const IComponentFactory* factory : ComponentRegistry::GetInstance().GetFactories())
		myComponentBaseDatas[factory->GetID()] = factory->CreateComponentData();
}

EntityPrefab::~EntityPrefab()
{
	myComponentBaseDatas.DeleteAll();
}

void EntityPrefab::OnParse(Slush::AssetParser::Handle aRootHandle)
{
	Slush::AssetParser::Handle entityTypeHandle = aRootHandle.ParseChildElement("entitytype");
	if (entityTypeHandle.IsValid())
	{
		int entityTypeAsInt = myEntityType;
		entityTypeHandle.ParseIntField("type", entityTypeAsInt);
		myEntityType = EntityType(entityTypeAsInt);
	}

	for (Component::BaseData* data : myComponentBaseDatas)
	{
		if (data)
			data->Parse(aRootHandle);
	}
}

void EntityPrefab::BuildUI()
{
	const char* entityTypeNames[] = { "Environment", "Player", "NPC", "Player Projectile", "NPC Projectile", "Pickup" };
	static_assert(IM_ARRAYSIZE(entityTypeNames) == EntityType::ENTITYTYPE_COUNT);
	ImGui::Combo("Entity Type", &myEntityType, entityTypeNames, IM_ARRAYSIZE(entityTypeNames));

	FW_GrowingArray<MissingComponent> missingComponents;
	for (Component::BaseData* data : myComponentBaseDatas)
	{
		if (data)
		{
			if (data->myEnabled)
			{
				data->BuildUI();
			}
			else
			{
				missingComponents.Add({ data->myComponentLabel, &data->myEnabled });
			}
		}
	}

	BuildMissingComponentsUI(missingComponents);
}

void EntityPrefab::BuildMissingComponentsUI(const FW_GrowingArray<MissingComponent>& someMissingComponents)
{
	if (ImGui::Button("Add Component"))
		ImGui::OpenPopup("add_component_popup");

	if (ImGui::BeginPopup("add_component_popup"))
	{
		for (int i = 0; i < someMissingComponents.Count(); ++i)
			if (ImGui::Selectable(someMissingComponents[i].myLabel.GetBuffer()))
				*someMissingComponents[i].myEnabledFlag = true;

		ImGui::EndPopup();
	}
}

bool EntityPrefab::Has(int aComponentID) const
{
	Component::BaseData* baseData = myComponentBaseDatas[aComponentID];
	if (baseData)
		return baseData->myEnabled;

	return false;
}