#include "stdafx.h"
#include "Graphics/Text.h"
#include "Graphics/Window.h"
#include "Graphics/Font.h"

#include "Core/Engine.h"

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace Slush
{
	Text::Text()
	{
		myText = new sf::Text();
	}

	Text::~Text()
	{
		FW_SAFE_DELETE(myText);
	}

	void Text::Render(float x, float y)
	{
		myText->setPosition({ x, y });
		Engine::GetInstance().GetWindow().GetActiveRenderTarget()->draw(*myText);
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

}