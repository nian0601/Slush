#pragma once

namespace sf
{
	class RenderWindow;
	class RenderTexture;
	class RenderTarget;
	class Texture;
	class CircleShape;
	class RectangleShape;
}

namespace Slush
{
	class IAppLayout;
	class Dockable;
	class Window
	{
	public:
		Window(unsigned int aWidth, unsigned int aHeight);
		~Window();

		bool PumpEvents();
		void RenderOffscreenBufferToImGUI();
		void StartFade(float aDuration);
		void RenderFade();

		void Present();

		void Close() { myShouldBeOpen = false; }

		void StartOffscreenBuffer();
		void EndOffscreenBuffer();

		void ToggleEditorUI() { myShowEditorUI = !myShowEditorUI; }

		void SetAppLayout(IAppLayout* aLayout);

		void UpdateAppLayout();
		void RenderAppLayout();

		sf::RenderWindow* GetRenderWindow() const { return myRenderWindow; }
		sf::RenderTexture* GetOffscreenBuffer() const { return myOffscreenBuffer; }

		sf::RenderTarget* GetActiveRenderTarget() const { return myActiveRenderTarget; }

		void RenderLine(const Vector2i& aStart, const Vector2i& aEnd, int aColor = 0xFFFFFFFF);
		void RenderLine(const Vector2f& aStart, const Vector2f& aEnd, int aColor = 0xFFFFFFFF);
		void RenderTriangle(const Vector2f& aV1, const Vector2f& aV2, const Vector2f& aV3, int aColor = 0xFFFFFFFF);
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
		IAppLayout* myAppLayout = nullptr;

		sf::RenderTarget* myActiveRenderTarget = nullptr;
		sf::RenderTexture* myOffscreenBuffer = nullptr;

		struct FadeData
		{
			sf::Texture* myFadeTexture = nullptr;
			bool myIsFading = false;
			float myRemainingTime = 0.f;
			float myTotalTime = 0.f;
		};
		FadeData myFadeData;

		sf::CircleShape* myCircleShape = nullptr;
		sf::RectangleShape* myRectShape = nullptr;
	};
}
