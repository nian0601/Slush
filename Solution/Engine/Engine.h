#pragma once

namespace Slush
{
	class Window;

	class Engine
	{
	public:
		static Engine& GetInstance();
		static void Shutdown();

		void Run();

	private:
		Engine();
		~Engine();
		static Engine* ourInstance;

		Window* myWindow;
	};
}