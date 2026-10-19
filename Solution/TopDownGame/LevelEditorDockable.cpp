#include "stdafx.h"

#include "LevelEditorDockable.h"

#include "Level/Level.h"
#include "Level/LevelData.h"
#include "Level/NavmeshData.h"
#include "LevelEditorLayout.h"
#include "LevelEditorUtils.h"
#include "TopDownGameGlobals.h"

#include "Core/Assets/AssetStorage.h"
#include "Core/Assets/DependencyTracker.h"
#include "Core/CommandLineArgs.h"
#include "Core/Dockables/IAppLayout.h"
#include "Core/Engine.h"
#include "Core/Input.h"
#include "Graphics/Renderer.h"
#include "Graphics/Window.h"
#include <EntitySystem/EntityPrefab.h>
#include <imgui/ImGuiWidgets.h>

LevelEditorDockable::LevelEditorDockable(LevelEditorLayout& aLayout, Level& aLevel)
	: Slush::DockableBase<LevelEditorDockable>(true)
	, myLayout(aLayout)
	, myLevel(&aLevel)
{
	myLevel->GetNavmesh().GetAsTriangleMesh(myTriangleMesh);
}

void LevelEditorDockable::SetLevel(Level& aLevel)
{
	myLevel = &aLevel;
	SetToolMode(ToolMode::None);
	mySelectedUnlockIndex = 0;
	myLevel->GetNavmesh().GetAsTriangleMesh(myTriangleMesh);
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

	// With the editor UI visible the hint is drawn crisp as an ImGui overlay on the Game View instead
	if (!Slush::Engine::GetInstance().GetWindow().IsEditorUIVisible())
		RenderToolHint();
}

void LevelEditorDockable::RenderBoxCutPreview() const
{
	if (myToolMode != ToolMode::BoxCut || !myBoxCutHasStartCorner)
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
	if (myToolMode != ToolMode::ManualCut || myCutPositions.IsEmpty())
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
	const LevelData& levelData = myLevel->GetLevelDataAsset();

	const float markerRadius = 10.f;
	const int startColor = 0xFF00FF00;
	const int goalColor = 0xFF0000FF;

	renderer.RenderCircle(levelData.myStartPosition, markerRadius, startColor);
	renderer.RenderCircle(levelData.myGoalPosition, markerRadius, goalColor);
}

void LevelEditorDockable::RenderToolHint() const
{
	const char* hint = GetToolHint();
	if (!hint)
		return;

	const Slush::UITheme& theme = TopDownGameGlobals::GetInstance().GetUITheme();
	Slush::Renderer& renderer = Slush::Engine::GetInstance().GetWindow().GetRenderer();
	renderer.RenderText(*theme.myFont, hint, Vector2f(20.f, 1030.f),
		theme.myTextSizes[static_cast<int>(Slush::UITextRole::Body)],
		theme.myTextColors[static_cast<int>(Slush::UITextTone::Default)]);
}

const char* LevelEditorDockable::GetToolHint() const
{
	switch (myToolMode)
	{
	case ToolMode::BoxCut: return "Box Cut: LMB first corner, LMB second corner. Esc cancels.";
	case ToolMode::ManualCut: return "Manual Cut: LMB adds a point, RMB cuts the polygon. Esc cancels.";
	case ToolMode::SetStart: return "Set Start: LMB places the start position. Esc cancels.";
	case ToolMode::SetGoal: return "Set Goal: LMB places the goal position. Esc cancels.";
	default: return nullptr;
	}
}

void LevelEditorDockable::SetToolMode(ToolMode aToolMode)
{
	myToolMode = aToolMode;

	// Any half-made box or polygon belongs to the tool being left
	myBoxCutHasStartCorner = false;
	myCutPositions.RemoveAll();
}

