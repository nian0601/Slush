#include "stdafx.h"
#include "Graphics/Texture.h"
#include "Core/Engine.h"
#include <SFML/Graphics/Texture.hpp>

namespace Slush
{
	Texture::~Texture()
	{
		FW_SAFE_DELETE(mySFMLTexture);
	}

	void Texture::Load(const char* aFilePath)
	{
		myFilePath = Engine::GetInstance().GetDataFolder();
		myFilePath += aFilePath;

		mySFMLTexture = new sf::Texture();
		if (!mySFMLTexture->loadFromFile(myFilePath.GetBuffer()))
		{
			SLUSH_ERROR("Texture: Failed to load %s", aFilePath);
			FW_SAFE_DELETE(mySFMLTexture);
			return;
		}

		sf::Vector2u size = mySFMLTexture->getSize();
		mySize.x = size.x;
		mySize.y = size.y;

		SLUSH_INFO("Texture: '%s' loaded", aFilePath);
	}
}