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
{
}

void LevelSelectState::StartState()
{
	Slush::AssetRegistry& assetRegistry = Slush::AssetRegistry::GetInstance();
	const FW_GrowingArray<Slush::Asset*>& assets = assetRegistry.GetAllAssets<LevelData>();
	myLevels.RemoveAll();
	for (Slush::Asset* asset : assets)
	{
		LevelData* levelData = static_cast<LevelData*>(asset);
		// Zero-wave levels are harness content, not playable selections.
		if (levelData->myTotalWaveCount > 0)
		{
			myLevels.Add(levelData);
		}
	}
}

Slush::IGameState::GameStateResult LevelSelectState::Update()
{
	const Slush::Input& input = Slush::Engine::GetInstance().GetInput();

	Slush::UIBuilder uiBuilder;
	uiBuilder.Start(TopDownGameGlobals::GetInstance().GetUITheme());
	uiBuilder.ScreenFade();
	uiBuilder.Finish(myUIRenderCommands);

	uiBuilder.Start(TopDownGameGlobals::GetInstance().GetUITheme());
	{
		uiBuilder.OpenElement();
		uiBuilder.GetStyle().SetLayoutDirection(Slush::UIElementStyle::TOP_TO_BOTTOM);
		uiBuilder.GetStyle().SetAlingment(Slush::UIElementStyle::CENTER);
		uiBuilder.Text("Select Level", Slush::UITextRole::Title);
		uiBuilder.VerticalSpacing(60);
		if (myLevels.Count() == 0)
		{
			uiBuilder.Text("No levels available");
		}
		for (int index = 0; index < myLevels.Count(); ++index)
		{
			FW_String label = GetLevelButtonLabel(*myLevels[index], index);
			uiBuilder.Button(label.GetBuffer(), Slush::UIState::Normal, { 250, 75 });
			uiBuilder.VerticalSpacing(20);
		}
		uiBuilder.VerticalSpacing(20);
		uiBuilder.Button("Back", Slush::UIState::Normal, { 250, 75 });
		uiBuilder.CloseElement();
	}
	uiBuilder.Finish(myUIRenderCommands);

	for (int index = 0; index < myLevels.Count(); ++index)
	{
		FW_String label = GetLevelButtonLabel(*myLevels[index], index);
		if (uiBuilder.WasClicked(label.GetBuffer()) ||
			(index < 9 && input.WasKeyReleased(static_cast<Slush::Input::KeyCode>(Slush::Input::_1 + index))))
		{
			myStateStack->PushSubState(new LevelState(*myLevels[index]));
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
