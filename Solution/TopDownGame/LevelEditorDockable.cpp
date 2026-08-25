#include "stdafx.h"

#include "LevelEditorDockable.h"

#include "Level/Level.h"
#include "Level/LevelData.h"
#include "Level/NavmeshData.h"

#include "Core/CommandLineArgs.h"
#include "Core/Dockables/IAppLayout.h"
#include "Core/Engine.h"
#include "Core/Input.h"
#include "Graphics/Renderer.h"
#include "Graphics/Window.h"
#include <imgui/ImGuiWidgets.h>

LevelEditorDockable::LevelEditorDockable(Level& aLevel)
	: Slush::DockableBase<LevelEditorDockable>(true)
	, myLevel(aLevel)
{
}

void LevelEditorDockable::RenderOverlay() const
{
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

void LevelEditorDockable::CutNavmeshHole(const FW_GrowingArray<Vector2f>& someCutPositions)
{
	NavmeshData& navmeshData = myLevel.GetNavmeshDataAsset();

	Slush::AssetEditScope editScope(navmeshData);
	navmeshData.myNavmesh.CutHole(someCutPositions);
	navmeshData.MarkAsUnsaved();
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

void LevelEditorDockable::OnBuildUI()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::MenuItem("Save"))
			SaveAssets();

		ImGui::EndMenuBar();
	}

	ImGui::Text("Level Editor");

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

		if (ImGui::Button("Save"))
		{
			SaveAssets();
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

void LevelEditorDockable::SaveAssets()
{
	myLevel.GetNavmeshDataAsset().Save();
	myLevel.GetLevelDataAsset().Save();
}

void LevelEditorDockable::DiscardUnsavedChanges()
{
	myLevel.GetNavmeshDataAsset().MarkAsSaved();
	myLevel.GetLevelDataAsset().MarkAsSaved();
}
