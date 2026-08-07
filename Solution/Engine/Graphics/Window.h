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

		// Single chokepoint for every way of quitting - resolves any unsaved changes (via the owning
		// IAppLayout's Dockables) before actually closing. With no unsaved changes, closes immediately.
		void Close();

		void Hide();

		void ToggleEditorUI() { myShowEditorUI = !myShowEditorUI; }

		void SetAppLayout(IAppLayout* aLayout);

		void RenderAppLayout();

		sf::RenderWindow* GetRenderWindow() const { return myRenderWindow; }
		Renderer& GetRenderer() const { return *myRenderer; }

		Rectf GetWindowRect() const { return myWindowRect; }
		Rectf GetGameViewRect() const { return myGameViewRect; }

	private:
		void SaveAppLayoutConfig();
		void LoadAppLayoutConfig();

		void SaveScreenshot();

		// Called every frame (from Present()) while a close is pending, until it's resolved or cancelled.
		void UpdatePendingClose();

		// Single point where myShouldBeOpen actually flips false, so there's always a log line marking
		// a graceful shutdown - useful for telling it apart from a crash or a forcibly-killed process.
		void ConfirmClose(const char* aReason);

		Vector2f GetSizeThatRespectsAspectRatio(int aWidth, int aHeight) const;

		Rectf myWindowRect;
		Rectf myGameViewRect;
		float myAspectRatio = 16.f / 9.f;
		bool myShowEditorUI = true;

		sf::RenderWindow* myRenderWindow = nullptr;
		Renderer* myRenderer = nullptr;
		bool myShouldBeOpen = true;
		bool myCloseRequested = false;
		bool myDisplayImGUIDemo = false;
		bool myScreenshotRequested = false;
		IAppLayout* myAppLayout = nullptr;
	};
}
