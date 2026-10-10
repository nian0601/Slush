#include "stdafx.h"

#include "LevelSelectState.h"
#include "LevelState.h"
#include "TopDownGameGlobals.h"
#include "Level/LevelData.h"
#include "Core/Assets/AssetStorage.h"
#include "Core/Engine.h"
#include "Core/Input.h"
#include "StateStack/StateStack.h"

namespace
{
	FW_String GetLevelButtonLabel(const LevelData& aLevelData, int anIndex)
	{
		FW_String label;
		if (anIndex < 9)
		{
			label += anIndex + 1;
			label += "  ";
		}
		label += aLevelData.GetAssetName();
		return label;
	}
}

LevelSelectState::LevelSelectState()
	: myUIRenderer(TopDownGameGlobals::GetInstance().GetFont())
	, myFont(TopDownGameGlobals::GetInstance().GetFont())
{
	myUIButtonStyle.SetXSizing(Slush::UIElementStyle::FIXED, 250);
	myUIButtonStyle.SetYSizing(Slush::UIElementStyle::FIXED, 75);
	myUIButtonStyle.SetAlingment(Slush::UIElementStyle::CENTER);
	myUIButtonStyle.SetLayoutDirection(Slush::UIElementStyle::TOP_TO_BOTTOM);
	myUIButtonStyle.SetChildGap(8);
	myUIButtonStyle.SetPadding(16, 16);
	myUIButtonStyle.SetColor(0xFF333333);
	myUIButtonStyle.SetOutlineColor(0xFF000000);
	myUIButtonStyle.SetOutlineThickness(-1.f);
	myUIButtonStyle.EnableButtonInteraction(0xFF888888);
}

Slush::IGameState::GameStateResult LevelSelectState::Update()
{
	Slush::AssetRegistry& assetRegistry = Slush::AssetRegistry::GetInstance();
	const Slush::Input& input = Slush::Engine::GetInstance().GetInput();
	const FW_GrowingArray<Slush::Asset*>& assets = assetRegistry.GetAllAssets<LevelData>();
	FW_GrowingArray<LevelData*> levels;
	for (Slush::Asset* asset : assets)
	{
		LevelData* levelData = static_cast<LevelData*>(asset);
		// Zero-wave levels are harness content, not playable selections.
		if (levelData->myTotalWaveCount > 0)
		{
			levels.Add(levelData);
		}
	}

	Slush::UIBuilder uiBuilder;
	uiBuilder.Start();
	uiBuilder.ScreenFade(0xAA121212);
	uiBuilder.Finish(myUIRenderCommands);

	uiBuilder.Start();
	{
		uiBuilder.OpenElement();
		uiBuilder.GetStyle().SetLayoutDirection(Slush::UIElementStyle::TOP_TO_BOTTOM);
		uiBuilder.GetStyle().SetAlingment(Slush::UIElementStyle::CENTER);
		uiBuilder.Text("Select Level", myFont, 50);
		uiBuilder.VerticalSpacing(60);
		if (levels.Count() == 0)
		{
			uiBuilder.Text("No levels available", myFont, 25);
		}
		for (int index = 0; index < levels.Count(); ++index)
		{
			FW_String label = GetLevelButtonLabel(*levels[index], index);
			uiBuilder.Button(label.GetBuffer(), myFont, 25, myUIButtonStyle, 0xFF333333, 0xFFFFFFFF);
			uiBuilder.VerticalSpacing(20);
		}
		uiBuilder.VerticalSpacing(20);
		uiBuilder.Button("Back", myFont, 25, myUIButtonStyle, 0xFF333333, 0xFFFFFFFF);
		uiBuilder.CloseElement();
	}
	uiBuilder.Finish(myUIRenderCommands);

	for (int index = 0; index < levels.Count(); ++index)
	{
		FW_String label = GetLevelButtonLabel(*levels[index], index);
		if (uiBuilder.WasClicked(label.GetBuffer()) ||
			(index < 9 && input.WasKeyReleased(static_cast<Slush::Input::KeyCode>(Slush::Input::_1 + index))))
		{
			myStateStack->PushSubState(new LevelState(*levels[index]));
			return Slush::IGameState::KEEP;
		}
	}
	if (uiBuilder.WasClicked("Back"))
	{
		return Slush::IGameState::POP_MAINSTATE;
	}
	return Slush::IGameState::KEEP;
}

void LevelSelectState::Render()
{
	myUIRenderer.Render(myUIRenderCommands);
	myUIRenderCommands.RemoveAll();
}