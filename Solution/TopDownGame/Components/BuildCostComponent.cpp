#include "stdafx.h"

#include "BuildCostComponent.h"

#include <EntitySystem/EntityPrefab.h>
#include <imgui/ImGuiWidgets.h>

void BuildCostComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle, unsigned int /*aVersion*/)
{
	aComponentHandle.ParseIntField("cost", myCost);
}

void BuildCostComponent::Data::OnBuildUI()
{
	Slush::ImGuiWidgets::InputInt("Cost", &myCost);
}

BuildCostComponent::BuildCostComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab)
	: Slush::Component(anEntity, anEntityPrefab)
{
}

int BuildCostComponent::GetCost(const Slush::EntityPrefab& aPrefab)
{
	if (!aPrefab.Has<BuildCostComponent>())
		return 0;

	return aPrefab.GetComponentData<BuildCostComponent>().myCost;
}
