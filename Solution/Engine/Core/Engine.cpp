#include "stdafx.h"

#include "Core/Engine.h"
#include "Core/CommandLineArgs.h"
#include "Core/Input.h"
#include "Core/DebugScriptRunner.h"
#include "Core/AssetRepository.h"
#include "Core/Time.h"
#include "Core/IApp.h"
#include "Core/Log.h"

#include "Graphics/Window.h"
#include "Graphics/Renderer.h"



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
		myLogger = new Logger();
		myWindow = new Window(1920, 1080);
		myInput = new Input();
		Time::Init();
		myDebugScriptRunner = new DebugScriptRunner();
		myDebugScriptRunner->SetReporter([](bool anIsError, const char* aMessage, void*)
		{
			if (anIsError)
			{
				SLUSH_ERROR("[DebugScript] %s", aMessage);
			}
			else
			{
				SLUSH_INFO("%s", aMessage);
			}
		});
		myDebugScriptRunner->RegisterCommand("key", [](DebugScriptRunner& aRunner, const DebugScriptRunner::Arguments& anArguments)
		{
			if (!aRunner.RequireArgumentCount(anArguments, 1))
				return;
			Input::KeyCode key;
			if (!InputUtils::FindKeyCode(anArguments[0].GetBuffer(), key))
				aRunner.Fail("unknown key name");
			else if (ImGui::GetIO().WantCaptureKeyboard)
				aRunner.Fail("key skipped: ImGui has captured keyboard input");
			else
				Engine::GetInstance().myInput->InjectKeyClick(key);
		});
		myDebugScriptRunner->RegisterCommand("screenshot", [](DebugScriptRunner& aRunner, const DebugScriptRunner::Arguments& anArguments)
		{
			if (anArguments.Count() > 1)
			{
				aRunner.Fail("screenshot expects at most one name");
				return;
			}
			const char* name = anArguments.IsEmpty() ? nullptr : anArguments[0].GetBuffer();
			if (name && !Logger::IsValidLogName(name))
			{
				aRunner.Fail("invalid screenshot name: use only A-Z, a-z, 0-9, underscore or hyphen");
				return;
			}
			Engine::GetInstance().GetWindow().RequestScreenshot(name);
		});
		myDebugScriptRunner->RegisterCommand("click", [](DebugScriptRunner& aRunner, const DebugScriptRunner::Arguments& anArguments)
		{
			if (aRunner.RequireArgumentCount(anArguments, 1))
				aRunner.RequestClick(anArguments[0].GetBuffer());
		});
		myDebugScriptRunner->RegisterCommand("quit", [](DebugScriptRunner& aRunner, const DebugScriptRunner::Arguments& anArguments)
		{
			if (aRunner.RequireArgumentCount(anArguments, 0))
				Engine::GetInstance().GetWindow().QuitFromDebugScript();
		});
		CommandLineArgs& args = CommandLineArgs::GetInstance();
		if (args.HasFlag("-debugscript"))
		{
			const char* path = args.GetString("-debugscript");
			if (path)
				myDebugScriptRunner->QueueFile(path);
			else
				myDebugScriptRunner->Fail("-debugscript requires a path");
		}

		if (CommandLineArgs::GetInstance().HasFlag("-hidewindow"))
			myWindow->Hide();

		// Editor look: swap these to compare (IMGUI_DEFAULT / COLD / WARM, NOTO_SANS / JETBRAINS_MONO)
		EditorTheme::ApplyPalette(EditorTheme::WARM);
		EditorTheme::LoadFonts(EditorTheme::JETBRAINS_MONO);
	}

	void Engine::Shutdown()
	{
		CommandLineArgs::Destroy();

		FW_SAFE_DELETE(myDebugScriptRunner);
		FW_SAFE_DELETE(myInput);
		FW_SAFE_DELETE(myWindow);

		myLogger->ForceFlush();
		FW_SAFE_DELETE(myLogger);
	}

	void Engine::BeginFrame()
	{
		Time::Update();
	}

	void Engine::UpdateInput()
	{
		ImGuiIO& imguiIO = ImGui::GetIO();

		if (!imguiIO.WantCaptureKeyboard)
			myInput->UpdateKeyboard();

		if (CommandLineArgs::GetInstance().HasFlag("-usedebuginput"))
		{
			FW_String path;
			AssetRepository::GetDebugFilePath("debug_input.txt", path);
			myDebugScriptRunner->QueueFile(path.GetBuffer(), true, true);

		}

		myDebugScriptRunner->Step(Time::GetDelta());

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
		myWindow->SaveAppLayoutConfigIfRequested();
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

	int Engine::Run(IApp& anApp)
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
			myDebugScriptRunner->EndFrame();
		}

		myWindow->DeleteAllLayouts();
		anApp.Shutdown();
		return myDebugScriptRunner->HasFailed() ? 1 : 0;
	}
}
