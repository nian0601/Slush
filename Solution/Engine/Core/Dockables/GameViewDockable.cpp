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

		ImGuiIO& imguiIO = ImGui::GetIO();
		imguiIO.WantCaptureKeyboard = false;
		imguiIO.WantCaptureMouse = false;
	}
}