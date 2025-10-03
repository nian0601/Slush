#pragma once

namespace sf
{
	class RenderWindow;
	class RenderTexture;
	class RenderTarget;
}

namespace Slush
{
	class Dockable;
	class Window
	{
	public:
		Window(int aWidth, int aHeight);
		~Window();

		bool PumpEvents();
		void RenderOffscreenBufferToImGUI();

		void Present();

		void Close() { myShouldBeOpen = false; }

		void StartOffscreenBuffer();
		void EndOffscreenBuffer();

		void ToggleEditorUI() { myShowEditorUI = !myShowEditorUI; }

		void AddDockable(Dockable* aDockable);

		sf::RenderWindow* GetRenderWindow() const { return myRenderWindow; }
		sf::RenderTexture* GetOffscreenBuffer() const { return myOffscreenBuffer; }

		sf::RenderTarget* GetActiveRenderTarget() const { return myActiveRenderTarget; }

		Rectf GetWindowRect() const { return myWindowRect; }
		Rectf GetGameViewRect() const { return myGameViewRect; }

	private:
		Vector2f GetSizeThatRespectsAspectRatio(int aWidth, int aHeight) const;

		Rectf myWindowRect;
		Rectf myGameViewRect;
		float myAspectRatio = 16.f / 9.f;
		bool myShowEditorUI = true;

		sf::RenderWindow* myRenderWindow = nullptr;
		bool myShouldBeOpen = true;
		bool myDisplayImGUIDemo = false;

		FW_GrowingArray<Dockable*> myDockables;
		int myNextDockableID = 0;

		sf::RenderTarget* myActiveRenderTarget = nullptr;
		sf::RenderTexture* myOffscreenBuffer = nullptr;
	};
}
