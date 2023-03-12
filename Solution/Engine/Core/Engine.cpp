#include "stdafx.h"

#include "Core/Engine.h"
#include "Core/Input.h"
#include "Core/Time.h"
#include "Core/IApp.h"
#include "Core/Log.h"

#include "Graphics/Window.h"

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
		myWindow = new Window(1280, 720);
		myInput = new Input();
		Time::Init();
	}

	void Engine::Shutdown()
	{
		FW_SAFE_DELETE(myInput);
		FW_SAFE_DELETE(myWindow);
		FW_SAFE_DELETE(myLogger);
	}

	void Engine::Run(IApp& anApp)
	{
		anApp.Initialize();

		while (myWindow->PumpEvents())
		{
			Time::Update();
			myInput->UpdateKeyboard();
			myInput->UpdateMouse(*myWindow->GetRenderWindow());

			anApp.Update();
			anApp.Render();

			anApp.RenderImGUI();

			myWindow->RenderImGUI();
			myWindow->Present();
		}

		anApp.Shutdown();
	}
}