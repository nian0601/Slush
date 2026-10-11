#include "stdafx.h"

#include "PauseState.h"
#include "ActionGameGlobals.h"
#include "EntitySystem\Entity.h"
#include "Graphics\Window.h"
#include "Graphics\Renderer.h"
#include "Components\StatsComponent.h"
#include "Core\Assets\AssetStorage.h"
#include "Components\WeaponComponent.h"
#include "CharacterInfo.h"
#include "Core\Input.h"

PauseState::PauseState(Slush::EntityHandle aPlayerHandle, const CharacterInfo& aCharacterInfo)
	: myUIRenderer(ActionGameGlobals::GetInstance().GetFont())
	, myCharacterInfo(aCharacterInfo)
{
	myPlayerHandle = aPlayerHandle;

	Slush::Engine::GetInstance().GetWindow().GetRenderer().StartFade(0.15f);
}

Slush::IGameState::GameStateResult PauseState::Update()
{
	Slush::Entity* player = myPlayerHandle.Get();
	if (!player)
	{
		SLUSH_ERROR("PauseState lost its player entity");
		return Slush::IGameState::POP_SUBSTATE;
	}

	Slush::UIBuilder uiBuilder;

	uiBuilder.Start(ActionGameGlobals::GetInstance().GetUITheme());
	uiBuilder.ScreenFade();
	uiBuilder.Finish(myUIRenderCommands);

	uiBuilder.Start(ActionGameGlobals::GetInstance().GetUITheme());

	uiBuilder.TextHeader("Paused!");

	uiBuilder.VerticalSpacing(50);

	uiBuilder.OpenElement().SetLayoutDirection(Slush::UIElementStyle::LEFT_TO_RIGHT).SetAlingment(Slush::UIElementStyle::CENTER);
	BuildStatsDisplay(uiBuilder);

	uiBuilder.HorizontalSpacing(50);
	BuildCharacterDisplay(uiBuilder);
	uiBuilder.HorizontalSpacing(50);

	BuildWeaponsDisplay(uiBuilder);
	uiBuilder.CloseElement();

	uiBuilder.VerticalSpacing(50);
	BuildNavigationButtons(uiBuilder);

	uiBuilder.Finish(myUIRenderCommands);

	if (uiBuilder.WasClicked("Resume"))
	{
		return Slush::IGameState::POP_SUBSTATE;
	}
	else if (uiBuilder.WasClicked("Main Menu"))
	{
		return Slush::IGameState::POP_MAINSTATE;
	}
	else if (uiBuilder.WasClicked("Quit"))
	{
		Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
		window.Close();
	}

	Slush::Engine& engine = Slush::Engine::GetInstance();
	const Slush::Input& input = engine.GetInput();
	if (input.WasKeyReleased(Slush::Input::ESC))
	{
		return Slush::IGameState::POP_SUBSTATE;
	}

	return Slush::IGameState::KEEP;
}

void PauseState::Render()
{
	myUIRenderer.Render(myUIRenderCommands);
	myUIRenderCommands.RemoveAll();
}

void PauseState::BuildStatsDisplay(Slush::UIBuilder& aUIBUilder)
{
	Slush::Entity* player = myPlayerHandle.Get();
	StatsComponent* stats = player->GetComponent<StatsComponent>();
	if (!stats)
	{
		SLUSH_ERROR("Entity with 'PauseState' player role is missing a 'StatsComponent'");
		return;
	}

	const StatsUpgradeData* upgradeData = stats->GetUpgradeData();

	aUIBUilder.BeginPanel().SetAlingment(Slush::UIElementStyle::CENTER).SetLayoutDirection(Slush::UIElementStyle::TOP_TO_BOTTOM);
	for (int i = 0; i < StatType::NUM_STATS; ++i)
	{
		StatType statType = static_cast<StatType>(i);

		aUIBUilder.BeginFrame().SetAlingment(Slush::UIElementStyle::CENTER).SetLayoutDirection(Slush::UIElementStyle::LEFT_TO_RIGHT);

		if (upgradeData)
		{
			const StatsUpgradeData::StatData& statdata = upgradeData->myStatDatas[statType];
			if (const Slush::Texture* iconTexture = statdata.myIconTexture.Get())
				aUIBUilder.Image(iconTexture, { 45, 45 }, statdata.myIconTextureRect);
		}

		FW_String upgradeValue;
		upgradeValue += "+";
		upgradeValue += static_cast<int>(stats->GetStatValue(statType) * 100);
		upgradeValue += "%";
		aUIBUilder.Text(upgradeValue.GetBuffer(), Slush::UITextRole::Small, stats->GetStatRank(statType) == 0 ? Slush::UITextTone::Default : Slush::UITextTone::Positive);

		aUIBUilder.EndFrame();
	}
	aUIBUilder.EndPanel();
}

void PauseState::BuildWeaponsDisplay(Slush::UIBuilder& aUIBUilder)
{
	Slush::Entity* player = myPlayerHandle.Get();
	WeaponComponent* weaponComponent = player->GetComponent<WeaponComponent>();
	if (!weaponComponent)
	{
		SLUSH_ERROR("Entity with 'PauseState' player role is missing a 'WeaponComponent'");
		return;
	}

	const FW_GrowingArray<Weapon*>& weapons = weaponComponent->GetWeapons();

	aUIBUilder.BeginPanel().SetAlingment(Slush::UIElementStyle::CENTER).SetLayoutDirection(Slush::UIElementStyle::TOP_TO_BOTTOM);
	for (const Weapon* weapon : weapons)
	{
		const WeaponData& weaponData = weapon->GetWeaponData();

		aUIBUilder.BeginFrame().SetAlingment(Slush::UIElementStyle::CENTER).SetLayoutDirection(Slush::UIElementStyle::TOP_TO_BOTTOM);

		if (const Slush::Texture* iconTexture = weaponData.myIconTexture.Get())
			aUIBUilder.Image(iconTexture, { 45, 45 }, weaponData.myIconTextureRect);

		FW_String weaponName = weaponData.myName;
		weaponName += " ";
		weaponName += weapon->GetRank() + 1;
		aUIBUilder.Text(weaponName.GetBuffer(), Slush::UITextRole::Small);

		aUIBUilder.EndFrame();
	}
	aUIBUilder.EndPanel();
}

void PauseState::BuildCharacterDisplay(Slush::UIBuilder& aUIBuilder)
{
	aUIBuilder.OpenElement().SetAlingment(Slush::UIElementStyle::CENTER);
	
	Slush::UIElementStyle& style = aUIBuilder.OpenElement(myCharacterInfo.myName.GetBuffer());
	style.SetAlingment(Slush::UIElementStyle::CENTER);
	style.SetLayoutDirection(Slush::UIElementStyle::TOP_TO_BOTTOM);
	style.SetChildGap(8);
	style.SetPadding(16, 16);

	if (const Slush::Texture* iconTexture = myCharacterInfo.myPortaitTexture.Get())
		aUIBuilder.Image(iconTexture, myCharacterInfo.myPortaitTextureRect.myExtents, myCharacterInfo.myPortaitTextureRect);

	aUIBuilder.Text(myCharacterInfo.myName.GetBuffer());

	aUIBuilder.CloseElement();
	aUIBuilder.CloseElement();
}

void PauseState::BuildNavigationButtons(Slush::UIBuilder& aUIBUilder)
{
	aUIBUilder.BeginPanel().SetAlingment(Slush::UIElementStyle::CENTER);

	aUIBUilder.Button("Resume");

	aUIBUilder.Button("Main Menu");

	aUIBUilder.Button("Quit");

	aUIBUilder.EndPanel();
}
