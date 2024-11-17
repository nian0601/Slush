#pragma once

#include "FW_String.h"
#include "Core/Assets/Asset.h"

namespace sf
{
	class Texture;
}

namespace Slush
{
	class Texture : public Asset
	{
	public:
		DEFINE_ASSET("Texture", "png", "data/textures");

		using Asset::Asset;
		~Texture();

		void Load(const char* aFilePath) override;

		const sf::Texture* GetSFMLTexture() const { return mySFMLTexture; }
		const Vector2i& GetSize() const { return mySize; }

	private:
		sf::Texture* mySFMLTexture = nullptr;
		Vector2i mySize;
	};
}