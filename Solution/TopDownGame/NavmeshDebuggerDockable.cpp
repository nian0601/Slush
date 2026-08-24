#include "stdafx.h"

#include "NavmeshDebuggerDockable.h"

#include "Navmesh.h"

#include "Core/Engine.h"
#include "Core/Input.h"
#include "Graphics/Window.h"
#include "Graphics/Renderer.h"

NavmeshDebuggerDockable::NavmeshDebuggerDockable(Navmesh& aNavmesh)
	: Slush::DockableBase<NavmeshDebuggerDockable>()
	, myNavmesh(aNavmesh)
{
}

void NavmeshDebuggerDockable::RenderDebugOverlay() const
{
	RenderPathfindResults();
	RenderBoxCutPreview();
	RenderManualCutPreview();
}

void NavmeshDebuggerDockable::RenderPathfindResults() const
{
	Slush::Renderer& renderer = Slush::Engine::GetInstance().GetWindow().GetRenderer();

	const int pathColor = 0xFFFFFF00;

	for (const PathfindResult& result : myPathfindResults)
	{
		if (mySelectedPathfindResultID != -1 && mySelectedPathfindResultID != result.myID)
			continue;

		for (int i = 0; i < result.myWaypoints.Count() - 1; ++i)
			renderer.RenderLine(result.myWaypoints[i], result.myWaypoints[i + 1], pathColor);
	}
}

void NavmeshDebuggerDockable::RenderBoxCutPreview() const
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

void NavmeshDebuggerDockable::RenderManualCutPreview() const
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

void NavmeshDebuggerDockable::OnUpdate()
{
	UpdateBoxCutMode();
	UpdateManualCutMode();
	UpdatePathfindTestMode();
}

void NavmeshDebuggerDockable::UpdateBoxCutMode()
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

	myNavmesh.CutHole(boxCorners);
}

void NavmeshDebuggerDockable::UpdateManualCutMode()
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
		myNavmesh.CutHole(myCutPositions);
		myCutPositions.RemoveAll();
	}
}

void NavmeshDebuggerDockable::UpdatePathfindTestMode()
{
	if (!myIsPathfindTestModeActive)
		return;

	Slush::Engine& engine = Slush::Engine::GetInstance();
	if (!engine.GetInput().WasMouseReleased(Slush::Input::LEFTMB))
		return;

	if (!myPathfindTestHasStart)
	{
		myPathfindTestStart = engine.GetInput().GetMousePositionf();
		myPathfindTestHasStart = true;
		return;
	}

	const Vector2f goal = engine.GetInput().GetMousePositionf();
	myPathfindTestHasStart = false;

	FW_GrowingArray<Vector2f> waypoints;
	Navmesh::PathCorridor corridor;
	if (!myNavmesh.FindPath(myPathfindTestStart, goal, waypoints, corridor))
		return;

	PathfindResult& result = myPathfindResults.Add();
	result.myID = myNextPathfindResultID++;
	result.myLabel = "Path ";
	result.myLabel += result.myID;
	result.myStart = myPathfindTestStart;
	result.myGoal = goal;
	result.myWaypoints = waypoints;
}

void NavmeshDebuggerDockable::DisableBoxCutMode()
{
	if (!myIsBoxCutModeActive)
		return;

	myIsBoxCutModeActive = false;
	myBoxCutHasStartCorner = false;
}

void NavmeshDebuggerDockable::DisablePathfindTestMode()
{
	if (!myIsPathfindTestModeActive)
		return;

	myIsPathfindTestModeActive = false;
	myPathfindTestHasStart = false;
}

void NavmeshDebuggerDockable::DisableManualCutMode()
{
	if (!myIsManualCutModeActive)
		return;

	myIsManualCutModeActive = false;
	myCutPositions.RemoveAll();
}

void NavmeshDebuggerDockable::OnBuildUI()
{
	ImGui::Text("Navmesh Debugger");

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

			DisablePathfindTestMode();
			DisableManualCutMode();
		}
	}

	if (myIsPathfindTestModeActive)
	{
		if (ImGui::Button("Disable Pathfind Test"))
			DisablePathfindTestMode();
	}
	else
	{
		if (ImGui::Button("Enable Pathfind Test"))
		{
			myIsPathfindTestModeActive = true;
			myPathfindTestHasStart = false;

			DisableBoxCutMode();
			DisableManualCutMode();
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
			DisablePathfindTestMode();
		}
	}

	BuildPathfindResultsList();
}

void NavmeshDebuggerDockable::BuildPathfindResultsList()
{
	if (ImGui::Button("Clear All"))
	{
		myPathfindResults.RemoveAll();
		mySelectedPathfindResultID = -1;
	}

	for (int i = 0; i < myPathfindResults.Count(); ++i)
	{
		PathfindResult& result = myPathfindResults[i];

		ImGui::PushID(result.myID);

		// Selectable() defaults to stretching across the whole remaining row width, which would
		// otherwise sit underneath (and eat clicks meant for) the Remove button placed via SameLine()
		// below - give it an explicit width that stops short of the button instead.
		const float removeButtonWidth = ImGui::CalcTextSize("Remove").x + ImGui::GetStyle().FramePadding.x * 2.0f;
		const float selectableWidth = ImGui::GetContentRegionAvail().x - removeButtonWidth - ImGui::GetStyle().ItemSpacing.x;

		const bool isSelected = (mySelectedPathfindResultID == result.myID);
		if (ImGui::Selectable(result.myLabel.GetBuffer(), isSelected, 0, ImVec2(selectableWidth, 0)))
			mySelectedPathfindResultID = isSelected ? -1 : result.myID;

		ImGui::SameLine();
		if (ImGui::Button("Remove"))
		{
			if (mySelectedPathfindResultID == result.myID)
				mySelectedPathfindResultID = -1;

			myPathfindResults.RemoveNonCyclicAtIndex(i);
			ImGui::PopID();
			--i;
			continue;
		}

		ImGui::PopID();
	}
}
