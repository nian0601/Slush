#include "stdafx.h"

#include "UpgradeWeaponState.h"
#include "Components\WeaponComponent.h"
#include "EntitySystem\Entity.h"
#include "ActionGameGlobals.h"

UpgradeWeaponState::UpgradeWeaponState(EntityHandle aPlayerHandle)
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
}

GameState::GameStateResult UpgradeWeaponState::Update()
{
	Entity* player = myPlayerHandle.Get();
	WeaponComponent* weaponComponent = player->GetComponent<WeaponComponent>();
	Slush::DynamicUIBuilder uiBuilder;

	uiBuilder.Start();
	uiBuilder.ScreenFade(myUIBackgroundStyle.myColor);
	uiBuilder.Finish(myUIRenderCommands);

	weaponComponent->HandleUpgrading(uiBuilder);
	uiBuilder.GenerateRenderCommands(myUIRenderCommands);

	if (weaponComponent->HasPendingUpgrade())
		return GameState::KEEP;

	return GameState::POP_SUBSTATE;
}

void UpgradeWeaponState::Render()
{
	myUIRenderer.Render(myUIRenderCommands);
	myUIRenderCommands.RemoveAll();
}

