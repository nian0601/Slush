#include "stdafx.h"

#include "KillRewardComponent.h"

#include <EntitySystem/EntityPrefab.h>
#include <imgui/ImGuiWidgets.h>

#include "Level/Level.h"
#include "TopDownGameGlobals.h"

void KillRewardComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle, unsigned int /*aVersion*/)
{
	aComponentHandle.ParseIntField("reward", myReward);
}

void KillRewardComponent::Data::OnBuildUI()
{
	Slush::ImGuiWidgets::InputInt("Reward", &myReward);
}

KillRewardComponent::KillRewardComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab)
	: Slush::Component(anEntity, anEntityPrefab)
{
	myReward = anEntityPrefab.GetComponentData<KillRewardComponent>().myReward;
}

void KillRewardComponent::OnDeath()
{
	TopDownGameGlobals& globals = TopDownGameGlobals::GetInstance();
	Level& level = globals.GetLevel();
	level.AddResources(myReward);
	SLUSH_INFO("[Resources] +%d from %s, now %d", myReward, myEntityPrefab.GetAssetName().GetBuffer(), level.GetResources());
}
