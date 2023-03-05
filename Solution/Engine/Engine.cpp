#include "stdafx.h"

#include "Engine.h"

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
}