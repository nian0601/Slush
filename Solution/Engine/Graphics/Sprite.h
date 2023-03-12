#pragma once

namespace sf
{
	class RectangleShape;
}

namespace Slush
{
	class Sprite
	{
	public:
		Sprite();
		~Sprite();

		void Render(float x, float y);

	private:
		sf::RectangleShape* myShape;
	};
}
