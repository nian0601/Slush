#include "stdafx.h"

#include "LevelEditorDockable.h"

#include "Level/Level.h"
#include "Level/LevelData.h"
#include "Level/NavmeshData.h"
#include "Components/TowerCombatComponent.h"

#include "Core/Assets/AssetStorage.h"
#include "Core/CommandLineArgs.h"
#include "Core/Dockables/IAppLayout.h"
#include "Core/Engine.h"
#include "Core/Input.h"
#include "Graphics/Renderer.h"
#include "Graphics/Window.h"
#include <EntitySystem/Components/SpriteComponent.h>
#include <EntitySystem/EntityPrefab.h>
#include <imgui/ImGuiWidgets.h>

LevelEditorDockable::LevelEditorDockable(Level& aLevel)
	: Slush::DockableBase<LevelEditorDockable>(true)
	, myLevel(aLevel)
{
	myLevel.GetNavmesh().GetAsTriangleMesh(myTriangleMesh);
}

void LevelEditorDockable::RenderOverlay() const
{
	Slush::Engine& engine = Slush::Engine::GetInstance();
	Slush::Renderer& renderer = engine.GetWindow().GetRenderer();
	for (int i = 0; i < myTriangleMesh.myVertices.Count(); i += 3)
	{
		renderer.RenderTriangle(myTriangleMesh.myVertices[i], myTriangleMesh.myVertices[i + 1], myTriangleMesh.myVertices[i + 2], 0xFF004400);
	}

	RenderBoxCutPreview();
	RenderManualCutPreview();
	RenderStartAndGoalMarkers();
}

void LevelEditorDockable::RenderBoxCutPreview() const
{
	if (!myIsBoxCutModeActive || !myBoxCutHasStartCorner)
		return;

	Slush::Engine& engine = Slush::Engine::GetInstance();
	Slush::Renderer& renderer = engine.GetWindow().GetRenderer();
	const Vector2f& mousePos = engine.GetInput().GetMousePositionf();

	const Vector2f topLeft{ FW_Min(myBoxCutStartCorner.x, mousePos.x), FW_Min(myBoxCutStartCorner.y, mousePos.y) };
	const Vector2f bottomRight{ FW_Max(myBoxCutStartCorner.x, mousePos.x), FW_Max(myBoxCutStartCorner.y, mousePos.y) };
	const Vector2f topRight{ bottomRight.x, topLeft.y };
	const Vector2f bottomLeft{ topLeft.x, bottomRight.y };

	const int boxPreviewColor = 0xFF00FF00;
	renderer.RenderLine(topLeft, topRight, boxPreviewColor);
	renderer.RenderLine(topRight, bottomRight, boxPreviewColor);
	renderer.RenderLine(bottomRight, bottomLeft, boxPreviewColor);
	renderer.RenderLine(bottomLeft, topLeft, boxPreviewColor);
}

void LevelEditorDockable::RenderManualCutPreview() const
{
	if (!myIsManualCutModeActive || myCutPositions.IsEmpty())
		return;

	Slush::Engine& engine = Slush::Engine::GetInstance();
	Slush::Renderer& renderer = engine.GetWindow().GetRenderer();
	const Vector2f& mousePos = engine.GetInput().GetMousePositionf();

	const int cutPreviewColor = 0xFF000000;
	for (int i = 0; i < myCutPositions.Count() - 1; ++i)
		renderer.RenderLine(myCutPositions[i], myCutPositions[i + 1], cutPreviewColor);

	renderer.RenderLine(myCutPositions.GetLast(), mousePos, cutPreviewColor);
}

void LevelEditorDockable::RenderStartAndGoalMarkers() const
{
	Slush::Engine& engine = Slush::Engine::GetInstance();
	Slush::Renderer& renderer = engine.GetWindow().GetRenderer();
	const LevelData& levelData = myLevel.GetLevelDataAsset();

	const float markerRadius = 10.f;
	const int startColor = 0xFF00FF00;
	const int goalColor = 0xFF0000FF;

	renderer.RenderCircle(levelData.myStartPosition, markerRadius, startColor);
	renderer.RenderCircle(levelData.myGoalPosition, markerRadius, goalColor);
}

void LevelEditorDockable::OnUpdate()
{
	UpdateBoxCutMode();
	UpdateManualCutMode();
	UpdateSetStartMode();
	UpdateSetGoalMode();
	UpdatePlaceTowerMode();
}

