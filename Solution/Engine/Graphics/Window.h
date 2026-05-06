#pragma once

namespace sf
{
	class RenderWindow;
	class RenderTexture;
	class RenderTarget;

	class CircleShape;
	class RectangleShape;
}

namespace Slush
{
	class Dockable;
	class Window
	{
	public:
		Window(unsigned int aWidth, unsigned int aHeight);
		~Window();

		bool PumpEvents();
		void RenderOffscreenBufferToImGUI();

		void Present();

		void Close() { myShouldBeOpen = false; }

		void StartOffscreenBuffer();
		void EndOffscreenBuffer();

		void ToggleEditorUI() { myShowEditorUI = !myShowEditorUI; }

		void AddDockable(Dockable* aDockable);
		void DeleteAllDockables();

		void SetAppLayout(const char* aName);

		sf::RenderWindow* GetRenderWindow() const { return myRenderWindow; }
		sf::RenderTexture* GetOffscreenBuffer() const { return myOffscreenBuffer; }

		sf::RenderTarget* GetActiveRenderTarget() const { return myActiveRenderTarget; }

		void RenderLine(const Vector2i& aStart, const Vector2i& aEnd, int aColor = 0xFFFFFFFF);
		void RenderLine(const Vector2f& aStart, const Vector2f& aEnd, int aColor = 0xFFFFFFFF);
		void RenderRect(const Rectf& aRect, int aColor = 0xFFFFFFFF, float aRotationInRadians = 0.f);
		void RenderCircle(const Vector2f& aCenter, float aRadius, int aColor = 0xFFFFFFFF);

		Rectf GetWindowRect() const { return myWindowRect; }
		Rectf GetGameViewRect() const { return myGameViewRect; }

	private:
		void SaveAppLayoutConfig();
		void LoadAppLayoutConfig();

		Vector2f GetSizeThatRespectsAspectRatio(int aWidth, int aHeight) const;

		Rectf myWindowRect;
		Rectf myGameViewRect;
		float myAspectRatio = 16.f / 9.f;
		bool myShowEditorUI = true;

		sf::RenderWindow* myRenderWindow = nullptr;
		bool myShouldBeOpen = true;
		bool myDisplayImGUIDemo = false;
		const char* myAppLayoutName = "Default Layout";

		FW_GrowingArray<Dockable*> myDockables;
		int myNextDockableID = 0;

		sf::RenderTarget* myActiveRenderTarget = nullptr;
		sf::RenderTexture* myOffscreenBuffer = nullptr;
		sf::CircleShape* myCircleShape = nullptr;
		sf::RectangleShape* myRectShape = nullptr;
	};
}
