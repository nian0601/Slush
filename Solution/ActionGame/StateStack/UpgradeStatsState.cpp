#include "stdafx.h"

#include "UpgradeStatsState.h"
#include "Components\WeaponComponent.h"
#include "ActionGameGlobals.h"
#include "Components\ExperienceComponent.h"
#include "EntitySystem\Entity.h"
#include "Graphics\Window.h"

UpgradeStatsState::UpgradeStatsState(EntityHandle aPlayerHandle)
	: myFont(ActionGameGlobals::GetInstance().GetFont())
	, myUIRenderer(ActionGameGlobals::GetInstance().GetFont())
{
	myPlayerHandle = aPlayerHandle;

	myUIBackgroundStyle.SetPadding(16, 16);
	myUIBackgroundStyle.SetChildGap(16);
	myUIBackgroundStyle.SetColor(0xAA121212);
	myUIBackgroundStyle.SetXSizing(Slush::UIElementStyle::FIT);
	myUIBackgroundStyle.SetAlingment(Slush::UIElementStyle::CENTER);

	myUIButtonStyle.SetXSizing(Slush::UIElementStyle::FIXED, 250);
	myUIButtonStyle.SetYSizing(Slush::UIElementStyle::FIXED, 75);
	myUIButtonStyle.SetAlingment(Slush::UIElementStyle::CENTER);
	myUIButtonStyle.SetColor(0xFFAAFFAF);
	myUIButtonStyle.SetOutlineThickness(-1.f);
	myUIButtonStyle.EnableButtonInteraction(0xFFDDDDDD);

	Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
	window.StartFade(0.15f);

	Entity* player = myPlayerHandle.Get();
	ExperienceComponent* expComp = player->GetComponent<ExperienceComponent>();
	StatsComponent* stats = player->GetComponent<StatsComponent>();

	FW_GrowingArray<StatType> potentialUpgrades;
	potentialUpgrades.Add(StatType::COOLDOWN);
	potentialUpgrades.Add(StatType::DAMAGE);
	potentialUpgrades.Add(StatType::EXPERIENCE_GAIN);
	const char* statTypeNames[] = { "Cooldown", "Damage", "Experience" };
	static_assert(IM_ARRAYSIZE(statTypeNames) == StatType::NUM_STATS);

	while (!potentialUpgrades.IsEmpty() && myUpgradeOptions.Count() < 3)
	{
		int statIndex = FW_RandInt(0, potentialUpgrades.Count() - 1);
		StatType statToUpgrade = potentialUpgrades[statIndex];

		if (stats->CanUpgradeStat(statToUpgrade))
		{
			myUpgradeOptions.Add(statToUpgrade);
			myUpgradeLabels.Add(statTypeNames[statToUpgrade]);
		}

		potentialUpgrades.RemoveCyclicAtIndex(statIndex);
	}
}

GameState::GameStateResult UpgradeStatsState::Update()
{
	Entity* player = myPlayerHandle.Get();
	ExperienceComponent* expComp = player->GetComponent<ExperienceComponent>();

	if (myUpgradeOptions.IsEmpty())
	{
		SLUSH_WARNING("No more available upgrades, auto-leveling");
		expComp->LevelUp();
		return GameState::POP_SUBSTATE;
	}
	
	Slush::DynamicUIBuilder uiBuilder;

	uiBuilder.Start();
	uiBuilder.ScreenFade(myUIBackgroundStyle.myColor);
	uiBuilder.Finish(myUIRenderCommands);

	uiBuilder.Start();

	uiBuilder.TextHeader("Leveled Up!", myFont, 32, myUIBackgroundStyle, 0xFFFFFFFF);

	uiBuilder.VerticalSpacing(50);

	uiBuilder.OpenElement(myUIBackgroundStyle);
	for (int i = 0; i < myUpgradeOptions.Count(); ++i)
		uiBuilder.Button(myUpgradeLabels[i].GetBuffer(), myFont, 25, myUIButtonStyle, 0xFFFF3333, 0xFF000000);
	uiBuilder.CloseElement();

	uiBuilder.Finish(myUIRenderCommands);

	for (int i = 0; i < myUpgradeOptions.Count(); ++i)
	{
		if (uiBuilder.WasClicked(myUpgradeLabels[i].GetBuffer()))
		{
			StatsComponent* stats = player->GetComponent<StatsComponent>();
			stats->UpgradeStat(myUpgradeOptions[i]);
			expComp->LevelUp();
			return GameState::POP_SUBSTATE;
		}
	}
	
	return GameState::KEEP;
}

void UpgradeStatsState::Render()
{
	myUIRenderer.Render(myUIRenderCommands);
	myUIRenderCommands.RemoveAll();
}