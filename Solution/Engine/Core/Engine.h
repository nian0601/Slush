#pragma once
#include "FW_String.h"

namespace Slush
{
	class Window;
	class Input;
	class IApp;
	class Logger;

	class Engine
	{
	public:
		static Engine& GetInstance();
		static void Destroy();

		void Initialize();
		void Shutdown();

		void Run(IApp& anApp);

		const Input& GetInput() const { return *myInput; }
		Logger& GetLogger() { return *myLogger; }

		Window& GetWindow() { return *myWindow; }

		const FW_String& GetDataFolder() const { return myDataFolder; }

		bool myByPassImGUIInputRestriction = false;
	private:
		Engine() {};
		~Engine() {};
		static Engine* ourInstance;

		Window* myWindow = nullptr;
		Input* myInput = nullptr;
		Logger* myLogger = nullptr;

		FW_String myDataFolder;
	};
}