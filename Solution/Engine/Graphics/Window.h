#pragma once

namespace sf
{
	class RenderWindow;
	class Texture;
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
		void StartFade(float aDuration);
		void RenderFade();

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

		struct FadeData
		{
			sf::Texture* myFadeTexture = nullptr;
			bool myIsFading = false;
			float myRemainingTime = 0.f;
			float myTotalTime = 0.f;
		};
		FadeData myFadeData;
	};
}
