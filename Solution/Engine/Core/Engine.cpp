#include "stdafx.h"

#include "Core/Engine.h"
#include "Core/Input.h"
#include "Core/Time.h"
#include "Core/IApp.h"
#include "Core/Log.h"

#include "Graphics/Window.h"

#include <windows.h>
#include <string>
#include <FW_FileSystem.h>

namespace Slush
{
	void ReplaceAllOccurancesInString(std::string& aString, const std::string& aSomethingToReplace, const std::string& aNewString)
	{
		// Get the first occurrence
		size_t pos = aString.find(aSomethingToReplace);

		// Repeat till end is reached
		while (pos != std::string::npos)
		{
			// Replace this occurrence of Sub String
			aString.replace(pos, aSomethingToReplace.size(), aNewString);
			// Get the next occurrence from the current position
			pos = aString.find(aSomethingToReplace, pos + aNewString.size());
		}
	}

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

		char fileBuffer[128];
		GetModuleFileNameA(NULL, fileBuffer, 128);
		
		std::string assetPath;
		assetPath.append(fileBuffer);
		assetPath.erase(assetPath.rfind("\\"), std::string::npos);
		ReplaceAllOccurancesInString(assetPath, "\\", "/");
		assetPath.append("/");
		myDataFolder = assetPath.c_str();

		// TODO: Make File-handling a part of engine instead to simplify filepath-handling?
		FW_FileSystem::SetDataFolder(myDataFolder.GetBuffer());
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

			ImGuiIO& imguiIO = ImGui::GetIO();

			if(!imguiIO.WantCaptureKeyboard)
				myInput->UpdateKeyboard();

			if (!imguiIO.WantCaptureMouse || myByPassImGUIInputRestriction)
			{
				myInput->UpdateMouse(*myWindow->GetRenderWindow());
				myInput->RemapMousePosition(myWindow->GetWindowRect(), myWindow->GetGameViewRect());
			}

			anApp.Update();
			anApp.Render();

			if (myInput->WasKeyPressed(Slush::Input::HYPHEN))
				myWindow->ToggleEditorUI();

			if (myInput->WasKeyPressed(Slush::Input::ESC))
				myWindow->Close();

			myWindow->Present();
		}

		anApp.Shutdown();
	}
}