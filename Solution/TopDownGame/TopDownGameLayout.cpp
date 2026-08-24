#include "stdafx.h"

#include "TopDownGameLayout.h"

#include "NavmeshDebuggerDockable.h"

#include "Core/Engine.h"
#include "Core/Dockables/GameViewDockable.h"
#include "Graphics/Window.h"
#include "Graphics/Renderer.h"

TopDownGameLayout::TopDownGameLayout(Navmesh& aNavmesh)
	: Slush::IAppLayout("TopDownGame")
	, myNavmesh(aNavmesh)
{
	AddDockable(new Slush::GameViewDockable());

	myNavmeshDebuggerDockable = new NavmeshDebuggerDockable(myNavmesh);
	AddDockable(myNavmeshDebuggerDockable);
}

void TopDownGameLayout::OnRender()
{
	Slush::Engine& engine = Slush::Engine::GetInstance();
	engine.GetWindow().GetRenderer().StartOffscreenBuffer();

	myNavmeshDebuggerDockable->RenderPathfindResults();

	engine.GetWindow().GetRenderer().EndOffscreenBuffer();
}
