#include "stdafx.h"

#include "UpgradeStatsState.h"
#include "Components\WeaponComponent.h"
#include "ActionGameGlobals.h"
#include "Components\ExperienceComponent.h"
#include "Components\StatsComponent.h"
#include "EntitySystem\Entity.h"
#include "Graphics\Window.h"
#include "Graphics\Renderer.h"

UpgradeStatsState::UpgradeStatsState(Slush::EntityHandle aPlayerHandle)
	: myUIRenderer(ActionGameGlobals::GetInstance().GetFont())
{
	myPlayerHandle = aPlayerHandle;

	Slush::Engine::GetInstance().GetWindow().GetRenderer().StartFade(0.15f);

	Slush::Entity* player = myPlayerHandle.Get();
	ExperienceComponent* expComp = player->GetComponent<ExperienceComponent>();
	StatsComponent* stats = player->GetComponent<StatsComponent>();

	if (!expComp)
	{
		SLUSH_ERROR("Entity with 'UpgradeStatsState' player role is missing an 'ExperienceComponent'");
		return;
	}

	if (!stats)
	{
		SLUSH_ERROR("Entity with 'UpgradeStatsState' player role is missing a 'StatsComponent'");
		return;
	}

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

Slush::IGameState::GameStateResult UpgradeStatsState::Update()
{
	Slush::Entity* player = myPlayerHandle.Get();
	if (!player)
	{
		SLUSH_ERROR("UpgradeStatsState lost its player entity");
		return Slush::IGameState::POP_SUBSTATE;
	}

	ExperienceComponent* expComp = player->GetComponent<ExperienceComponent>();
	if (!expComp)
	{
		SLUSH_ERROR("Entity with 'UpgradeStatsState' player role is missing an 'ExperienceComponent'");
		return Slush::IGameState::POP_SUBSTATE;
	}

	if (myUpgradeOptions.IsEmpty())
	{
		SLUSH_WARNING("No more available upgrades, auto-leveling");
		expComp->LevelUp();
		return Slush::IGameState::POP_SUBSTATE;
	}

	StatsComponent* stats = player->GetComponent<StatsComponent>();
	if (!stats)
	{
		SLUSH_ERROR("Entity with 'UpgradeStatsState' player role is missing a 'StatsComponent'");
		return Slush::IGameState::POP_SUBSTATE;
	}

	const StatsUpgradeData* upgradeData = stats->GetUpgradeData();

	Slush::UIBuilder uiBuilder;

	uiBuilder.Start(ActionGameGlobals::GetInstance().GetUITheme());
	uiBuilder.ScreenFade();
	uiBuilder.Finish(myUIRenderCommands);

	uiBuilder.Start(ActionGameGlobals::GetInstance().GetUITheme());

	uiBuilder.TextHeader("Leveled Up!");

	uiBuilder.VerticalSpacing(50);

	uiBuilder.BeginPanel().SetAlingment(Slush::UIElementStyle::CENTER);
	for (int i = 0; i < myUpgradeOptions.Count(); ++i)
	{
		uiBuilder.BeginButton(myUpgradeLabels[i].GetBuffer());

		if (upgradeData)
		{
			const StatsUpgradeData::StatData& statdata = upgradeData->myStatDatas[myUpgradeOptions[i]];
			if (const Slush::Texture* iconTexture = statdata.myIconTexture.Get())
				uiBuilder.Image(iconTexture, { 45, 45 }, statdata.myIconTextureRect);

			uiBuilder.Text(myUpgradeLabels[i].GetBuffer());
			FW_String upgradeValue;
			upgradeValue += "+";
			upgradeValue += static_cast<int>(statdata.myIncreasePerUpgrade * 100);
			upgradeValue += "%";
			uiBuilder.Text(upgradeValue.GetBuffer(), Slush::UITextRole::Small, Slush::UITextTone::Positive);
		}

		uiBuilder.EndButton();
	}
	uiBuilder.EndPanel();

	uiBuilder.Finish(myUIRenderCommands);

	for (int i = 0; i < myUpgradeOptions.Count(); ++i)
	{
		if (uiBuilder.WasClicked(myUpgradeLabels[i].GetBuffer()))
		{
			
			stats->UpgradeStat(myUpgradeOptions[i]);
			expComp->LevelUp();
			return Slush::IGameState::POP_SUBSTATE;
		}
	}

	return Slush::IGameState::KEEP;
}

void UpgradeStatsState::Render()
{
	myUIRenderer.Render(myUIRenderCommands);
	myUIRenderCommands.RemoveAll();
}