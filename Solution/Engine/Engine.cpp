#include "stdafx.h"

#include "Engine.h"
#include "Window.h"

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
		while (myWindow->PumpEvents())
		{
			// Do things
		}
	}

	Engine::Engine()
	{
		myWindow = new Window(800, 600);
	}

	Engine::~Engine()
	{
		FW_SAFE_DELETE(myWindow);
	}
}