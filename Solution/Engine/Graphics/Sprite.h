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
		void Render(float x, float y);

	private:
		sf::RectangleShape* myShape;
	};
}
