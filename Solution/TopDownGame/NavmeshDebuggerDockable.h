#pragma once

#include <Core/Dockables/Dockable.h>
#include <FW_String.h>

class Navmesh;

class NavmeshDebuggerDockable : public Slush::DockableBase<NavmeshDebuggerDockable>
{
public:
	NavmeshDebuggerDockable(Navmesh& aNavmesh);

	const char* GetName() const override { return "Navmesh Debugger"; }

	void RenderDebugOverlay() const;

protected:
	void OnUpdate() override;
	void OnBuildUI() override;

private:
	struct PathfindResult
	{
		int myID = 0;
		FW_String myLabel;
		Vector2f myStart;
		Vector2f myGoal;
		FW_GrowingArray<Vector2f> myWaypoints;
	};

	void BuildPathfindResultsList();

	void UpdateBoxCutMode();
	void UpdateManualCutMode();
	void UpdatePathfindTestMode();

	void RenderPathfindResults() const;
	void RenderBoxCutPreview() const;
	void RenderManualCutPreview() const;

	void DisableBoxCutMode();
	void DisablePathfindTestMode();
	void DisableManualCutMode();

	Navmesh& myNavmesh;

	bool myIsBoxCutModeActive = false;
	bool myBoxCutHasStartCorner = false;
	Vector2f myBoxCutStartCorner;

	bool myIsManualCutModeActive = false;
	FW_GrowingArray<Vector2f> myCutPositions;

	bool myIsPathfindTestModeActive = false;
	bool myPathfindTestHasStart = false;
	Vector2f myPathfindTestStart;

	FW_GrowingArray<PathfindResult> myPathfindResults;
	int myNextPathfindResultID = 1;
	int mySelectedPathfindResultID = -1;
};
