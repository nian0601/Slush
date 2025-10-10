#include "stdafx.h"

#include <Graphics\Window.h>

#include "Core/Dockables/GameViewDockable.h"
#include "Core/Dockables/TextureViewerDockable.h"
#include "Core/Dockables/LogDockable.h"
#include "Core/Dockables/ContentBrowserDockable.h"

#include "ActionGameGlobals.h"
#include "EntityEditorLayout.h"
#include "AssetEditorDockable.h"

EntityEditorLayout::EntityEditorLayout()
{
	ActionGameGlobals& globals = ActionGameGlobals::GetInstance();

	Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
	window.AddDockable(new Slush::LogDockable());
	window.AddDockable(new Slush::TextureViewerDockable(globals.GetTextureStorage()));
	window.AddDockable(new AssetEditorDockable(globals.GetEntityPrefabStorage(), "EntityPrefab Editor"));
	window.AddDockable(new AssetEditorDockable(globals.GetLevelDataStorage(), "LevelData Editor"));

	Slush::ContentBrowserDockable* contentBrowser = new Slush::ContentBrowserDockable();
	window.AddDockable(contentBrowser);

	contentBrowser->AddAssetStorage(&globals.GetEntityPrefabStorage());
	contentBrowser->AddAssetStorage(&globals.GetTextureStorage());
	contentBrowser->AddAssetStorage(&globals.GetUILayoutStorage());	
	contentBrowser->AddAssetStorage(&globals.GetLevelDataStorage());
}

EntityEditorLayout::~EntityEditorLayout()
{
}

void EntityEditorLayout::Update()
{
}

void EntityEditorLayout::Render()
{
	Slush::Engine& engine = Slush::Engine::GetInstance();
	engine.GetWindow().StartOffscreenBuffer();



	engine.GetWindow().EndOffscreenBuffer();
}