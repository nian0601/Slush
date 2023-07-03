#pragma once

namespace sf
{
	class Shape;
}

namespace Slush
{
	class Texture;
	class BaseSprite
	{
	public:
		virtual ~BaseSprite();

		void SetTexture(const Texture& aTexture);
		void SetColor(int argb);
		void SetColor(float a, float r, float g, float b);
		void SetPosition(float x, float y);
		void SetRotation(float aRadians);

		int GetColor() const { return myColor; }
		const Vector2f& GetPosition() const { return myPosition; }
		float GetRotation() const { return myRotation; }

		void Render();
		void Render(float x, float y);

	protected:
		BaseSprite(sf::Shape* aShape);

		sf::Shape* myShape;
		Vector2f mySize;
		Vector2f myPosition;
		int myColor;
		float myRotation;
	};
}
