#pragma once

namespace sf
{
	class RectangleShape;
}

namespace Slush
{
	class Texture;
	class Sprite
	{
	public:
		Sprite();
		~Sprite();

		void SetTexture(const Texture& aTexture);
		void SetColor(int argb);
		void SetColor(float a, float r, float g, float b);
		void SetSize(float aWidth, float aHeight);
		void SetPosition(float x, float y);
		void SetRotation(float aRadians);

		int GetColor() const { return myColor; }
		const Vector2f& GetSize() const { return mySize; }
		const Vector2f& GetPosition() const { return myPosition; }
		float GetRotation() const { return myRotation; }

		void Render();
		void Render(float x, float y);

	private:
		sf::RectangleShape* myShape;
		Vector2f mySize;
		Vector2f myPosition;
		int myColor;
		float myRotation;
	};
}
