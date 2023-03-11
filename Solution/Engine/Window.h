#pragma once

namespace sf
{
	class RenderWindow;
}

namespace Slush
{
	class Window
	{
	public:
		Window(int aWidth, int aHeight);
		~Window();

		bool PumpEvents();
		void RenderImGUI();

		void Present();

	private:
		sf::RenderWindow* myRenderWindow;
	};
}
