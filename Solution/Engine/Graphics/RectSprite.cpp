#include "stdafx.h"

#include "Core/Engine.h"

#include "Graphics/RectSprite.h"
#include "Graphics/Texture.h"
#include "Graphics/Window.h"
#include "Graphics/SFMLHelpers.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

namespace Slush
{
	RectSprite::RectSprite()
		: BaseSprite(new sf::RectangleShape())
	{
		float w = 150.f;
		float h = 150.f;
		SetSize(h, w);
	}

	void RectSprite::SetSize(float aWidth, float aHeight)
	{
		mySize = { aWidth, aHeight };
		static_cast<sf::RectangleShape*>(myShape)->setSize({ mySize.x, mySize.y });
	}

	void RectSprite::SetWidth(float aWidth)
	{
		mySize.x = aWidth;
		static_cast<sf::RectangleShape*>(myShape)->setSize({ mySize.x, mySize.y });
	}

	void RectSprite::SetHeight(float aHeight)
	{
		mySize.y = aHeight;
		static_cast<sf::RectangleShape*>(myShape)->setSize({ mySize.x, mySize.y });
	}

}