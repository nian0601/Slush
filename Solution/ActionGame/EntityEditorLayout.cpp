#include "stdafx.h"

#include <Graphics\Window.h>

#include "Core/Dockables/GameViewDockable.h"
#include "Core/Dockables/TextureViewerDockable.h"
#include "Core/Dockables/LogDockable.h"
#include "Core/Dockables/ContentBrowserDockable.h"

#include "ActionGameGlobals.h"
#include "EntityEditorLayout.h"
#include "AssetEditorDockable.h"
#include <Level\LevelData.h>

EntityEditorLayout::EntityEditorLayout()
{
	ActionGameGlobals& globals = ActionGameGlobals::GetInstance();

	Slush::Window& window = Slush::Engine::GetInstance().GetWindow();
	window.AddDockable(new Slush::LogDockable());
	window.AddDockable(new Slush::TextureViewerDockable());

	Slush::AssetRegistry& assets = Slush::AssetRegistry::GetInstance();
	window.AddDockable(new AssetEditorDockable(assets.GetAssetStorage<EntityPrefab>(), "EntityPrefab Editor"));
	window.AddDockable(new AssetEditorDockable(assets.GetAssetStorage<LevelData>(), "LevelData Editor"));

	Slush::ContentBrowserDockable* contentBrowser = new Slush::ContentBrowserDockable();
	window.AddDockable(contentBrowser);
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