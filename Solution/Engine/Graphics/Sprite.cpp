#include "stdafx.h"

#include "Core/Engine.h"

#include "Graphics/Sprite.h"
#include "Graphics/Texture.h"
#include "Graphics/Window.h"
#include "Graphics/SFMLHelpers.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

namespace Slush
{
	Sprite::Sprite()
	{
		myShape = new sf::RectangleShape();

		float w = 150.f;
		float h = 150.f;
		SetSize(h, w);
	}

	Sprite::~Sprite()
	{
		FW_SAFE_DELETE(myShape);
	}

	void Sprite::SetTexture(const Texture& aTexture)
	{
		if (const sf::Texture* sfmlTexture = aTexture.GetSFMLTexture())
			myShape->setTexture(sfmlTexture);
		else
			SLUSH_ERROR("Sprite: Invalid texture passed to SetTexture");
	}

	void Sprite::SetColor(int argb)
	{
		myColor = argb;
		myShape->setFillColor(SFMLHelpers::GetColor(argb));
	}

	void Sprite::SetColor(float a, float r, float g, float b)
	{
		SetColor(FW_Float_To_ARGB(a, r, g, b));
	}

	void Sprite::SetSize(float aWidth, float aHeight)
	{
		mySize = { aWidth, aHeight };
		myShape->setSize({ aWidth, aHeight });
		//myShape->setOrigin({ aWidth * 0.5f, aHeight * 0.5f });
	}

	void Sprite::SetPosition(float x, float y)
	{
		myPosition = { x, y };
		myShape->setPosition({ x, y });
	}

	void Sprite::SetRotation(float aRadians)
	{
		myRotation = aRadians;
		myShape->setRotation(FW_RadiansToDegrees(aRadians));
	}

	void Sprite::Render()
	{
		Engine::GetInstance().GetWindow().GetActiveRenderTarget()->draw(*myShape);
	}

	void Sprite::Render(float x, float y)
	{
		SetPosition(x, y);
		Engine::GetInstance().GetWindow().GetActiveRenderTarget()->draw(*myShape);
	}
}