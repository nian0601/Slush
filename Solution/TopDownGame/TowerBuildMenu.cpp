#include "stdafx.h"

#include "TowerBuildMenu.h"

#include "Components/BuildCostComponent.h"
#include "Components/TowerCombatComponent.h"
#include "Level/Level.h"
#include "TopDownGameGlobals.h"

#include <Core/Assets/AssetStorage.h>
#include <Core/Engine.h>
#include <Core/Input.h>
#include <EntitySystem/Components/SpriteComponent.h>
#include <EntitySystem/EntityPrefab.h>
#include <Graphics/Renderer.h>
#include <Graphics/Window.h>

TowerBuildMenu::TowerBuildMenu(Level& aLevel)
	: myLevel(aLevel)
{
}

void TowerBuildMenu::Update(bool aIsMouseOverOtherUI)
{
	Slush::Engine& engine = Slush::Engine::GetInstance();
	Slush::AssetRegistry& assetRegistry = Slush::AssetRegistry::GetInstance();
	const FW_GrowingArray<Slush::Asset*>& prefabs = assetRegistry.GetAllAssets<Slush::EntityPrefab>();

	myRenderCommands.RemoveAll();
	Slush::UIBuilder uiBuilder;
	uiBuilder.Start(TopDownGameGlobals::GetInstance().GetUITheme());
	uiBuilder.BeginPanel().SetXSizing(Slush::UIElementStyle::GROW)
		.SetAnchor(Slush::UIElementStyle::AnchorPoint::BOTTOM_LEFT)
		.SetYSizing(Slush::UIElementStyle::FIXED, ourBarHeight)
		.SetLayoutDirection(Slush::UIElementStyle::LEFT_TO_RIGHT)
		.SetPadding(16, 12).SetChildGap(12);


	for (Slush::Asset* asset : prefabs)
	{
		Slush::EntityPrefab* prefab = static_cast<Slush::EntityPrefab*>(asset);
		if (!TowerCombatComponent::IsBuildableTowerPrefab(*prefab))
			continue;

		FW_String label = prefab->GetAssetName();
		label += " (";
		label += BuildCostComponent::GetCost(*prefab);
		label += ")";
		const bool isSelected = prefab == mySelectedTowerPrefab;
		const bool isAffordable = BuildCostComponent::GetCost(*prefab) <= myLevel.GetResources();
		const Slush::UIState state = isSelected ? Slush::UIState::Selected : (isAffordable ? Slush::UIState::Normal : Slush::UIState::Disabled);
		uiBuilder.BeginButton(label.GetBuffer(), state)
			.SetXSizing(Slush::UIElementStyle::FIXED, 230).SetYSizing(Slush::UIElementStyle::FIXED, 64);
		uiBuilder.Text(label.GetBuffer());
		if (state == Slush::UIState::Disabled)
		{
			FW_String tooltip = "Not enough resources (need ";
			tooltip += BuildCostComponent::GetCost(*prefab);
			tooltip += ", have ";
			tooltip += myLevel.GetResources();
			tooltip += ")";
			uiBuilder.SetTooltip(tooltip.GetBuffer());
		}
		uiBuilder.EndButton();
	}
	if (myFeedbackTimer.IsStarted() && !myFeedbackTimer.HasExpired())
		uiBuilder.Text(myFeedbackText.GetBuffer(), Slush::UITextRole::Small, Slush::UITextTone::Warning);

	uiBuilder.EndPanel();
	uiBuilder.Finish(myRenderCommands);
	myIsMouseOverUI = aIsMouseOverOtherUI || uiBuilder.IsMouseOverUI();

	if (engine.GetInput().WasMouseReleased(Slush::Input::RIGHTMB))
		mySelectedTowerPrefab = nullptr;

	for (Slush::Asset* asset : prefabs)
	{
		Slush::EntityPrefab* prefab = static_cast<Slush::EntityPrefab*>(asset);
		if (!TowerCombatComponent::IsBuildableTowerPrefab(*prefab))
			continue;

		FW_String label = prefab->GetAssetName();
		label += " (";
		label += BuildCostComponent::GetCost(*prefab);
		label += ")";
		if (uiBuilder.WasClicked(label.GetBuffer()))
			mySelectedTowerPrefab = prefab;
	}

	const Vector2f mousePosition = engine.GetInput().GetMousePositionf();
	if (mySelectedTowerPrefab && engine.GetInput().WasMouseReleased(Slush::Input::LEFTMB)
		&& !myIsMouseOverUI)
	{
		const PlaceTowerResult result = myLevel.TryPlaceTower(*mySelectedTowerPrefab, mousePosition);
		if (result == PlaceTowerResult::NotAffordable)
		{
			myFeedbackText = "Not enough resources (need ";
			myFeedbackText += BuildCostComponent::GetCost(*mySelectedTowerPrefab);
			myFeedbackText += ", have ";
			myFeedbackText += myLevel.GetResources();
			myFeedbackText += ")";
			myFeedbackTimer.Start(2.f);
		}
		else if (result == PlaceTowerResult::FootprintBlocked)
		{
			myFeedbackText = "Can't build there";
			myFeedbackTimer.Start(2.f);
		}
		else if (result == PlaceTowerResult::TooCloseToEndpoint)
		{
			myFeedbackText = "Too close to the start or goal";
			myFeedbackTimer.Start(2.f);
		}
	}
}

void TowerBuildMenu::RenderPreview() const
{
	if (!mySelectedTowerPrefab)
		return;

	Slush::Engine& engine = Slush::Engine::GetInstance();
	Slush::Renderer& renderer = engine.GetWindow().GetRenderer();
	const int outlineColor = 0xFF66CCFF;

	// The no-build zones around start and goal, for as long as a tower is selected.
	const LevelData& levelData = myLevel.GetLevelDataAsset();
	renderer.RenderCircle(levelData.myStartPosition, levelData.myEndpointClearance, nullptr, Recti(), 0, outlineColor, 2.f);
	renderer.RenderCircle(levelData.myGoalPosition, levelData.myEndpointClearance, nullptr, Recti(), 0, outlineColor, 2.f);

	if (myIsMouseOverUI)
		return;

	const Vector2f mousePosition = engine.GetInput().GetMousePositionf();

	const Slush::SpriteComponent::Data& spriteData = mySelectedTowerPrefab->GetComponentData<Slush::SpriteComponent>();
	const Vector2f halfSize = spriteData.mySize * 0.5f;
	const Vector2f topLeft = mousePosition - halfSize;
	const Vector2f bottomRight = mousePosition + halfSize;
	renderer.RenderLine(topLeft, Vector2f{ bottomRight.x, topLeft.y }, outlineColor);
	renderer.RenderLine(Vector2f{ bottomRight.x, topLeft.y }, bottomRight, outlineColor);
	renderer.RenderLine(bottomRight, Vector2f{ topLeft.x, bottomRight.y }, outlineColor);
	renderer.RenderLine(Vector2f{ topLeft.x, bottomRight.y }, topLeft, outlineColor);
}
