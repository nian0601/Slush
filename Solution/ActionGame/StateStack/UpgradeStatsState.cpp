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
}

GameState::GameStateResult UpgradeStatsState::Update()
{
	Entity* player = myPlayerHandle.Get();
	ExperienceComponent* expComp = player->GetComponent<ExperienceComponent>();

	bool leveledUp = false;
	if (StatsComponent* stats = player->GetComponent<StatsComponent>())
	{
		if (!stats->CanUpgradeCooldownReduction() && !stats->CanUpgradeDamage() && !stats->CanUpgradeExperience())
		{
			expComp->LevelUp();
			leveledUp = true;
			SLUSH_WARNING("No more available upgrades, auto-leveling");
		}
		else
		{
			Slush::DynamicUIBuilder uiBuilder;

			uiBuilder.Start();
			uiBuilder.ScreenFade(myUIBackgroundStyle.myColor);
			uiBuilder.Finish(myUIRenderCommands);

			uiBuilder.Start();

			uiBuilder.TextHeader("Leveled Up!", myFont, 32, myUIBackgroundStyle, 0xFFFFFFFF);

			uiBuilder.VerticalSpacing(50);

			uiBuilder.OpenElement("ButtonBackground", myUIBackgroundStyle);

			if (stats->CanUpgradeCooldownReduction())
				uiBuilder.Button("Cooldown", myFont, 25, myUIButtonStyle, 0xFFFF3333, 0xFF000000);

			if (stats->CanUpgradeDamage())
				uiBuilder.Button("Damage", myFont, 25, myUIButtonStyle, 0xFFFFFF33, 0xFF000000);

			if (stats->CanUpgradeExperience())
				uiBuilder.Button("Experience", myFont, 25, myUIButtonStyle, 0xFF33FF33, 0xFF000000);

			uiBuilder.CloseElement(); // ButtonBackground

			uiBuilder.Finish(myUIRenderCommands);


			if (stats->CanUpgradeCooldownReduction() && uiBuilder.WasClicked("Cooldown"))
			{
				stats->AddCooldownReductionUpgrade();
				expComp->LevelUp();
				leveledUp = true;
			}
			if (stats->CanUpgradeDamage() && uiBuilder.WasClicked("Damage"))
			{
				stats->AddDamageUpgrade();
				expComp->LevelUp();
				leveledUp = true;
			}
			if (stats->CanUpgradeExperience() && uiBuilder.WasClicked("Experience"))
			{
				stats->AddExperienceUpgrade();
				expComp->LevelUp();
				leveledUp = true;
			}
		}
	}
	else
	{
		SLUSH_ERROR("Player doesnt have a StatsComponent, not able to pick upgrade when Leveling");
		expComp->LevelUp();
		leveledUp = true;
	}

	if (leveledUp)
		return GameState::POP_SUBSTATE;

	return GameState::KEEP;
}

void UpgradeStatsState::Render()
{
	myUIRenderer.Render(myUIRenderCommands);
	myUIRenderCommands.RemoveAll();
}