void LevelEditorDockable::UpdateBoxCutMode()
{
	if (!myIsBoxCutModeActive)
		return;

	Slush::Engine& engine = Slush::Engine::GetInstance();
	if (!engine.GetInput().WasMouseReleased(Slush::Input::LEFTMB))
		return;

	if (!myBoxCutHasStartCorner)
	{
		myBoxCutStartCorner = engine.GetInput().GetMousePositionf();
		myBoxCutHasStartCorner = true;
		return;
	}

	const Vector2f endCorner = engine.GetInput().GetMousePositionf();
	myBoxCutHasStartCorner = false;

	const Vector2f topLeft{ FW_Min(myBoxCutStartCorner.x, endCorner.x), FW_Min(myBoxCutStartCorner.y, endCorner.y) };
	const Vector2f bottomRight{ FW_Max(myBoxCutStartCorner.x, endCorner.x), FW_Max(myBoxCutStartCorner.y, endCorner.y) };

	FW_GrowingArray<Vector2f> boxCorners;
	boxCorners.Add(topLeft);
	boxCorners.Add(Vector2f{ bottomRight.x, topLeft.y });
	boxCorners.Add(bottomRight);
	boxCorners.Add(Vector2f{ topLeft.x, bottomRight.y });

	CutNavmeshHole(boxCorners);
}

void LevelEditorDockable::UpdateManualCutMode()
{
	if (!myIsManualCutModeActive)
		return;

	Slush::Engine& engine = Slush::Engine::GetInstance();
	if (engine.GetInput().WasMouseReleased(Slush::Input::LEFTMB))
	{
		myCutPositions.Add(engine.GetInput().GetMousePositionf());
	}
	else if (engine.GetInput().WasMouseReleased(Slush::Input::RIGHTMB))
	{
		CutNavmeshHole(myCutPositions);
		myCutPositions.RemoveAll();
	}
}

void LevelEditorDockable::UpdateSetStartMode()
{
	if (!myIsSetStartModeActive)
		return;

	Slush::Engine& engine = Slush::Engine::GetInstance();
	if (!engine.GetInput().WasMouseReleased(Slush::Input::LEFTMB))
		return;

	SetStartPosition(engine.GetInput().GetMousePositionf());
}

void LevelEditorDockable::UpdateSetGoalMode()
{
	if (!myIsSetGoalModeActive)
		return;

	Slush::Engine& engine = Slush::Engine::GetInstance();
	if (!engine.GetInput().WasMouseReleased(Slush::Input::LEFTMB))
		return;

	SetGoalPosition(engine.GetInput().GetMousePositionf());
}

void LevelEditorDockable::UpdatePlaceTowerMode()
{
	if (!myIsPlaceTowerModeActive)
		return;

	Slush::Engine& engine = Slush::Engine::GetInstance();
	if (!engine.GetInput().WasMouseReleased(Slush::Input::LEFTMB) || myTowerPrefabToPlace == nullptr)
		return;

	const Vector2f& position = engine.GetInput().GetMousePositionf();
	const Slush::SpriteComponent::Data& spriteData = myTowerPrefabToPlace->GetComponentData<Slush::SpriteComponent>();
	const Vector2f halfSize = spriteData.mySize * 0.5f;

	FW_GrowingArray<Vector2f> footprint;
	footprint.Add(position - halfSize);
	footprint.Add(Vector2f{ position.x + halfSize.x, position.y - halfSize.y });
	footprint.Add(position + halfSize);
	footprint.Add(Vector2f{ position.x - halfSize.x, position.y + halfSize.y });

	Navmesh& navmesh = myLevel.GetNavmesh();
	if (!navmesh.IsAreaFullyOnMesh(footprint))
	{
		SLUSH_WARNING("[Level Editor] Tower placement rejected: footprint is not fully on open navmesh");
		return;
	}

	myLevel.GetEntityManager().CreateEntity(position, *myTowerPrefabToPlace);
	navmesh.CutHole(footprint);
}

void LevelEditorDockable::CutNavmeshHole(const FW_GrowingArray<Vector2f>& someCutPositions)
{
	NavmeshData& navmeshData = myLevel.GetNavmeshDataAsset();

	Slush::AssetEditScope editScope(navmeshData);
	navmeshData.myNavmesh.CutHole(someCutPositions);
	navmeshData.MarkAsUnsaved();

	navmeshData.myNavmesh.GetAsTriangleMesh(myTriangleMesh);
}

void LevelEditorDockable::SetStartPosition(const Vector2f& aPosition)
{
	LevelData& levelData = myLevel.GetLevelDataAsset();

	Slush::AssetEditScope editScope(levelData);
	levelData.myStartPosition = aPosition;
	levelData.MarkAsUnsaved();
}

