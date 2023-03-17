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

		void Render(float x, float y);

	private:
		sf::RectangleShape* myShape;
	};
}
