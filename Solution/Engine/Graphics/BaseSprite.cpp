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
		, myTexture(nullptr)
	{
	}

	BaseSprite::~BaseSprite()
	{
		FW_SAFE_DELETE(myShape);
	}

	void BaseSprite::SetTexture(const Texture& aTexture)
	{
		myTexture = &aTexture;

		if (const sf::Texture* sfmlTexture = aTexture.GetSFMLTexture())
			myShape->setTexture(sfmlTexture);
		else
			SLUSH_ERROR("Sprite: Invalid texture passed to SetTexture");
	}

	void BaseSprite::SetTextureRect(int x, int y, int aWidth, int aHeight)
	{
		myTextureRect = MakeRectFromTopLeft<int>({ x, y }, { aWidth, aHeight });
		sf::IntRect rect = { myTextureRect.myTopLeft.x, myTextureRect.myTopLeft.y, myTextureRect.myExtents.x, myTextureRect.myExtents.y };
		if (myEnableHorizontalFlip)
		{
			rect.left += rect.width;
			rect.width = -rect.width;
		}

		myShape->setTextureRect(rect);
	}

	void BaseSprite::SetFillColor(int argb)
	{
		myFillColor = argb;
		myShape->setFillColor(SFMLHelpers::GetColor(argb));
	}

	void BaseSprite::SetFillColor(float a, float r, float g, float b)
	{
		SetFillColor(FW_Float_To_ARGB(a, r, g, b));
	}

	void BaseSprite::SetOutlineColor(int argb)
	{
		myOutlineColor = argb;
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

	void BaseSprite::SetHorizontalFlip(bool aEnableFlip)
	{
		if (myEnableHorizontalFlip == aEnableFlip)
			return;

		myEnableHorizontalFlip = aEnableFlip;
		SetTextureRect(myTextureRect.myTopLeft.x, myTextureRect.myTopLeft.y, myTextureRect.myExtents.x, myTextureRect.myExtents.y);
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

	void BaseSprite::SetScale(float aScale)
	{
		if (myScale != aScale)
		{
			myShape->setScale({ aScale, aScale });
			myScale = aScale;
		}
	}

	float BaseSprite::GetOutlineThickness() const
	{
		return myShape->getOutlineThickness();
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