void LevelEditorDockable::SetGoalPosition(const Vector2f& aPosition)
{
	LevelData& levelData = myLevel.GetLevelDataAsset();

	Slush::AssetEditScope editScope(levelData);
	levelData.myGoalPosition = aPosition;
	levelData.MarkAsUnsaved();
}

void LevelEditorDockable::DisableBoxCutMode()
{
	if (!myIsBoxCutModeActive)
		return;

	myIsBoxCutModeActive = false;
	myBoxCutHasStartCorner = false;
}

void LevelEditorDockable::DisableManualCutMode()
{
	if (!myIsManualCutModeActive)
		return;

	myIsManualCutModeActive = false;
	myCutPositions.RemoveAll();
}

void LevelEditorDockable::DisableSetStartMode()
{
	myIsSetStartModeActive = false;
}

void LevelEditorDockable::DisableSetGoalMode()
{
	myIsSetGoalModeActive = false;
}

void LevelEditorDockable::DisablePlaceTowerMode()
{
	myIsPlaceTowerModeActive = false;
}

void LevelEditorDockable::OnBuildUI()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::MenuItem("Save"))
			SaveAssets();

		ImGui::EndMenuBar();
	}

	if (mySaveBlockedTowerCount > 0)
	{
		ImGui::Text("Save blocked: %d tower(s) currently placed - remove them or restart before saving", mySaveBlockedTowerCount);
		ImGui::Separator();
	}

	ImGui::Text("Enemies");

	if (ImGui::Button("Spawn Normal"))
		myLevel.SpawnEnemyNormal();

	if (ImGui::Button("Spawn Fast"))
		myLevel.SpawnEnemyFast();

	if (ImGui::Button("Spawn Slow"))
		myLevel.SpawnEnemySlow();

	if (ImGui::Button("Damage All Enemies"))
		myLevel.DamageAllEnemies();

	ImGui::Separator();

	ImGui::Text("Navmesh");
	if (myIsBoxCutModeActive)
	{
		if (ImGui::Button("Disable Box Cut"))
			DisableBoxCutMode();
	}
	else
	{
		if (ImGui::Button("Enable Box Cut"))
		{
			myIsBoxCutModeActive = true;
			myBoxCutHasStartCorner = false;

			DisableManualCutMode();
			DisableSetStartMode();
			DisableSetGoalMode();
		}
	}

	if (myIsManualCutModeActive)
	{
		if (ImGui::Button("Disable Manual Cut"))
			DisableManualCutMode();
	}
	else
	{
		if (ImGui::Button("Enable Manual Cut"))
		{
			myIsManualCutModeActive = true;
			myCutPositions.RemoveAll();

			DisableBoxCutMode();
			DisableSetStartMode();
			DisableSetGoalMode();
		}
	}

	if (ImGui::Button("Clear Navmesh"))
	{
		myLevel.GetNavmesh().GenerateDefaultGrid();
		myLevel.GetNavmesh().GetAsTriangleMesh(myTriangleMesh);
	}

	ImGui::Separator();
	ImGui::Text("Start/Goal");
	if (myIsSetStartModeActive)
	{
		if (ImGui::Button("Disable Set Start"))
			DisableSetStartMode();
	}
	else
	{
		if (ImGui::Button("Enable Set Start"))
		{
			myIsSetStartModeActive = true;

			DisableBoxCutMode();
			DisableManualCutMode();
			DisableSetGoalMode();
		}
	}

	if (myIsSetGoalModeActive)
	{
		if (ImGui::Button("Disable Set Goal"))
			DisableSetGoalMode();
	}
	else
	{
		if (ImGui::Button("Enable Set Goal"))
		{
			myIsSetGoalModeActive = true;

			DisableBoxCutMode();
			DisableManualCutMode();
			DisableSetStartMode();
		}
	}

	ImGui::Separator();

	ImGui::Text("Towers");
	Slush::AssetRegistry& assetRegistry = Slush::AssetRegistry::GetInstance();
	const FW_GrowingArray<Slush::Asset*>& prefabs = assetRegistry.GetAllAssets<Slush::EntityPrefab>();
	const char* selectedPrefabName = myTowerPrefabToPlace ? myTowerPrefabToPlace->GetAssetName().GetBuffer() : "Select a tower";
	if (ImGui::BeginCombo("Tower Prefab", selectedPrefabName))
	{
		for (Slush::Asset* asset : prefabs)
		{
			Slush::EntityPrefab* prefab = static_cast<Slush::EntityPrefab*>(asset);
			if (!prefab->Has<TowerCombatComponent>() || !prefab->Has<Slush::SpriteComponent>())
				continue;

			const bool isSelected = prefab == myTowerPrefabToPlace;
			if (ImGui::Selectable(prefab->GetAssetName().GetBuffer(), isSelected))
				myTowerPrefabToPlace = prefab;
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	if (myIsPlaceTowerModeActive)
	{
		if (ImGui::Button("Disable Place Tower"))
			DisablePlaceTowerMode();
	}
	else if (ImGui::Button("Enable Place Tower"))
	{
		if (myTowerPrefabToPlace == nullptr)
		{
			SLUSH_WARNING("[Level Editor] Select a tower prefab before enabling tower placement");
		}
		else
		{
			myIsPlaceTowerModeActive = true;
			DisableBoxCutMode();
			DisableManualCutMode();
			DisableSetStartMode();
			DisableSetGoalMode();
		}
	}
}

bool LevelEditorDockable::HasUnsavedChanges() const
{
	return myLevel.GetNavmeshDataAsset().HasUnsavedChanges() || myLevel.GetLevelDataAsset().HasUnsavedChanges();
}

void LevelEditorDockable::OnCloseRequested()
{
	// Under -hidewindow, no user is around to click a popup - log which asset(s) are being discarded, by
	// name, and resolve immediately instead of opening one that would otherwise hang the close forever.
	if (Slush::CommandLineArgs::GetInstance().HasFlag("-hidewindow"))
	{
		if (myLevel.GetNavmeshDataAsset().HasUnsavedChanges())
			SLUSH_ERROR("[Level Editor] Closing with unsaved changes in '%s', discarding them (-hidewindow)", myLevel.GetNavmeshDataAsset().GetAssetName().GetBuffer());

		if (myLevel.GetLevelDataAsset().HasUnsavedChanges())
			SLUSH_ERROR("[Level Editor] Closing with unsaved changes in '%s', discarding them (-hidewindow)", myLevel.GetLevelDataAsset().GetAssetName().GetBuffer());

		DiscardUnsavedChanges();
		return;
	}

	myWantToOpenUnsavedChangesPopup = true;
}

void LevelEditorDockable::OnBuildModals()
{
	if (myWantToOpenUnsavedChangesPopup)
	{
		ImGui::OpenPopup("Unsaved Changes");
		myWantToOpenUnsavedChangesPopup = false;
	}

	if (ImGui::BeginPopupModal("Unsaved Changes", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("The following assets have unsaved changes:");
		if (myLevel.GetNavmeshDataAsset().HasUnsavedChanges())
			ImGui::BulletText("%s", myLevel.GetNavmeshDataAsset().GetAssetName().GetBuffer());
		if (myLevel.GetLevelDataAsset().HasUnsavedChanges())
			ImGui::BulletText("%s", myLevel.GetLevelDataAsset().GetAssetName().GetBuffer());
		if (mySaveBlockedTowerCount > 0)
			ImGui::Text("Save blocked: %d tower(s) currently placed - remove them or restart before saving", mySaveBlockedTowerCount);

		if (ImGui::Button("Save"))
		{
			if (SaveAssets())
				ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Discard"))
		{
			DiscardUnsavedChanges();
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel"))
		{
			if (myOwnerLayout)
				myOwnerLayout->CancelCloseRequest();

			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

bool LevelEditorDockable::SaveAssets()
{
	mySaveBlockedTowerCount = GetPlacedTowerCount();
	if (mySaveBlockedTowerCount > 0)
	{
		SLUSH_WARNING("[Level Editor] Save blocked: %d tower(s) currently placed - remove them or restart before saving", mySaveBlockedTowerCount);
		return false;
	}

	myLevel.GetNavmeshDataAsset().Save();
	myLevel.GetLevelDataAsset().Save();
	return true;
}

int LevelEditorDockable::GetPlacedTowerCount() const
{
	FW_GrowingArray<Slush::EntityHandle> entities;
	myLevel.GetEntityManager().GetAllEntities(entities);

	int towerCount = 0;
	for (const Slush::EntityHandle& handle : entities)
	{
		Slush::Entity* entity = handle.Get();
		if (entity && !entity->myIsMarkedForRemoval && entity->GetComponent<TowerCombatComponent>())
			++towerCount;
	}

	return towerCount;
}

void LevelEditorDockable::DiscardUnsavedChanges()
{
	myLevel.GetNavmeshDataAsset().MarkAsSaved();
	myLevel.GetLevelDataAsset().MarkAsSaved();
}
