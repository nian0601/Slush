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
		void SetPosition(float x, float y);
		void SetRotation(float aRadians);

		void SetFillColor(int argb);
		void SetFillColor(float a, float r, float g, float b);
		void SetOutlineColor(int argb);
		void SetOutlineColor(float a, float r, float g, float b);
		void SetOutlineThickness(float aThickness);

		const Vector2f& GetPosition() const { return myPosition; }
		float GetRotation() const { return myRotation; }

		void Render();
		void Render(float x, float y);

	protected:
		BaseSprite(sf::Shape* aShape);

		sf::Shape* myShape;
		Vector2f mySize;
		Vector2f myPosition;
		float myRotation = 0.f;
	};
}
