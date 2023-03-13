#pragma once

#include "FW_String.h"

namespace sf
{
	class Font;
}

namespace Slush
{
	class Font
	{
	public:
		~Font();

		void Load(const char* aFilePath);

		const sf::Font* GetSFMLFont() const { return mySFMLFont; }
		const FW_String& GetFilePath() const { return myFilePath; }

	private:
		sf::Font* mySFMLFont = nullptr;
		FW_String myFilePath;
	};
}
