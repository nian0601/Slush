#include "stdafx.h"

#include "CharacterSelectionState.h"
#include "ActionGameGlobals.h"
#include "CharacterInfo.h"
#include "Graphics\Window.h"
#include "Core\Assets\AssetStorage.h"
#include "LevelState.h"
#include "StateStack.h"
#include "Level\Tilemap.h"

CharacterSelectionState::CharacterSelectionState()
	: myFont(ActionGameGlobals::GetInstance().GetFont())
	, myUIRenderer(ActionGameGlobals::GetInstance().GetFont())
{
	myTilemap = new Tilemap();

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

	Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
	window.StartFade(0.15f);
}

CharacterSelectionState::~CharacterSelectionState()
{
	FW_SAFE_DELETE(myTilemap);
}

GameState::GameStateResult CharacterSelectionState::Update()
{
	Slush::AssetRegistry& assets = Slush::AssetRegistry::GetInstance();
	const FW_GrowingArray<Slush::Asset*>& charAssets = assets.GetAllAssets<CharacterInfo>();

	Slush::DynamicUIBuilder uiBuilder;

	uiBuilder.Start();
	uiBuilder.ScreenFade(myUIBackgroundStyle.myColor);
	uiBuilder.Finish(myUIRenderCommands);

	uiBuilder.Start();

	uiBuilder.TextHeader("Choose your Hero!", myFont, 32, myUIBackgroundStyle, 0xFFFFFFFF);

	uiBuilder.VerticalSpacing(50);

	BuildCharacterDisplay(uiBuilder);

	uiBuilder.VerticalSpacing(50);

	uiBuilder.OpenElement("Back", myUIButtonStyle);
	uiBuilder.Text("Back", ActionGameGlobals::GetInstance().GetFont(), 25, 0xFFFFFFFF);
	uiBuilder.CloseElement();

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
		return GameState::POP_MAINSTATE;

	return GameState::KEEP;
}

void CharacterSelectionState::Render()
{
	myTilemap->Render();

	myUIRenderer.Render(myUIRenderCommands);
	myUIRenderCommands.RemoveAll();
}

void CharacterSelectionState::BuildCharacterDisplay(Slush::DynamicUIBuilder& aUIBUilder)
{
	Slush::AssetRegistry& assets = Slush::AssetRegistry::GetInstance();
	const FW_GrowingArray<Slush::Asset*>& charAssets = assets.GetAllAssets<CharacterInfo>();

	aUIBUilder.OpenElement(myUIBackgroundStyle);
	for (Slush::Asset* charAsset : charAssets)
	{
		CharacterInfo* charInfo = static_cast<CharacterInfo*>(charAsset);
		Slush::UIElementStyle& style = aUIBUilder.OpenElement(charInfo->myName.GetBuffer());
		style.SetAlingment(Slush::UIElementStyle::CENTER);
		style.SetLayoutDirection(Slush::UIElementStyle::TOP_TO_BOTTOM);
		style.SetChildGap(8);
		style.SetPadding(16, 16);

		style.SetColor(0xFF333333);
		style.SetOutlineColor(0xFF000000);
		style.SetOutlineThickness(-1.f);
		style.EnableButtonInteraction(0xFF888888);

		if (const Slush::Texture* iconTexture = assets.GetAsset<Slush::Texture>(charInfo->myPortaitTextureID))
			aUIBUilder.Image(iconTexture, charInfo->myPortaitTextureRect.myExtents, charInfo->myPortaitTextureRect);

		aUIBUilder.Text(charInfo->myName.GetBuffer(), ActionGameGlobals::GetInstance().GetFont(), 25, 0xFFFFFFFF);

		aUIBUilder.CloseElement();
	}
	aUIBUilder.CloseElement();
}