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
		Texture(const char* anAssetName);
		~Texture();

		void Load(const char* aFilePath, bool aIsAbsolutePath = false);

		const sf::Texture* GetSFMLTexture() const { return mySFMLTexture; }
		const Vector2i& GetSize() const { return mySize; }

		const FW_String& GetAssetName() const { return myAssetName; }
		const FW_String& GetFilePath() const { return myFilePath; }
		const FW_String& GetAbsoluteFilePath() const { return myAbsoluteFilePath; }

	private:
		sf::Texture* mySFMLTexture = nullptr;
		Vector2i mySize;

		FW_String myAssetName;
		FW_String myFilePath;
		FW_String myAbsoluteFilePath;
	};
}