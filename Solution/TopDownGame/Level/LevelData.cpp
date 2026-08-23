#include "stdafx.h"

#include "LevelData.h"
#include <imgui\ImGuiWidgets.h>

LevelData::LevelData(const char* aName, unsigned int aAssetID)
	: DataAsset(aName, aAssetID)
{
}

void LevelData::OnParse(Slush::AssetParser::Handle aRootHandle, unsigned int /*aVersion*/)
{
	if (aRootHandle.IsReading())
		OnLoad(aRootHandle);
	else
		OnSave(aRootHandle);
}

void LevelData::ResolveDependencies()
{
	myNavmeshData.ResolveDependency();
}

void LevelData::BuildUI()
{
	Slush::ImGuiWidgets::InputFloat2("Start Position", &myStartPosition.x);
	Slush::ImGuiWidgets::InputFloat2("Goal Position", &myGoalPosition.x);

	ImGui::Text("Navmesh: %s", myNavmeshData.GetName().GetBuffer());
	if (ImGui::BeginDragDropTarget())
	{
		if (Slush::Asset* asset = ImGui::AcceptDraggedAsset(Slush::GetAssetID<NavmeshData>()))
			myNavmeshData.Set(static_cast<NavmeshData*>(asset));

		ImGui::EndDragDropTarget();
	}
}

void LevelData::OnLoad(Slush::AssetParser::Handle aRootHandle)
{
	aRootHandle.ParseVec2fField("startPosition", myStartPosition);
	aRootHandle.ParseVec2fField("goalPosition", myGoalPosition);
	myNavmeshData.Parse(aRootHandle, "navmeshData");
}

void LevelData::OnSave(Slush::AssetParser::Handle aRootHandle)
{
	aRootHandle.ParseVec2fField("startPosition", myStartPosition);
	aRootHandle.ParseVec2fField("goalPosition", myGoalPosition);
	myNavmeshData.Parse(aRootHandle, "navmeshData");
}
