#include "stdafx.h"

#include "LevelEditorLayout.h"

#include "LevelEditorDockable.h"
#include "Level/Level.h"

#include "Core/Engine.h"
#include "Core/Dockables/GameViewDockable.h"
#include "Graphics/Window.h"
#include "Graphics/Renderer.h"

LevelEditorLayout::LevelEditorLayout(Level& aLevel)
	: Slush::IAppLayout("LevelEditor", "Level Editor")
	, myLevel(aLevel)
{
	AddDockable(new Slush::GameViewDockable());

	myLevelEditorDockable = new LevelEditorDockable(aLevel);
	AddDockable(myLevelEditorDockable);
}

void LevelEditorLayout::OnRender()
{
	Slush::Engine& engine = Slush::Engine::GetInstance();
	engine.GetWindow().GetRenderer().StartOffscreenBuffer();

	myLevel.Render();
	myLevelEditorDockable->RenderOverlay();

	engine.GetWindow().GetRenderer().EndOffscreenBuffer();
}
