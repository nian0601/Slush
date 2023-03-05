#pragma once

namespace Slush
{
	class Engine
	{
	public:
		static Engine& GetInstance();
		static void Shutdown();

	private:
		static Engine* ourInstance;
	};
}