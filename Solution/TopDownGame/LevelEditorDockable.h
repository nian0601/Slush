#pragma once

#include <Core/Dockables/Dockable.h>
#include "Navmesh.h"

class Level;
class LevelData;
class LevelEditorLayout;

namespace Slush
{
	class EntityPrefab;
}

class LevelEditorDockable : public Slush::DockableBase<LevelEditorDockable>
{
public:
	LevelEditorDockable(LevelEditorLayout& aLayout, Level& aLevel);

	// Points the editor at a freshly built Level (the layout owns it), resetting the active tool.
	void SetLevel(Level& aLevel);

	const char* GetName() const override { return "Level Editor"; }

	void RenderOverlay() const;

	// Null when no tool is active.
	const char* GetToolHint() const;

protected:
	void OnUpdate() override;
	void OnBuildUI() override;

	bool HasUnsavedChanges() const override;
	void OnCloseRequested() override;
	void OnBuildModals() override;

private:
	// What to do once unsaved changes are resolved (Save or Discard) - anything that leaves the current level
	// goes through the Unsaved Changes prompt first.
	enum class PendingAction
	{
		None,
		Close,
		OpenLevel,
		NewLevel,
		DuplicateLevel,
	};

	// One active tool at a time - picking a tool replaces the current one.
	enum class ToolMode
	{
		None,
		BoxCut,
		ManualCut,
		SetStart,
		SetGoal,
	};

	void SetToolMode(ToolMode aToolMode);

	void UpdateBoxCutMode();
	void UpdateManualCutMode();
	void UpdateSetStartMode();
	void UpdateSetGoalMode();

	void RenderBoxCutPreview() const;
	void RenderManualCutPreview() const;
	void RenderStartAndGoalMarkers() const;
	void RenderToolHint() const;

	void BuildHeaderUI();
	void BuildToolButton(const char* aLabel, ToolMode aToolMode);
	void BuildLevelTab();
	void BuildNavmeshTab();
	void BuildWavesTab();
	void BuildDebugTab();

	// Shared by both cut modes - wraps the CutHole() call in an AssetEditScope and marks the NavmeshData
	// asset dirty, since CutHole() itself is a direct field write, not one of the ImGuiWidgets:: calls
	// that would otherwise mark it dirty automatically.
	void CutNavmeshHole(const FW_GrowingArray<Vector2f>& someCutPositions);
	void ResetNavmeshToDefaultGrid();

	// Same idea for the single-point spawn/goal writes, against the LevelData asset instead.
	void SetStartPosition(const Vector2f& aPosition);
	void SetGoalPosition(const Vector2f& aPosition);

	void RequestAction(PendingAction anAction, LevelData* aLevelToOpen = nullptr);
	void RunPendingAction();

	void BuildUnsavedChangesModal();
	void BuildNameModal();
	void BuildOpenLevelModal();
	void BuildOpenLevelDropTarget();

	void SaveAssets();
	// Close path: the app is going away, so the edits only need to stop counting as unsaved.
	void DiscardUnsavedChanges();
	// Level-switch path: reloads the edited assets from disk and rebuilds the Level.
	void RevertUnsavedChanges();

	LevelEditorLayout& myLayout;
	Level* myLevel = nullptr;

	ToolMode myToolMode = ToolMode::None;

	bool myBoxCutHasStartCorner = false;
	Vector2f myBoxCutStartCorner;

	FW_GrowingArray<Vector2f> myCutPositions;

	bool myWantToOpenUnsavedChangesPopup = false;
	PendingAction myPendingAction = PendingAction::None;
	LevelData* myPendingLevelToOpen = nullptr;

	bool myWantToOpenNameDialog = false;
	bool myNameDialogIsDuplicate = false;
	FW_String myNewLevelName;

	bool myWantToOpenOpenLevelDialog = false;
	int mySelectedUnlockIndex = 0;

	Navmesh::TriangleMesh myTriangleMesh;
};
