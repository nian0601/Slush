#include "stdafx.h"

#include "Core/Engine.h"

#include "Graphics/CircleSprite.h"
#include "Graphics/Texture.h"
#include "Graphics/Window.h"
#include "Graphics/Renderer.h"
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

	void CircleSprite::Render()
	{
		// myShape's origin is always the circle's own center (set in SetRadius()), so unlike RectSprite
		// there's no origin offset to account for - GetPosition() is already the world-space center.
		Engine::GetInstance().GetWindow().GetRenderer().RenderCircle(
			GetPosition(), myRadius * GetScale(), GetTexture(), GetFlippedTextureRect(), GetFillColor(), GetOutlineColor(), GetOutlineThickness());
	}

	void CircleSprite::Render(float x, float y)
	{
		SetPosition(x, y);
		Render();
	}
}