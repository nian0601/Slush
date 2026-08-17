#include "stdafx.h"

#include "Core/Engine.h"

#include "Graphics/RectSprite.h"
#include "Graphics/Texture.h"
#include "Graphics/Window.h"
#include "Graphics/Renderer.h"
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

	void RectSprite::SetOrigin(Origin anOrigin)
	{
		if (myOrigin != anOrigin)
		{
			myOrigin = anOrigin;
			SetSize(mySize.x, mySize.y);
		}
	}

	void RectSprite::SetSize(float aWidth, float aHeight)
	{
		mySize = { aWidth, aHeight };
		static_cast<sf::RectangleShape*>(myShape)->setSize({ mySize.x, mySize.y });

		if (myOrigin == Origin::CENTER)
			myShape->setOrigin({ mySize.x * 0.5f, mySize.y * 0.5f });
		else if (myOrigin == Origin::BOTTOM_CENTER)
			myShape->setOrigin({ mySize.x * 0.5f, mySize.y });
		else if (myOrigin == Origin::TOP_LEFT)
			myShape->setOrigin({ 0.f, 0.f });
		else if (myOrigin == Origin::LEFT)
			myShape->setOrigin({ 0.f, mySize.y * 0.5f });
		else
			FW_ASSERT_ALWAYS;
	}

	void RectSprite::SetWidth(float aWidth)
	{
		SetSize(aWidth, mySize.y);
	}

	void RectSprite::SetHeight(float aHeight)
	{
		SetSize(mySize.x, aHeight);
	}

	void RectSprite::Render()
	{
		// myShape's origin (set via SetOrigin()/SetSize()) may not be the rect's center - Renderer::RenderRect
		// always rotates/positions around the rect's center, so account for the origin offset here to match
		// the previous direct-draw-of-myShape behavior for non-CENTER origins.
		sf::Vector2f shapeOrigin = myShape->getOrigin();
		Vector2f originOffset = { shapeOrigin.x, shapeOrigin.y };
		Vector2f centerOffset = Rotate(mySize * 0.5f - originOffset, GetRotation()) * GetScale();

		Rectf rect = MakeRectFromCenter(GetPosition() + centerOffset, mySize * GetScale());

		Engine::GetInstance().GetWindow().GetRenderer().RenderRect(
			rect, GetTexture(), GetFlippedTextureRect(), GetFillColor(), GetOutlineColor(), GetOutlineThickness(), GetRotation());
	}

	void RectSprite::Render(float x, float y)
	{
		SetPosition(x, y);
		Render();
	}
}