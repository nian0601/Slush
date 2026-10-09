#include "stdafx.h"
#include "Graphics/Font.h"
#include "Graphics/FallbackFontData.h"
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
			LoadFallback(aFilePath);
			return;
		}

		mySFMLFont = new sf::Font();
		if (!mySFMLFont->openFromFile(myFilePath.GetBuffer()))
		{
			SLUSH_ERROR("Font: Failed to load %s", aFilePath);
			LoadFallback(aFilePath);
			return;
		}

		SLUSH_INFO("Font: '%s' loaded", aFilePath);
	}

	void Font::LoadFallback(const char* aFilePath)
	{
		if (!mySFMLFont)
			mySFMLFont = new sf::Font();

		const bool loaded = mySFMLFont->openFromMemory(FallbackFontData::ourData, FallbackFontData::ourSize);
		FW_ASSERT(loaded, "Embedded fallback font failed to load");
		SLUSH_WARNING("Font: using embedded fallback for %s", aFilePath);
	}
}
