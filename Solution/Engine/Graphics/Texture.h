#pragma once

#include "FW_String.h"

namespace sf
{
	class Texture;
}

namespace Slush
{
	class Texture
	{
	public:
		~Texture();

		void Load(const char* aFilePath);

		const sf::Texture* GetSFMLTexture() const { return mySFMLTexture; }
		const Vector2i& GetSize() const { return mySize; }
		const FW_String& GetFilePath() const { return myFilePath; }

	private:
		sf::Texture* mySFMLTexture = nullptr;
		Vector2i mySize;
		FW_String myFilePath;
	};
}