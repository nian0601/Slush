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
	class Renderer
	{
	public:
		Renderer(sf::RenderWindow* aRenderWindow);
		~Renderer();

		void StartOffscreenBuffer();
		void EndOffscreenBuffer();

		sf::RenderTarget* GetActiveRenderTarget() const { return myActiveRenderTarget; }
		sf::RenderTexture* GetOffscreenBuffer() const { return myOffscreenBuffer; }

		void RenderLine(const Vector2i& aStart, const Vector2i& aEnd, int aColor = 0xFFFFFFFF);
		void RenderLine(const Vector2f& aStart, const Vector2f& aEnd, int aColor = 0xFFFFFFFF);
		void RenderTriangle(const Vector2f& aV1, const Vector2f& aV2, const Vector2f& aV3, int aColor = 0xFFFFFFFF);
		void RenderRect(const Rectf& aRect, int aColor = 0xFFFFFFFF, float aRotationInRadians = 0.f);
		void RenderCircle(const Vector2f& aCenter, float aRadius, int aColor = 0xFFFFFFFF);

		void StartFade(float aDuration);
		void RenderFade();

	private:
		sf::RenderWindow* myRenderWindow = nullptr;

		sf::RenderTarget* myActiveRenderTarget = nullptr;
		sf::RenderTexture* myOffscreenBuffer = nullptr;

		sf::CircleShape* myCircleShape = nullptr;
		sf::RectangleShape* myRectShape = nullptr;

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
