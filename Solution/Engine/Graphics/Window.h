#pragma once

namespace sf
{
	class RenderWindow;
	class RenderTexture;
	class RenderTarget;
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

		void StartOffscreenBuffer();
		void EndOffscreenBuffer();

		sf::RenderWindow* GetRenderWindow() const { return myRenderWindow; }
		sf::RenderTexture* GetOffscreenBuffer() const { return myOffscreenBuffer; }

		sf::RenderTarget* GetActiveRenderTarget() const { return myActiveRenderTarget; }

	private:
		sf::RenderWindow* myRenderWindow = nullptr;
		bool myShouldBeOpen = true;

		sf::RenderTarget* myActiveRenderTarget = nullptr;
		sf::RenderTexture* myOffscreenBuffer = nullptr;
	};
}
