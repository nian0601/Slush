#include "stdafx.h"
#include "GameViewDockable.h"

#include "Core/Engine.h"
#include "Graphics/Window.h"

namespace Slush
{
	void GameViewDockable::OnBuildUI()
	{
		Engine& engine = Engine::GetInstance();
		engine.GetWindow().RenderOffscreenBufferToImGUI();

		engine.myByPassImGUIInputRestriction = ImGui::IsItemHovered();
	}
}