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
		void RenderOffscreenBufferToImGUI();

		void Present();

		void Close() { myShouldBeOpen = false; }

		void StartOffscreenBuffer();
		void EndOffscreenBuffer();
		void SetShouldRenderOffscreenBufferToScreen(bool aState) { myShouldRenderOffscreenBufferToScreen = aState; }

		sf::RenderWindow* GetRenderWindow() const { return myRenderWindow; }
		sf::RenderTexture* GetOffscreenBuffer() const { return myOffscreenBuffer; }

		sf::RenderTarget* GetActiveRenderTarget() const { return myActiveRenderTarget; }

	private:
		Vector2f GetSizeThatRespectsAspectRatio(int aWidth, int aHeight) const;

		int myWidth;
		int myHeight;
		float myAspectRatio;
		bool myShouldRenderOffscreenBufferToScreen = false;

		sf::RenderWindow* myRenderWindow = nullptr;
		bool myShouldBeOpen = true;

		sf::RenderTarget* myActiveRenderTarget = nullptr;
		sf::RenderTexture* myOffscreenBuffer = nullptr;
	};
}
