#include "stdafx.h"

#include "LevelEditorLayout.h"

#include "LevelEditorDockable.h"
#include "Level/Level.h"
#include "Level/LevelData.h"

#include "Core/Assets/AssetStorage.h"
#include "Core/Engine.h"
#include "Core/Dockables/GameViewDockable.h"
#include "Core/Dockables/LogDockable.h"
#include "Graphics/Window.h"
#include "Graphics/Renderer.h"

namespace
{
	const char* const ourLevelDataAssetName = "level_main";
}

LevelEditorLayout::LevelEditorLayout()
	: Slush::IAppLayout("LevelEditor", "Level Editor")
{
	LevelData* levelData = Slush::AssetRegistry::GetInstance().GetAsset<LevelData>(ourLevelDataAssetName);
	FW_ASSERT(levelData, "Level Editor has no valid LevelData - expected a 'level_main' LevelData asset");
	myLevel = new Level(*levelData);

	AddDockable(new Slush::GameViewDockable());

	myLevelEditorDockable = new LevelEditorDockable(*myLevel);
	AddDockable(myLevelEditorDockable);
	AddDockable(new Slush::LogDockable());
}

LevelEditorLayout::~LevelEditorLayout()
{
	FW_SAFE_DELETE(myLevel);
}

void LevelEditorLayout::OnUpdate()
{
	myLevel->Update();
}

void LevelEditorLayout::OnRender()
{
	Slush::Engine& engine = Slush::Engine::GetInstance();
	engine.GetWindow().GetRenderer().StartOffscreenBuffer();

	myLevelEditorDockable->RenderOverlay();
	myLevel->Render();

	engine.GetWindow().GetRenderer().EndOffscreenBuffer();
}
