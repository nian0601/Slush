#include "stdafx.h"

#include "UpgradeWeaponState.h"
#include "Components\WeaponComponent.h"
#include "EntitySystem\Entity.h"
#include "ActionGameGlobals.h"
#include "Graphics\Window.h"
#include "Graphics\Renderer.h"

UpgradeWeaponState::UpgradeWeaponState(Slush::EntityHandle aPlayerHandle)
	: myUIRenderer(ActionGameGlobals::GetInstance().GetFont())
{
	myPlayerHandle = aPlayerHandle;

	Slush::Engine::GetInstance().GetWindow().GetRenderer().StartFade(0.15f);
}

Slush::IGameState::GameStateResult UpgradeWeaponState::Update()
{
	Slush::Entity* player = myPlayerHandle.Get();
	if (!player)
	{
		SLUSH_ERROR("UpgradeWeaponState lost its player entity");
		return Slush::IGameState::POP_SUBSTATE;
	}

	WeaponComponent* weaponComponent = player->GetComponent<WeaponComponent>();
	if (!weaponComponent)
	{
		SLUSH_ERROR("Entity with 'UpgradeWeaponState' player role is missing a 'WeaponComponent'");
		return Slush::IGameState::POP_SUBSTATE;
	}

	if (!weaponComponent->HasPendingUpgrade())
		return Slush::IGameState::POP_SUBSTATE;

	Slush::UIBuilder uiBuilder;

	uiBuilder.Start(ActionGameGlobals::GetInstance().GetUITheme());
	uiBuilder.ScreenFade();
	uiBuilder.Finish(myUIRenderCommands);

	weaponComponent->HandleUpgrading(uiBuilder);
	uiBuilder.GenerateRenderCommands(myUIRenderCommands);

	return Slush::IGameState::KEEP;
}

void UpgradeWeaponState::Render()
{
	myUIRenderer.Render(myUIRenderCommands);
	myUIRenderCommands.RemoveAll();
}

