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
	: Slush::IAppLayout("Entity")
{
	ActionGameGlobals& globals = ActionGameGlobals::GetInstance();

	AddDockable(new Slush::LogDockable());
	AddDockable(new Slush::TextureViewerDockable());
	AddDockable(new AssetEditorDockable());
	AddDockable(new Slush::ContentBrowserDockable());
}

EntityEditorLayout::~EntityEditorLayout()
{
}

void EntityEditorLayout::OnUpdate()
{
}

void EntityEditorLayout::OnRender()
{
	Slush::Engine& engine = Slush::Engine::GetInstance();
	engine.GetWindow().StartOffscreenBuffer();



	engine.GetWindow().EndOffscreenBuffer();
}