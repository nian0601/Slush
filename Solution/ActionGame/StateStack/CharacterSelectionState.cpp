#include "stdafx.h"

#include "CharacterSelectionState.h"
#include "ActionGameGlobals.h"
#include "CharacterInfo.h"
#include "Graphics\Window.h"
#include "Graphics\Renderer.h"
#include "Core\Assets\AssetStorage.h"
#include "LevelState.h"
#include "StateStack/StateStack.h"
#include "Level\Tilemap.h"

CharacterSelectionState::CharacterSelectionState()
	: myUIRenderer(ActionGameGlobals::GetInstance().GetFont())
{
	myTilemap = new Tilemap();

	Slush::Engine::GetInstance().GetWindow().GetRenderer().StartFade(0.15f);
}

CharacterSelectionState::~CharacterSelectionState()
{
	FW_SAFE_DELETE(myTilemap);
}

Slush::IGameState::GameStateResult CharacterSelectionState::Update()
{
	Slush::AssetRegistry& assets = Slush::AssetRegistry::GetInstance();
	const FW_GrowingArray<Slush::Asset*>& charAssets = assets.GetAllAssets<CharacterInfo>();

	if (ActionGameGlobals::GetInstance().myDebugSettings.mySkipStartScreen && !charAssets.IsEmpty())
	{
		CharacterInfo* charInfo = static_cast<CharacterInfo*>(charAssets[0]);
		myStateStack->PushSubState(new LevelState(*charInfo));
		return Slush::IGameState::KEEP;
	}

	Slush::UIBuilder uiBuilder;

	uiBuilder.Start(ActionGameGlobals::GetInstance().GetUITheme());
	uiBuilder.ScreenFade();
	uiBuilder.Finish(myUIRenderCommands);

	uiBuilder.Start(ActionGameGlobals::GetInstance().GetUITheme());

	uiBuilder.TextHeader("Choose your Hero!");

	uiBuilder.VerticalSpacing(50);

	BuildCharacterDisplay(uiBuilder);

	uiBuilder.VerticalSpacing(50);

	uiBuilder.Button("Back");

	uiBuilder.Finish(myUIRenderCommands);

	for (Slush::Asset* charAsset : charAssets)
	{
		CharacterInfo* charInfo = static_cast<CharacterInfo*>(charAsset);
		if (uiBuilder.WasClicked(charInfo->myName.GetBuffer()))
		{
			myStateStack->PushSubState(new LevelState(*charInfo));
		}
	}

	if (uiBuilder.WasClicked("Back"))
		return Slush::IGameState::POP_MAINSTATE;

	return Slush::IGameState::KEEP;
}

void CharacterSelectionState::Render()
{
	myTilemap->Render();

	myUIRenderer.Render(myUIRenderCommands);
	myUIRenderCommands.RemoveAll();
}

void CharacterSelectionState::BuildCharacterDisplay(Slush::UIBuilder& aUIBUilder)
{
	Slush::AssetRegistry& assets = Slush::AssetRegistry::GetInstance();
	const FW_GrowingArray<Slush::Asset*>& charAssets = assets.GetAllAssets<CharacterInfo>();

	aUIBUilder.BeginPanel().SetAlingment(Slush::UIElementStyle::CENTER);
	for (Slush::Asset* charAsset : charAssets)
	{
		CharacterInfo* charInfo = static_cast<CharacterInfo*>(charAsset);
		aUIBUilder.BeginButton(charInfo->myName.GetBuffer());

		if (const Slush::Texture* iconTexture = charInfo->myPortaitTexture.Get())
			aUIBUilder.Image(iconTexture, charInfo->myPortaitTextureRect.myExtents, charInfo->myPortaitTextureRect);

		aUIBUilder.Text(charInfo->myName.GetBuffer());

		aUIBUilder.EndButton();
	}
	aUIBUilder.EndPanel();
}