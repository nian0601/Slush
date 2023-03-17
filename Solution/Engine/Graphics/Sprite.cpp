#include "stdafx.h"

#include "Core/Engine.h"

#include "Graphics/Sprite.h"
#include "Graphics/Texture.h"
#include "Graphics/Window.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

namespace Slush
{
	sf::Color GetSFMLColor(int aHexColor)
	{
		return{
			unsigned char((aHexColor >> 16) & 255),
			unsigned char((aHexColor >> 8) & 255),
			unsigned char((aHexColor >> 0) & 255),
			unsigned char((aHexColor >> 24) & 255)
		};
	}

	Sprite::Sprite()
	{
		myShape = new sf::RectangleShape();

		float w = 150.f;
		float h = 150.f;
		myShape->setSize({ w, h });
		myShape->setOrigin({ w * 0.5f, h * 0.5f });

		SLUSH_INFO("Sprite created");
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
		myShape->setFillColor(GetSFMLColor(argb));
	}

	void Sprite::SetColor(float a, float r, float g, float b)
	{
		SetColor(FW_Float_To_ARGB(a, r, g, b));
	}

	void Sprite::SetSize(float aWidth, float aHeight)
	{
		myShape->setSize({ aWidth, aHeight });
		myShape->setOrigin({ aWidth * 0.5f, aHeight * 0.5f });
	}

	void Sprite::Render(float x, float y)
	{
		myShape->setPosition({ x, y });
		Engine::GetInstance().GetWindow().GetActiveRenderTarget()->draw(*myShape);
	}

}