#pragma once

namespace Slush
{
	class Window;
	class Input;

	class Engine
	{
	public:
		static Engine& GetInstance();
		static void Shutdown();

		void Run();

		const Input& GetInput() const { return *myInput; }

	private:
		Engine();
		~Engine();
		static Engine* ourInstance;

		Window* myWindow;
		Input* myInput;
	};
}