void LevelEditorDockable::OnUpdate()
{
	Slush::Engine& engine = Slush::Engine::GetInstance();
	if (engine.GetInput().WasKeyReleased(Slush::Input::ESC) || myLayout.IsPlaying())
		SetToolMode(ToolMode::None);

	switch (myToolMode)
	{
	case ToolMode::BoxCut: UpdateBoxCutMode(); break;
	case ToolMode::ManualCut: UpdateManualCutMode(); break;
	case ToolMode::SetStart: UpdateSetStartMode(); break;
	case ToolMode::SetGoal: UpdateSetGoalMode(); break;
	default: break;
	}
}

void LevelEditorDockable::UpdateBoxCutMode()
{
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
	Slush::Engine& engine = Slush::Engine::GetInstance();
	if (!engine.GetInput().WasMouseReleased(Slush::Input::LEFTMB))
		return;

	SetStartPosition(engine.GetInput().GetMousePositionf());
}

void LevelEditorDockable::UpdateSetGoalMode()
{
	Slush::Engine& engine = Slush::Engine::GetInstance();
	if (!engine.GetInput().WasMouseReleased(Slush::Input::LEFTMB))
		return;

	SetGoalPosition(engine.GetInput().GetMousePositionf());
}

void LevelEditorDockable::CutNavmeshHole(const FW_GrowingArray<Vector2f>& someCutPositions)
{
	NavmeshData& navmeshData = myLevel->GetNavmeshDataAsset();

	Slush::AssetEditScope editScope(navmeshData);
	navmeshData.myNavmesh.CutHole(someCutPositions);
	navmeshData.MarkAsUnsaved();

	navmeshData.myNavmesh.GetAsTriangleMesh(myTriangleMesh);
}

void LevelEditorDockable::ResetNavmeshToDefaultGrid()
{
	NavmeshData& navmeshData = myLevel->GetNavmeshDataAsset();

	Slush::AssetEditScope editScope(navmeshData);
	navmeshData.myNavmesh.GenerateDefaultGrid();
	navmeshData.MarkAsUnsaved();

	navmeshData.myNavmesh.GetAsTriangleMesh(myTriangleMesh);
}

void LevelEditorDockable::SetStartPosition(const Vector2f& aPosition)
{
	LevelData& levelData = myLevel->GetLevelDataAsset();

	Slush::AssetEditScope editScope(levelData);
	levelData.myStartPosition = aPosition;
	levelData.MarkAsUnsaved();
}

void LevelEditorDockable::SetGoalPosition(const Vector2f& aPosition)
{
	LevelData& levelData = myLevel->GetLevelDataAsset();

	Slush::AssetEditScope editScope(levelData);
	levelData.myGoalPosition = aPosition;
	levelData.MarkAsUnsaved();
}

