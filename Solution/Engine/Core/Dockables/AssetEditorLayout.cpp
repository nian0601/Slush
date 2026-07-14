#include "stdafx.h"

#include "AssetEditorLayout.h"

#include "Graphics/Window.h"

#include "Core/Dockables/AssetEditorDockable.h"
#include "Core/Dockables/TextureViewerDockable.h"
#include "Core/Dockables/LogDockable.h"
#include "Core/Dockables/ContentBrowserDockable.h"

namespace Slush
{
	AssetEditorLayout::AssetEditorLayout()
		: IAppLayout("Assets")
	{
		AddDockable(new LogDockable());
		AddDockable(new TextureViewerDockable());
		AddDockable(new AssetEditorDockable());
		AddDockable(new ContentBrowserDockable());
	}

	AssetEditorLayout::~AssetEditorLayout()
	{
	}

	void AssetEditorLayout::OnUpdate()
	{
	}

	void AssetEditorLayout::OnRender()
	{
		Slush::Engine& engine = Slush::Engine::GetInstance();
		engine.GetWindow().StartOffscreenBuffer();



		engine.GetWindow().EndOffscreenBuffer();
	}
}
