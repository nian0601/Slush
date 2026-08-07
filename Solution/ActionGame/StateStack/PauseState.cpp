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
	: myFont(ActionGameGlobals::GetInstance().GetFont())
	, myUIRenderer(ActionGameGlobals::GetInstance().GetFont())
	, myCharacterInfo(aCharacterInfo)
{
	myPlayerHandle = aPlayerHandle;

	myUIBackgroundStyle.SetPadding(16, 16);
	myUIBackgroundStyle.SetChildGap(16);
	myUIBackgroundStyle.SetColor(0xAA121212);
	myUIBackgroundStyle.SetXSizing(Slush::UIElementStyle::FIT);
	myUIBackgroundStyle.SetAlingment(Slush::UIElementStyle::CENTER);

	myUIButtonStyle.SetAlingment(Slush::UIElementStyle::CENTER);
	myUIButtonStyle.SetLayoutDirection(Slush::UIElementStyle::TOP_TO_BOTTOM);
	myUIButtonStyle.SetChildGap(8);
	myUIButtonStyle.SetPadding(16, 16);

	myUIButtonStyle.SetColor(0xFF333333);
	myUIButtonStyle.SetOutlineColor(0xFF000000);
	myUIButtonStyle.SetOutlineThickness(-1.f);
	myUIButtonStyle.EnableButtonInteraction(0xFF888888);

	Slush::Engine::GetInstance().GetWindow().GetRenderer().StartFade(0.15f);
}

Slush::IGameState::GameStateResult PauseState::Update()
{
	Slush::UIBuilder uiBuilder;

	uiBuilder.Start();
	uiBuilder.ScreenFade(myUIBackgroundStyle.myColor);
	uiBuilder.Finish(myUIRenderCommands);

	uiBuilder.Start();

	uiBuilder.TextHeader("Paused!", myFont, 32, myUIBackgroundStyle, 0xFFFFFFFF);

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

	const StatsUpgradeData* upgradeData = stats->GetUpgradeData();

	aUIBUilder.OpenElement(myUIBackgroundStyle).SetLayoutDirection(Slush::UIElementStyle::TOP_TO_BOTTOM);
	for (int i = 0; i < StatType::NUM_STATS; ++i)
	{
		StatType statType = static_cast<StatType>(i);

		Slush::UIElementStyle& style = aUIBUilder.OpenElement();
		style.SetAlingment(Slush::UIElementStyle::CENTER);
		style.SetLayoutDirection(Slush::UIElementStyle::LEFT_TO_RIGHT);
		style.SetChildGap(8);
		style.SetPadding(16, 16);

		style.SetColor(0xFF333333);
		style.SetOutlineColor(0xFF000000);
		style.SetOutlineThickness(-1.f);
		style.EnableButtonInteraction(0xFF888888);

		const StatsUpgradeData::StatData& statdata = upgradeData->myStatDatas[statType];
		if (const Slush::Texture* iconTexture = statdata.myIconTexture.Get())
			aUIBUilder.Image(iconTexture, { 45, 45 }, statdata.myIconTextureRect);

		FW_String upgradeValue;
		upgradeValue += "+";
		upgradeValue += static_cast<int>(stats->GetStatValue(statType) * 100);
		upgradeValue += "%";
		aUIBUilder.Text(upgradeValue.GetBuffer(), ActionGameGlobals::GetInstance().GetFont(), 18, stats->GetStatRank(statType) == 0 ? 0xFFFFFFFF : 0xFF44FF44);

		aUIBUilder.CloseElement();
	}
	aUIBUilder.CloseElement();
}

void PauseState::BuildWeaponsDisplay(Slush::UIBuilder& aUIBUilder)
{
	Slush::Entity* player = myPlayerHandle.Get();
	WeaponComponent* weaponComponent = player->GetComponent<WeaponComponent>();
	const FW_GrowingArray<Weapon*>& weapons = weaponComponent->GetWeapons();

	aUIBUilder.OpenElement(myUIBackgroundStyle).SetLayoutDirection(Slush::UIElementStyle::TOP_TO_BOTTOM);
	for (const Weapon* weapon : weapons)
	{
		const WeaponData& weaponData = weapon->GetWeaponData();

		Slush::UIElementStyle& style = aUIBUilder.OpenElement();
		style.SetAlingment(Slush::UIElementStyle::CENTER);
		style.SetLayoutDirection(Slush::UIElementStyle::TOP_TO_BOTTOM);
		style.SetChildGap(8);
		style.SetPadding(16, 16);

		style.SetColor(0xFF333333);
		style.SetOutlineColor(0xFF000000);
		style.SetOutlineThickness(-1.f);
		style.EnableButtonInteraction(0xFF888888);

		if (const Slush::Texture* iconTexture = weaponData.myIconTexture.Get())
			aUIBUilder.Image(iconTexture, { 45, 45 }, weaponData.myIconTextureRect);

		FW_String weaponName = weaponData.myName;
		weaponName += " ";
		weaponName += weapon->GetRank() + 1;
		aUIBUilder.Text(weaponName.GetBuffer(), ActionGameGlobals::GetInstance().GetFont(), 18, 0xFFFFFFFF);

		aUIBUilder.CloseElement();
	}
	aUIBUilder.CloseElement();
}

void PauseState::BuildCharacterDisplay(Slush::UIBuilder& aUIBuilder)
{
	aUIBuilder.OpenElement().SetAlingment(Slush::UIElementStyle::CENTER);
	
	Slush::UIElementStyle& style = aUIBuilder.OpenElement(myCharacterInfo.myName.GetBuffer());
	style.SetAlingment(Slush::UIElementStyle::CENTER);
	style.SetLayoutDirection(Slush::UIElementStyle::TOP_TO_BOTTOM);
	style.SetChildGap(8);
	style.SetPadding(16, 16);
	style.SetColor(0x00333333);
	style.SetOutlineColor(0x00000000);

	if (const Slush::Texture* iconTexture = myCharacterInfo.myPortaitTexture.Get())
		aUIBuilder.Image(iconTexture, myCharacterInfo.myPortaitTextureRect.myExtents, myCharacterInfo.myPortaitTextureRect);

	aUIBuilder.Text(myCharacterInfo.myName.GetBuffer(), ActionGameGlobals::GetInstance().GetFont(), 25, 0xFFFFFFFF);

	aUIBuilder.CloseElement();
	aUIBuilder.CloseElement();
}

void PauseState::BuildNavigationButtons(Slush::UIBuilder& aUIBUilder)
{
	aUIBUilder.OpenElement(myUIBackgroundStyle);

	aUIBUilder.OpenElement("Resume", myUIButtonStyle);
	aUIBUilder.Text("Resume", ActionGameGlobals::GetInstance().GetFont(), 25, 0xFFFFFFFF);
	aUIBUilder.CloseElement();

	aUIBUilder.OpenElement("Main Menu", myUIButtonStyle);
	aUIBUilder.Text("Main Menu", ActionGameGlobals::GetInstance().GetFont(), 25, 0xFFFFFFFF);
	aUIBUilder.CloseElement();

	aUIBUilder.OpenElement("Quit", myUIButtonStyle);
	aUIBUilder.Text("Quit", ActionGameGlobals::GetInstance().GetFont(), 25, 0xFFFFFFFF);
	aUIBUilder.CloseElement();

	aUIBUilder.CloseElement();
}
