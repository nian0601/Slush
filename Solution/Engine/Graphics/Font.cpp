#include "stdafx.h"
#include "Graphics/Font.h"

#include <SFML/Graphics/Font.hpp>

namespace Slush
{
	Font::~Font()
	{
		FW_SAFE_DELETE(mySFMLFont);
	}

	void Font::Load(const char* aFilePath)
	{
		myFilePath = Engine::GetInstance().GetDataFolder();
		myFilePath += aFilePath;

		mySFMLFont = new sf::Font();
		if (!mySFMLFont->loadFromFile(myFilePath.GetBuffer()))
		{
			SLUSH_ERROR("Font: Failed to load %s", aFilePath);
			FW_SAFE_DELETE(mySFMLFont);
			return;
		}

		SLUSH_INFO("Font: '%s' loaded", aFilePath);
	}
}