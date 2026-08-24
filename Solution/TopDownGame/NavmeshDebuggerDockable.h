#pragma once

#include <Core/Dockables/Dockable.h>
#include <FW_String.h>

class Navmesh;

class NavmeshDebuggerDockable : public Slush::DockableBase<NavmeshDebuggerDockable>
{
public:
	NavmeshDebuggerDockable(Navmesh& aNavmesh);

	const char* GetName() const override { return "Navmesh Debugger"; }

	void RenderPathfindResults() const;

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
	void UpdatePathfindTestMode();

	void DisableBoxCutMode();
	void DisablePathfindTestMode();
	void DisableManualCutMode();

	Navmesh& myNavmesh;

	bool myIsBoxCutModeActive = false;
	bool myIsManualCutModeActive = false;

	bool myIsPathfindTestModeActive = false;
	bool myPathfindTestHasStart = false;
	Vector2f myPathfindTestStart;

	FW_GrowingArray<PathfindResult> myPathfindResults;
	int myNextPathfindResultID = 1;
	int mySelectedPathfindResultID = -1;
};
