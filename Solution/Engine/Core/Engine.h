#pragma once

namespace Slush
{
	class Window;
	class Input;
	class IApp;

	class Engine
	{
	public:
		static Engine& GetInstance();
		static void Shutdown();

		void Run(IApp& anApp);

		const Input& GetInput() const { return *myInput; }

		Window& GetWindow() { return *myWindow; }

	private:
		Engine();
		~Engine();
		static Engine* ourInstance;

		Window* myWindow;
		Input* myInput;
	};
}