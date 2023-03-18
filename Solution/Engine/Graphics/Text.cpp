#include "stdafx.h"
#include "Graphics/Text.h"
#include "Graphics/Window.h"
#include "Graphics/Font.h"
#include "Graphics/SFMLHelpers.h"

#include "Core/Engine.h"

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace Slush
{
	Text::Text()
	{
		myText = new sf::Text();
		myText->setCharacterSize(15);
	}

	Text::~Text()
	{
		FW_SAFE_DELETE(myText);
	}

	void Text::SetText(const char* aText)
	{
		myText->setString(aText);
	}

	void Text::SetFont(const Font& aFont)
	{
		if(const sf::Font* sfmlFont = aFont.GetSFMLFont())
			myText->setFont(*sfmlFont);
		else
			SLUSH_ERROR("Text: Invalid font passed to SetFont")
	}

	void Text::SetColor(int argb)
	{
		myText->setFillColor(SFMLHelpers::GetColor(argb));
	}

	void Text::SetColor(float a, float r, float g, float b)
	{
		SetColor(FW_Float_To_ARGB(a, r, g, b));
	}

	void Text::SetPosition(float x, float y)
	{
		myPosition = { x, y };

		Vector2f alignedPosition = myPosition;

		sf::FloatRect bounds = myText->getGlobalBounds();
		switch (myHorizontalAlignment)
		{
		case HorizontalAlignment::CENTER:
			alignedPosition.x -= bounds.width * 0.5f;
			break;
		case HorizontalAlignment::RIGHT:
			alignedPosition.x -= bounds.width;
			break;
		}

		switch (myVerticalAlignment)
		{
		case VerticalAlignment::CENTER:
			alignedPosition.y -= bounds.height * 0.5f;
			break;
		case VerticalAlignment::BOTTOM:
			alignedPosition.y -= bounds.height;
			break;
		}

		myText->setPosition(alignedPosition.x, alignedPosition.y);
	}

	void Text::SetHorizontalAlignment(HorizontalAlignment anAlignment)
	{
		if (myHorizontalAlignment != anAlignment)
		{
			myHorizontalAlignment = anAlignment;
			SetPosition(myPosition.x, myPosition.y);
		}
	}

	void Text::SetVerticalAlignment(VerticalAlignment anAlignment)
	{
		if (myVerticalAlignment != anAlignment)
		{
			myVerticalAlignment = anAlignment;
			SetPosition(myPosition.x, myPosition.y);
		}
	}

	void Text::Render(float x, float y)
	{
		SetPosition(x, y);
		Engine::GetInstance().GetWindow().GetActiveRenderTarget()->draw(*myText);
	}

	void Text::Render()
	{
		Engine::GetInstance().GetWindow().GetActiveRenderTarget()->draw(*myText);
	}
}