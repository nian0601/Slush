#include "stdafx.h"

#include "EntityPrefab.h"

#include <FW_FileSystem.h>
#include "ComponentRegistry.h"
#include <imgui\ImGuiWidgets.h>

namespace Slush
{
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

	void EntityPrefab::OnParse(AssetParser::Handle aRootHandle, unsigned int /*aVersion*/)
	{
		myAnyComponentNeedsUpgrade = false;
		for (Component::BaseData* data : myComponentBaseDatas)
		{
			if (data && data->Parse(aRootHandle))
			{
				myAnyComponentNeedsUpgrade = true;
				SLUSH_WARNING("[Asset] '%s' (Entity Prefab): component '%s' needs upgrading, resaving to upgrade", myAssetName.GetBuffer(), data->myComponentLabel);
			}
		}
	}

	bool EntityPrefab::NeedsUpgrade(unsigned int aLoadedVersion) const
	{
		return DataAsset::NeedsUpgrade(aLoadedVersion) || myAnyComponentNeedsUpgrade;
	}

	void EntityPrefab::ResolveDependencies()
	{
		for (Component::BaseData* data : myComponentBaseDatas)
			if (data && data->myEnabled)
				data->ResolveDependencies();
	}

	void EntityPrefab::BuildUI()
	{
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
				if (Slush::ImGuiWidgets::Selectable(someMissingComponents[i].myLabel.GetBuffer()))
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
}
