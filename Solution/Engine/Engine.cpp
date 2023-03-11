#include "stdafx.h"

#include "Engine.h"
#include "Window.h"
#include "Input.h"
#include "Time.h"

namespace Slush
{
	Engine* Engine::ourInstance = nullptr;
	Engine& Engine::GetInstance()
	{
		if (!ourInstance)
			ourInstance = new Engine();

		return *ourInstance;
	}

	void Engine::Shutdown()
	{
		FW_SAFE_DELETE(ourInstance);
	}

	void Engine::Run()
	{
		float x = 0.f;
		float y = 0.f;
		float z = 0.f;
		while (myWindow->PumpEvents())
		{
			Time::Update();
			myInput->UpdateKeyboard();
			myInput->UpdateMouse(*myWindow->GetRenderWindow());

			if (myInput->WasKeyPressed(Input::ESC))
				myWindow->Close();

			// Do things
			ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

			ImGui::ShowDemoWindow();

			if (ImGui::Begin("X Window"))
				ImGui::DragFloat("X", &x);
			ImGui::End();

			if (ImGui::Begin("Y Window"))
				ImGui::DragFloat("Y", &y);
			ImGui::End();

			if (ImGui::Begin("Z Window"))
				ImGui::DragFloat("Z", &z);
			ImGui::End();

			myWindow->RenderImGUI();
			myWindow->Present();
		}
	}

	Engine::Engine()
	{
		myWindow = new Window(1280, 720);
		myInput = new Input();
		Time::Init();
	}

	Engine::~Engine()
	{
		FW_SAFE_DELETE(myInput);
		FW_SAFE_DELETE(myWindow);
	}
}