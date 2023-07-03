#include "stdafx.h"

#include "Core/Engine.h"

#include "Graphics/BaseSprite.h"
#include "Graphics/Texture.h"
#include "Graphics/Window.h"
#include "Graphics/SFMLHelpers.h"

#include <SFML/Graphics/Shape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

namespace Slush
{
	BaseSprite::BaseSprite(sf::Shape* aShape)
		: myShape(aShape)
	{
	}

	BaseSprite::~BaseSprite()
	{
		FW_SAFE_DELETE(myShape);
	}

	void BaseSprite::SetTexture(const Texture& aTexture)
	{
		if (const sf::Texture* sfmlTexture = aTexture.GetSFMLTexture())
			myShape->setTexture(sfmlTexture);
		else
			SLUSH_ERROR("Sprite: Invalid texture passed to SetTexture");
	}

	void BaseSprite::SetFillColor(int argb)
	{
		myShape->setFillColor(SFMLHelpers::GetColor(argb));
	}

	void BaseSprite::SetFillColor(float a, float r, float g, float b)
	{
		SetFillColor(FW_Float_To_ARGB(a, r, g, b));
	}

	void BaseSprite::SetOutlineColor(int argb)
	{
		myShape->setOutlineColor(SFMLHelpers::GetColor(argb));
	}

	void BaseSprite::SetOutlineColor(float a, float r, float g, float b)
	{
		SetOutlineColor(FW_Float_To_ARGB(a, r, g, b));
	}

	void BaseSprite::SetOutlineThickness(float aThickness)
	{
		myShape->setOutlineThickness(aThickness);
	}

	void BaseSprite::SetPosition(float x, float y)
	{
		myPosition = { x, y };
		myShape->setPosition({ x, y });
	}

	void BaseSprite::SetRotation(float aRadians)
	{
		myRotation = aRadians;
		myShape->setRotation(FW_RadiansToDegrees(aRadians));
	}

	void BaseSprite::Render()
	{
		Engine::GetInstance().GetWindow().GetActiveRenderTarget()->draw(*myShape);
	}

	void BaseSprite::Render(float x, float y)
	{
		SetPosition(x, y);
		Engine::GetInstance().GetWindow().GetActiveRenderTarget()->draw(*myShape);
	}
}