#include "stdafx.h"
#include "Graphics/Texture.h"
#include "Core/Engine.h"
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Image.hpp>
#include <FW_FileSystem.h>

namespace Slush
{
	Texture::~Texture()
	{
		FW_SAFE_DELETE(mySFMLTexture);
	}

	void Texture::Load(const char* aFilePath, int aRepositoryIndex)
	{
		Asset::Load(aFilePath, aRepositoryIndex);

		mySFMLTexture = new sf::Texture();
		if (!mySFMLTexture->loadFromFile(myAbsoluteFilePath.GetBuffer()))
		{
			SLUSH_ERROR("Texture: Failed to load %s", aFilePath);
			const sf::Image fallbackImage({ 64, 64 }, sf::Color::Magenta);
			const bool loaded = mySFMLTexture->loadFromImage(fallbackImage);
			FW_ASSERT(loaded, "Generated fallback texture failed to load");
			mySize = { 64, 64 };
			return;
		}

		sf::Vector2u size = mySFMLTexture->getSize();
		mySize.x = size.x;
		mySize.y = size.y;

		SLUSH_INFO("Texture: '%s' loaded", aFilePath);
	}
}