void LevelEditorDockable::OnBuildUI()
{
	BuildHeaderUI();

	// Edits mid-play would desync from what's running, so everything below the Play button is read-only
	ImGui::BeginDisabled(myLayout.IsPlaying());
	if (ImGui::BeginTabBar("LevelEditorTabs"))
	{
		// Only the tab content scrolls - the header above stays put.
		if (ImGui::BeginTabItem("Level"))
		{
			if (ImGui::BeginChild("LevelTab"))
				BuildLevelTab();
			ImGui::EndChild();
			BuildOpenLevelDropTarget();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Navmesh"))
		{
			if (ImGui::BeginChild("NavmeshTab"))
				BuildNavmeshTab();
			ImGui::EndChild();
			BuildOpenLevelDropTarget();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Waves"))
		{
			if (ImGui::BeginChild("WavesTab"))
				BuildWavesTab();
			ImGui::EndChild();
			BuildOpenLevelDropTarget();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Debug"))
		{
			if (ImGui::BeginChild("DebugTab"))
				BuildDebugTab();
			ImGui::EndChild();
			BuildOpenLevelDropTarget();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
	ImGui::EndDisabled();
}

void LevelEditorDockable::BuildHeaderUI()
{
	const bool isPlaying = myLayout.IsPlaying();
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File", !isPlaying))
		{
			if (ImGui::MenuItem("New Level..."))
				RequestAction(PendingAction::NewLevel);
			if (ImGui::MenuItem("Duplicate Level..."))
				RequestAction(PendingAction::DuplicateLevel);
			if (ImGui::MenuItem("Open Level..."))
				myWantToOpenOpenLevelDialog = true;

			ImGui::Separator();
			if (ImGui::MenuItem("Save", "Ctrl+S"))
				SaveAssets();

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	if (isPlaying)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.65f, 0.15f, 0.15f, 1.f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 1.f));
		if (ImGui::Button(ICON_FA_STOP " Stop"))
			myLayout.StopPlay();
		ImGui::PopStyleColor(2);
	}
	else
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.5f, 0.2f, 1.f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.65f, 0.25f, 1.f));
		if (ImGui::Button(ICON_FA_PLAY " Play"))
			myLayout.StartPlay();
		ImGui::PopStyleColor(2);
	}
	ImGui::SameLine();

	ImGui::BeginDisabled(isPlaying);
	if (!isPlaying && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_S, ImGuiInputFlags_RouteGlobal))
		SaveAssets();

	if (ImGui::Button("Save"))
		SaveAssets();

	ImGui::SameLine();
	const bool hasUnsavedChanges = HasUnsavedChanges();
	ImGui::Text("%s%s", myLevel->GetLevelDataAsset().GetAssetName().GetBuffer(), hasUnsavedChanges ? " *" : "");
	ImGui::SameLine();
	if (hasUnsavedChanges)
		ImGui::TextColored(ImGui::GetStyleColorVec4(ImGuiCol_PlotHistogram), "Unsaved changes");
	else
		ImGui::TextDisabled("Saved");

	ImGui::AlignTextToFramePadding();
	ImGui::TextDisabled("Tool");
	ImGui::SameLine();
	BuildToolButton("None", ToolMode::None);
	ImGui::SameLine();
	BuildToolButton("Box Cut", ToolMode::BoxCut);
	ImGui::SameLine();
	BuildToolButton("Manual Cut", ToolMode::ManualCut);
	ImGui::SameLine();
	BuildToolButton("Set Start", ToolMode::SetStart);
	ImGui::SameLine();
	BuildToolButton("Set Goal", ToolMode::SetGoal);
	ImGui::EndDisabled();

	ImGui::Separator();
}

void LevelEditorDockable::BuildToolButton(const char* aLabel, ToolMode aToolMode)
{
	const bool isActive = myToolMode == aToolMode;
	if (isActive)
		ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));

	if (ImGui::Button(aLabel) && !isActive)
		SetToolMode(aToolMode);

	if (isActive)
		ImGui::PopStyleColor();
}

void LevelEditorDockable::BuildLevelTab()
{
	LevelData& levelData = myLevel->GetLevelDataAsset();
	ImGui::Text("Level: %s", levelData.GetAssetName().GetBuffer());

	Slush::AssetEditScope editScope(levelData);
	levelData.BuildLevelSettingsUI();

	ImGui::Spacing();
	ImGui::TextDisabled("Set Start / Set Goal click-place in the view.");
}

void LevelEditorDockable::BuildNavmeshTab()
{
	NavmeshData& navmeshData = myLevel->GetNavmeshDataAsset();
	ImGui::Text("Navmesh: %s", navmeshData.GetAssetName().GetBuffer());
	ImGui::Text("Triangles: %d", myTriangleMesh.myVertices.Count() / 3);

	ImGui::Separator();
	if (ImGui::Button("Box Cut"))
		SetToolMode(ToolMode::BoxCut);
	ImGui::SameLine();
	if (ImGui::Button("Manual Cut"))
		SetToolMode(ToolMode::ManualCut);
	ImGui::SameLine();
	if (ImGui::Button("Reset to Default Grid"))
		ResetNavmeshToDefaultGrid();

	ImGui::Spacing();
	ImGui::TextDisabled("Box Cut: LMB first corner, LMB second corner.");
	ImGui::TextDisabled("Manual Cut: LMB adds points, RMB cuts the polygon.");
}

