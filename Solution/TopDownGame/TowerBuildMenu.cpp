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

void TowerBuildMenu::Update()
{
	Slush::Engine& engine = Slush::Engine::GetInstance();
	Slush::Renderer& renderer = engine.GetWindow().GetRenderer();
	Slush::Font& font = TopDownGameGlobals::GetInstance().GetFont();
	Slush::AssetRegistry& assetRegistry = Slush::AssetRegistry::GetInstance();
	const FW_GrowingArray<Slush::Asset*>& prefabs = assetRegistry.GetAllAssets<Slush::EntityPrefab>();

	myRenderCommands.RemoveAll();
	Slush::UIBuilder uiBuilder;
	uiBuilder.Start();
	uiBuilder.OpenElement().SetXSizing(Slush::UIElementStyle::GROW)
		.SetYSizing(Slush::UIElementStyle::GROW)
		.SetLayoutDirection(Slush::UIElementStyle::TOP_TO_BOTTOM);

	uiBuilder.OpenElement().SetXSizing(Slush::UIElementStyle::GROW)
		.SetYSizing(Slush::UIElementStyle::GROW);
	uiBuilder.CloseElement();

	uiBuilder.OpenElement().SetXSizing(Slush::UIElementStyle::GROW)
		.SetYSizing(Slush::UIElementStyle::FIXED, ourBarHeight)
		.SetLayoutDirection(Slush::UIElementStyle::LEFT_TO_RIGHT)
		.SetPadding(16, 12).SetChildGap(12).SetColor(0xDD18202A);

	Slush::UIElementStyle buttonStyle;
	buttonStyle.SetXSizing(Slush::UIElementStyle::FIXED, 230)
		.SetYSizing(Slush::UIElementStyle::FIXED, 64)
		.SetLayoutDirection(Slush::UIElementStyle::TOP_TO_BOTTOM)
		.SetAlingment(Slush::UIElementStyle::CENTER);

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
		const int buttonColor = !isAffordable ? 0xFF242D36 : (isSelected ? 0xFF4379A6 : 0xFF334453);
		buttonStyle.EnableButtonInteraction(isAffordable ? 0xFF60758A : 0xFF34414C);
		buttonStyle.SetOutlineColor(isSelected ? 0xFF81C9FF : 0x00000000);
		buttonStyle.SetOutlineThickness(isSelected ? 2.f : 0.f);
		uiBuilder.Button(label.GetBuffer(), font, 20, buttonStyle, buttonColor, 0xFFFFFFFF);
	}
	if (myFeedbackTimer.IsStarted() && !myFeedbackTimer.HasExpired())
		uiBuilder.Text(myFeedbackText.GetBuffer(), font, 18, 0xFFFFD17A);

	uiBuilder.CloseElement();
	uiBuilder.CloseElement();
	uiBuilder.Finish(myRenderCommands);

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
	const float barTop = renderer.GetOffscreenBufferSize().y - ourBarHeight;
	if (mySelectedTowerPrefab && engine.GetInput().WasMouseReleased(Slush::Input::LEFTMB)
		&& mousePosition.y < barTop)
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
	}
}

void TowerBuildMenu::RenderPreview() const
{
	if (!mySelectedTowerPrefab)
		return;

	Slush::Engine& engine = Slush::Engine::GetInstance();
	Slush::Renderer& renderer = engine.GetWindow().GetRenderer();
	const Vector2f mousePosition = engine.GetInput().GetMousePositionf();
	if (mousePosition.y >= renderer.GetOffscreenBufferSize().y - ourBarHeight)
		return;

	const Slush::SpriteComponent::Data& spriteData = mySelectedTowerPrefab->GetComponentData<Slush::SpriteComponent>();
	const Vector2f halfSize = spriteData.mySize * 0.5f;
	const Vector2f topLeft = mousePosition - halfSize;
	const Vector2f bottomRight = mousePosition + halfSize;
	const int outlineColor = 0xFF66CCFF;
	renderer.RenderLine(topLeft, Vector2f{ bottomRight.x, topLeft.y }, outlineColor);
	renderer.RenderLine(Vector2f{ bottomRight.x, topLeft.y }, bottomRight, outlineColor);
	renderer.RenderLine(bottomRight, Vector2f{ topLeft.x, bottomRight.y }, outlineColor);
	renderer.RenderLine(Vector2f{ topLeft.x, bottomRight.y }, topLeft, outlineColor);
}
