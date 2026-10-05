#include "stdafx.h"

#include "Core/Engine.h"
#include "Core/CommandLineArgs.h"
#include "Core/Input.h"
#include "Core/Time.h"
#include "Core/IApp.h"
#include "Core/Log.h"

#include "Graphics/Window.h"
#include "Graphics/Renderer.h"

#include <windows.h>
#include <FW_FileSystem.h>

#include "Core/EditorTheme.h"

namespace Slush
{
	Engine* Engine::ourInstance = nullptr;
	Engine& Engine::GetInstance()
	{
		if (!ourInstance)
			ourInstance = new Engine();

		return *ourInstance;
	}

	void Engine::Destroy()
	{
		ourInstance->Shutdown();
		FW_SAFE_DELETE(ourInstance);
	}

	void Engine::Initialize()
	{
		// TODO: Make File-handling a part of engine instead to simplify filepath-handling?
		FW_FileSystem::InitDataFolderFromExecutable();

		myLogger = new Logger();
		myWindow = new Window(1920, 1080);
		myInput = new Input();
		Time::Init();

		if (CommandLineArgs::GetInstance().HasFlag("-hidewindow"))
			myWindow->Hide();

		// Editor look: swap these to compare (IMGUI_DEFAULT / COLD / WARM, NOTO_SANS / JETBRAINS_MONO)
		EditorTheme::ApplyPalette(EditorTheme::WARM);
		EditorTheme::LoadFonts(EditorTheme::JETBRAINS_MONO);
	}

	void Engine::Shutdown()
	{
		CommandLineArgs::Destroy();

		FW_SAFE_DELETE(myInput);
		FW_SAFE_DELETE(myWindow);

		myLogger->ForceFlush();
		FW_SAFE_DELETE(myLogger);
	}

	void Engine::BeginFrame()
	{
		Time::Update();
		myLogger->Update();
	}

	void Engine::UpdateInput()
	{
		ImGuiIO& imguiIO = ImGui::GetIO();

		if (!imguiIO.WantCaptureKeyboard)
			myInput->UpdateKeyboard();

		if (CommandLineArgs::GetInstance().HasFlag("-usedebuginput"))
			myInput->PollDebugInputFile(imguiIO.WantCaptureKeyboard);

		if (!imguiIO.WantCaptureMouse || myByPassImGUIInputRestriction)
		{
			myInput->UpdateMouse(*myWindow->GetRenderWindow());
			myInput->RemapMousePosition(myWindow->GetWindowRect(), myWindow->GetGameViewRect());
		}

		if (myInput->WasKeyPressed(Slush::Input::_F10))
			myWindow->RequestScreenshot();

		if (myInput->WasKeyPressed(Slush::Input::HYPHEN))
			myWindow->ToggleEditorUI();

		myWindow->UpdatePendingClose();
	}

	void Engine::BeginImGuiFrame()
	{
		ImGui::SFML::Update(*myWindow->GetRenderWindow(), Time::GetDelta());
	}

	void Engine::UpdateSimulation(IApp& anApp)
	{
		anApp.Update();
		myWindow->UpdateAppLayout();
		myWindow->GetRenderer().UpdateFade();
	}

	void Engine::RenderFrame(IApp& anApp)
	{
		anApp.Render();
		myWindow->RenderAppLayout();

		myWindow->GetRenderer().ProcessRenderQueue();
		myWindow->GetRenderer().RenderFade();
		myWindow->GetRenderer().FinalizeOffscreenBuffer();
	}

	void Engine::BuildEditorUI()
	{
		myWindow->BuildEditorChrome();
	}

	void Engine::CompositeAndPresent()
	{
		// The editor path composites via GameViewDockable -> RenderOffscreenBufferToImGUI() -> ImGui::Image
		// instead, since the game view there is an ImGui-laid-out dockable panel.
		if (!myWindow->IsEditorUIVisible())
			myWindow->Composite();

		myWindow->Present();
	}

	void Engine::Run(IApp& anApp)
	{
		anApp.Initialize();

		while (myWindow->IsOpen())
		{
			myWindow->PumpEvents();
			BeginFrame();
			UpdateInput();
			BeginImGuiFrame();
			UpdateSimulation(anApp);
			RenderFrame(anApp);
			BuildEditorUI();
			CompositeAndPresent();
		}

		anApp.Shutdown();
	}
}
