#include "stdafx.h"

#include "NavmeshDebuggingLayout.h"

#include "NavmeshDebuggerDockable.h"

#include "Core/Engine.h"
#include "Core/Dockables/GameViewDockable.h"
#include "Graphics/Window.h"
#include "Graphics/Renderer.h"

NavmeshDebuggingLayout::NavmeshDebuggingLayout()
	: Slush::IAppLayout("TopDownGame")
{
	myNavmesh.GenerateDefaultGrid();

	AddDockable(new Slush::GameViewDockable());

	myNavmeshDebuggerDockable = new NavmeshDebuggerDockable(myNavmesh);
	AddDockable(myNavmeshDebuggerDockable);
}

void NavmeshDebuggingLayout::OnRender()
{
	Slush::Engine& engine = Slush::Engine::GetInstance();
	engine.GetWindow().GetRenderer().StartOffscreenBuffer();

	myNavmeshDebuggerDockable->RenderDebugOverlay();

	engine.GetWindow().GetRenderer().EndOffscreenBuffer();
}
