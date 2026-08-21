#pragma once

namespace sf
{
	class RenderWindow;
	class RenderTexture;
	class RenderTarget;
	class Texture;
	class CircleShape;
	class RectangleShape;
	class Text;
}

namespace Slush
{
	class Texture;
	class Font;

	class Renderer
	{
	public:
		Renderer(sf::RenderWindow* aRenderWindow);
		~Renderer();

		void StartOffscreenBuffer();
		void EndOffscreenBuffer();

		sf::RenderTarget* GetActiveRenderTarget() const { return myActiveRenderTarget; }
		sf::RenderTexture* GetOffscreenBuffer() const { return myOffscreenBuffer; }
		Vector2f GetOffscreenBufferSize() const;

		void RenderLine(const Vector2i& aStart, const Vector2i& aEnd, int aColor = 0xFFFFFFFF);
		void RenderLine(const Vector2f& aStart, const Vector2f& aEnd, int aColor = 0xFFFFFFFF);
		void RenderTriangle(const Vector2f& aV1, const Vector2f& aV2, const Vector2f& aV3, int aColor = 0xFFFFFFFF);
		void RenderRect(const Rectf& aRect, int aColor = 0xFFFFFFFF, float aRotationInRadians = 0.f);
		void RenderRect(const Rectf& aRect, const Texture* aTexture, const Recti& aTextureRect, int aFillColor, int aOutlineColor, float aOutlineThickness, float aRotationInRadians = 0.f);
		void RenderCircle(const Vector2f& aCenter, float aRadius, int aColor = 0xFFFFFFFF);
		void RenderCircle(const Vector2f& aCenter, float aRadius, const Texture* aTexture, const Recti& aTextureRect, int aFillColor, int aOutlineColor, float aOutlineThickness);

		void RenderText(const Font& aFont, const FW_String& aString, const Vector2f& aPosition, int aCharacterSize, int aColor);

		void StartFade(float aDuration);
		void UpdateFade();
		void RenderFade();
		void FinalizeOffscreenBuffer();

		void ProcessRenderQueue();

	private:
		struct RenderCommand
		{
			enum class Type
			{
				Line,
				Triangle,
				Rect,
				Circle,
				Sprite,
				Text
			};

			Type myType = Type::Line;

			Vector2f myPoint1;
			Vector2f myPoint2;
			Vector2f myPoint3;

			Rectf myRect;
			float myRotationInRadians = 0.f;

			Vector2f myCenter;
			float myRadius = 0.f;

			int myColor = 0xFFFFFFFF;

			const Texture* myTexture = nullptr;
			Recti myTextureRect;
			int myOutlineColor = 0xFFFFFFFF;
			float myOutlineThickness = 0.f;

			const Font* myFont = nullptr;
			FW_String myString;
			int myCharacterSize = 0;
		};

		struct TargetQueue
		{
			sf::RenderTarget* myTarget = nullptr;

			// RenderCommand::myString (used by Type::Text) relies on FW_GrowingArray's
			// safe-mode (element-wise copy-assignment) semantics rather than memcpy --
			// a memcpy of an FW_String would leave two commands sharing one buffer
			// pointer, corrupting/use-after-freeing it on RemoveAll()/growth. Never flip
			// this array to fast/memcpy mode.
			FW_GrowingArray<RenderCommand> myCommands;
		};

		TargetQueue& GetOrCreateTargetQueue(sf::RenderTarget* aTarget);

		sf::RenderWindow* myRenderWindow = nullptr;

		sf::RenderTarget* myActiveRenderTarget = nullptr;
		sf::RenderTexture* myOffscreenBuffer = nullptr;

		sf::CircleShape* myCircleShape = nullptr;
		sf::RectangleShape* myRectShape = nullptr;
		sf::Text* myTextShape = nullptr;

		FW_GrowingArray<TargetQueue> myTargetQueues;

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
