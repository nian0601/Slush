#include "stdafx.h"
#include "Graphics/Font.h"
#include "Core/AssetRepository.h"

#include <SFML/Graphics/Font.hpp>

namespace Slush
{
	Font::~Font()
	{
		FW_SAFE_DELETE(mySFMLFont);
	}

	void Font::Load(const char* aFilePath)
	{
		if (!AssetRepository::FindAssetFile(aFilePath, myFilePath))
		{
			SLUSH_ERROR("Font: %s not found in any Asset Repository", aFilePath);
			return;
		}

		mySFMLFont = new sf::Font();
		if (!mySFMLFont->openFromFile(myFilePath.GetBuffer()))
		{
			SLUSH_ERROR("Font: Failed to load %s", aFilePath);
			FW_SAFE_DELETE(mySFMLFont);
			return;
		}

		SLUSH_INFO("Font: '%s' loaded", aFilePath);
	}
}