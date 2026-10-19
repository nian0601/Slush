#pragma once

#include <Core/Dockables/Dockable.h>
#include "Navmesh.h"

class Level;

namespace Slush
{
	class EntityPrefab;
}

class LevelEditorDockable : public Slush::DockableBase<LevelEditorDockable>
{
public:
	LevelEditorDockable(Level& aLevel);

	const char* GetName() const override { return "Level Editor"; }

	void RenderOverlay() const;

protected:
	void OnUpdate() override;
	void OnBuildUI() override;

	bool HasUnsavedChanges() const override;
	void OnCloseRequested() override;
	void OnBuildModals() override;

private:
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
	const char* GetToolHint() const;

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

	void SaveAssets();
	void DiscardUnsavedChanges();

	Level& myLevel;

	ToolMode myToolMode = ToolMode::None;

	bool myBoxCutHasStartCorner = false;
	Vector2f myBoxCutStartCorner;

	FW_GrowingArray<Vector2f> myCutPositions;

	bool myWantToOpenUnsavedChangesPopup = false;
	int mySelectedUnlockIndex = 0;

	Navmesh::TriangleMesh myTriangleMesh;
};
