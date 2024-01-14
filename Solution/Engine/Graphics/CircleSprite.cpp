#include "stdafx.h"

#include "Core/Engine.h"

#include "Graphics/CircleSprite.h"
#include "Graphics/Texture.h"
#include "Graphics/Window.h"
#include "Graphics/SFMLHelpers.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

namespace Slush
{
	CircleSprite::CircleSprite()
		: CircleSprite(20.f)
	{}

	CircleSprite::CircleSprite(float aRadius)
		: BaseSprite(new sf::CircleShape())
	{
		SetRadius(aRadius);
	}

	void CircleSprite::SetRadius(float aRadius)
	{
		myRadius = aRadius;
		static_cast<sf::CircleShape*>(myShape)->setRadius(aRadius);
		myShape->setOrigin({ myRadius, myRadius });
	}
}