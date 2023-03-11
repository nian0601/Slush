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

		void Close() { myShouldBeOpen = false; }

		sf::RenderWindow* GetRenderWindow() const { return myRenderWindow; }

	private:
		sf::RenderWindow* myRenderWindow = nullptr;
		bool myShouldBeOpen = true;
	};
}
