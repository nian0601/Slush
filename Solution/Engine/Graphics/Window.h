#pragma once

namespace sf
{
	class RenderWindow;
}

namespace Slush
{
	class IAppLayout;
	class Dockable;
	class Renderer;
	class Window
	{
	public:
		Window(unsigned int aWidth, unsigned int aHeight);
		~Window();

		bool PumpEvents();
		void RenderOffscreenBufferToImGUI();

		void RequestScreenshot() { myScreenshotRequested = true; }

		void Present();

		void Close() { myShouldBeOpen = false; }

		void Hide();

		void ToggleEditorUI() { myShowEditorUI = !myShowEditorUI; }

		void SetAppLayout(IAppLayout* aLayout);

		void UpdateAppLayout();
		void RenderAppLayout();

		sf::RenderWindow* GetRenderWindow() const { return myRenderWindow; }
		Renderer& GetRenderer() const { return *myRenderer; }

		Rectf GetWindowRect() const { return myWindowRect; }
		Rectf GetGameViewRect() const { return myGameViewRect; }

	private:
		void SaveAppLayoutConfig();
		void LoadAppLayoutConfig();

		void SaveScreenshot();

		Vector2f GetSizeThatRespectsAspectRatio(int aWidth, int aHeight) const;

		Rectf myWindowRect;
		Rectf myGameViewRect;
		float myAspectRatio = 16.f / 9.f;
		bool myShowEditorUI = true;

		sf::RenderWindow* myRenderWindow = nullptr;
		Renderer* myRenderer = nullptr;
		bool myShouldBeOpen = true;
		bool myDisplayImGUIDemo = false;
		bool myScreenshotRequested = false;
		IAppLayout* myAppLayout = nullptr;
	};
}
