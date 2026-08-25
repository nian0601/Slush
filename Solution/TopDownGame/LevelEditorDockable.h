#pragma once

#include <Core/Dockables/Dockable.h>

class Level;

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
	void UpdateBoxCutMode();
	void UpdateManualCutMode();
	void UpdateSetStartMode();
	void UpdateSetGoalMode();

	void RenderBoxCutPreview() const;
	void RenderManualCutPreview() const;
	void RenderStartAndGoalMarkers() const;

	void DisableBoxCutMode();
	void DisableManualCutMode();
	void DisableSetStartMode();
	void DisableSetGoalMode();

	// Shared by both cut modes - wraps the CutHole() call in an AssetEditScope and marks the NavmeshData
	// asset dirty, since CutHole() itself is a direct field write, not one of the ImGuiWidgets:: calls
	// that would otherwise mark it dirty automatically.
	void CutNavmeshHole(const FW_GrowingArray<Vector2f>& someCutPositions);

	// Same idea for the single-point spawn/goal writes, against the LevelData asset instead.
	void SetStartPosition(const Vector2f& aPosition);
	void SetGoalPosition(const Vector2f& aPosition);

	void SaveAssets();
	void DiscardUnsavedChanges();

	Level& myLevel;

	bool myIsBoxCutModeActive = false;
	bool myBoxCutHasStartCorner = false;
	Vector2f myBoxCutStartCorner;

	bool myIsManualCutModeActive = false;
	FW_GrowingArray<Vector2f> myCutPositions;

	bool myIsSetStartModeActive = false;
	bool myIsSetGoalModeActive = false;

	bool myWantToOpenUnsavedChangesPopup = false;
};