void LevelEditorDockable::BuildWavesTab()
{
	LevelData& levelData = myLevel->GetLevelDataAsset();
	Slush::AssetEditScope editScope(levelData);
	levelData.BuildWaveScalingUI();
}

void LevelEditorDockable::BuildDebugTab()
{
	ImGui::TextWrapped("Waves don't run in edit mode. Use Play to test.");
	ImGui::Separator();

	// Picks from the level's own unlock roster, so each level's Debug spawns follow what its waves can send.
	const FW_GrowingArray<EnemyUnlockEntry>& unlocks = myLevel->GetLevelDataAsset().myEnemyUnlocks;
	Slush::EntityPrefab* selectedPrefab = nullptr;
	if (mySelectedUnlockIndex >= 0 && mySelectedUnlockIndex < unlocks.Count())
		selectedPrefab = unlocks[mySelectedUnlockIndex].myEnemyPrefab.Get();

	const char* selectedName = selectedPrefab ? selectedPrefab->GetAssetName().GetBuffer() : "Select an enemy";
	if (ImGui::BeginCombo("Enemy", selectedName))
	{
		for (int i = 0; i < unlocks.Count(); ++i)
		{
			Slush::EntityPrefab* prefab = unlocks[i].myEnemyPrefab.Get();
			if (!prefab)
				continue;

			ImGui::PushID(i);
			const bool isSelected = i == mySelectedUnlockIndex;
			if (ImGui::Selectable(prefab->GetAssetName().GetBuffer(), isSelected))
				mySelectedUnlockIndex = i;
			if (isSelected)
				ImGui::SetItemDefaultFocus();
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}

	ImGui::BeginDisabled(selectedPrefab == nullptr);
	if (ImGui::Button("Spawn"))
		myLevel->SpawnEnemy(*selectedPrefab);
	ImGui::EndDisabled();

	ImGui::SameLine();
	if (ImGui::Button("Damage All"))
		myLevel->DamageAllEnemies();

	ImGui::SameLine();
	if (ImGui::Button("Clear Enemies"))
		myLevel->ClearEnemies();
}

bool LevelEditorDockable::HasUnsavedChanges() const
{
	return myLevel->GetNavmeshDataAsset().HasUnsavedChanges() || myLevel->GetLevelDataAsset().HasUnsavedChanges();
}

void LevelEditorDockable::OnCloseRequested()
{
	// Under -hidewindow, no user is around to click a popup - log which asset(s) are being discarded, by
	// name, and resolve immediately instead of opening one that would otherwise hang the close forever.
	if (Slush::CommandLineArgs::GetInstance().HasFlag("-hidewindow"))
	{
		if (myLevel->GetNavmeshDataAsset().HasUnsavedChanges())
			SLUSH_ERROR("[Level Editor] Closing with unsaved changes in '%s', discarding them (-hidewindow)", myLevel->GetNavmeshDataAsset().GetAssetName().GetBuffer());

		if (myLevel->GetLevelDataAsset().HasUnsavedChanges())
			SLUSH_ERROR("[Level Editor] Closing with unsaved changes in '%s', discarding them (-hidewindow)", myLevel->GetLevelDataAsset().GetAssetName().GetBuffer());

		DiscardUnsavedChanges();
		return;
	}

	myPendingAction = PendingAction::Close;
	myWantToOpenUnsavedChangesPopup = true;
}

void LevelEditorDockable::RequestAction(PendingAction anAction, LevelData* aLevelToOpen)
{
	if (myLayout.IsPlaying())
		return;

	if (anAction == PendingAction::OpenLevel && aLevelToOpen == &myLevel->GetLevelDataAsset())
		return;

	myPendingAction = anAction;
	myPendingLevelToOpen = aLevelToOpen;

	if (HasUnsavedChanges())
	{
		myWantToOpenUnsavedChangesPopup = true;
		return;
	}

	RunPendingAction();
}

void LevelEditorDockable::RunPendingAction()
{
	const PendingAction action = myPendingAction;
	LevelData* levelToOpen = myPendingLevelToOpen;
	myPendingAction = PendingAction::None;
	myPendingLevelToOpen = nullptr;

	switch (action)
	{
	case PendingAction::OpenLevel:
		myLayout.OpenLevel(*levelToOpen);
		break;
	case PendingAction::NewLevel:
		myNameDialogIsDuplicate = false;
		myNewLevelName.Clear();
		myWantToOpenNameDialog = true;
		break;
	case PendingAction::DuplicateLevel:
		myNameDialogIsDuplicate = true;
		myNewLevelName = myLevel->GetLevelDataAsset().GetAssetName();
		myNewLevelName += "_copy";
		myWantToOpenNameDialog = true;
		break;
	default:
		break;
	}
}

void LevelEditorDockable::OnBuildModals()
{
	BuildUnsavedChangesModal();
	BuildNameModal();
	BuildOpenLevelModal();
}

void LevelEditorDockable::BuildUnsavedChangesModal()
{
	if (myWantToOpenUnsavedChangesPopup)
	{
		ImGui::OpenPopup("Unsaved Changes");
		myWantToOpenUnsavedChangesPopup = false;
	}

	if (ImGui::BeginPopupModal("Unsaved Changes", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("The following assets have unsaved changes:");
		if (myLevel->GetNavmeshDataAsset().HasUnsavedChanges())
			ImGui::BulletText("%s", myLevel->GetNavmeshDataAsset().GetAssetName().GetBuffer());
		if (myLevel->GetLevelDataAsset().HasUnsavedChanges())
			ImGui::BulletText("%s", myLevel->GetLevelDataAsset().GetAssetName().GetBuffer());

		if (ImGui::Button("Save"))
		{
			SaveAssets();
			ImGui::CloseCurrentPopup();
			RunPendingAction();
		}

		ImGui::SameLine();

		if (ImGui::Button("Discard"))
		{
			if (myPendingAction == PendingAction::Close)
				DiscardUnsavedChanges();
			else
				RevertUnsavedChanges();

			ImGui::CloseCurrentPopup();
			RunPendingAction();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel"))
		{
			if (myPendingAction == PendingAction::Close && myOwnerLayout)
				myOwnerLayout->CancelCloseRequest();

			myPendingAction = PendingAction::None;
			myPendingLevelToOpen = nullptr;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void LevelEditorDockable::BuildNameModal()
{
	if (myWantToOpenNameDialog)
	{
		ImGui::OpenPopup("Level Name");
		myWantToOpenNameDialog = false;
	}

	if (ImGui::BeginPopupModal("Level Name", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (myNameDialogIsDuplicate)
			ImGui::Text("Duplicate '%s'", myLevel->GetLevelDataAsset().GetAssetName().GetBuffer());
		else
			ImGui::Text("New blank level");

		if (ImGui::IsWindowAppearing())
			ImGui::SetKeyboardFocusHere();
		ImGui::InputText("Name", &myNewLevelName);

		Slush::AssetRegistry& assetRegistry = Slush::AssetRegistry::GetInstance();
		FW_GrowingArray<FW_String> levelNames;
		for (const Slush::Asset* asset : assetRegistry.GetAllAssets<LevelData>())
			levelNames.Add(asset->GetAssetName());
		FW_GrowingArray<FW_String> navmeshNames;
		for (const Slush::Asset* asset : assetRegistry.GetAllAssets<NavmeshData>())
			navmeshNames.Add(asset->GetAssetName());

		FW_String navmeshName;
		LevelEditorUtils::GetNavmeshNameForLevel(myNewLevelName, navmeshName);
		ImGui::TextDisabled("Creates %s.tdlevel and %s.navmesh", myNewLevelName.GetBuffer(), navmeshName.GetBuffer());

		const LevelEditorUtils::NameError nameError = LevelEditorUtils::ValidateNewLevelName(myNewLevelName, levelNames, navmeshNames);
		if (nameError != LevelEditorUtils::NameError::None)
			ImGui::TextColored(ImGui::GetStyleColorVec4(ImGuiCol_PlotHistogram), "%s", LevelEditorUtils::GetNameErrorMessage(nameError));

		ImGui::BeginDisabled(nameError != LevelEditorUtils::NameError::None);
		if (ImGui::Button("Create"))
		{
			LevelData& levelData = myNameDialogIsDuplicate
				? myLayout.DuplicateLevel(myNewLevelName, myLevel->GetLevelDataAsset())
				: myLayout.CreateNewLevel(myNewLevelName);

			ImGui::CloseCurrentPopup();
			myLayout.OpenLevel(levelData);
		}
		ImGui::EndDisabled();

		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
}

void LevelEditorDockable::BuildOpenLevelModal()
{
	if (myWantToOpenOpenLevelDialog)
	{
		ImGui::OpenPopup("Open Level");
		myWantToOpenOpenLevelDialog = false;
	}

	if (ImGui::BeginPopupModal("Open Level", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		const LevelData* currentLevel = &myLevel->GetLevelDataAsset();
		LevelData* levelToOpen = nullptr;
		for (Slush::Asset* asset : Slush::AssetRegistry::GetInstance().GetAllAssets<LevelData>())
		{
			LevelData* levelData = static_cast<LevelData*>(asset);
			FW_String label = levelData->GetAssetName();
			if (levelData == currentLevel)
				label += "  (current)";

			if (ImGui::Selectable(label.GetBuffer(), levelData == currentLevel))
				levelToOpen = levelData;
		}

		ImGui::Separator();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();

		if (levelToOpen)
		{
			ImGui::CloseCurrentPopup();
			RequestAction(PendingAction::OpenLevel, levelToOpen);
		}

		ImGui::EndPopup();
	}
}

void LevelEditorDockable::BuildOpenLevelDropTarget()
{
	// Acts on the last item - the tab content child - so dropping a level anywhere in it opens it
	if (ImGui::BeginDragDropTarget())
	{
		if (Slush::Asset* asset = ImGui::AcceptDraggedAsset(Slush::GetAssetID<LevelData>()))
			RequestAction(PendingAction::OpenLevel, static_cast<LevelData*>(asset));

		ImGui::EndDragDropTarget();
	}
}

void LevelEditorDockable::SaveAssets()
{
	myLevel->GetNavmeshDataAsset().Save();
	myLevel->GetLevelDataAsset().Save();
}

void LevelEditorDockable::DiscardUnsavedChanges()
{
	myLevel->GetNavmeshDataAsset().MarkAsSaved();
	myLevel->GetLevelDataAsset().MarkAsSaved();
}

void LevelEditorDockable::RevertUnsavedChanges()
{
	// Leaving the level for another one, so the discarded edits must not stay in memory - the game's level
	// select and Play read the same LevelData. Reload both assets from disk, then rebuild the Level, since
	// the old one's entities may hold pointers into the navmesh that was just reloaded.
	LevelData& levelData = myLevel->GetLevelDataAsset();
	NavmeshData& navmeshData = myLevel->GetNavmeshDataAsset();

	if (navmeshData.HasUnsavedChanges())
		navmeshData.Load(navmeshData.GetFilePath().GetBuffer(), navmeshData.GetRepositoryIndex());

	if (levelData.HasUnsavedChanges())
	{
		levelData.Load(levelData.GetFilePath().GetBuffer(), levelData.GetRepositoryIndex());

		Slush::DependencyTracker& dependencyTracker = Slush::DependencyTracker::GetInstance();
		dependencyTracker.BeginTrackingAsset(&levelData);
		levelData.ResolveDependencies();
		dependencyTracker.EndTrackingAsset();
	}

	myLayout.OpenLevel(levelData);
}
