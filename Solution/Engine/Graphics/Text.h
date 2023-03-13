#pragma once

namespace sf
{
	class Text;
}

namespace Slush
{
	class Font;
	class Text
	{
	public:
		Text();
		~Text();

		void Render(float x, float y);

		void SetText(const char* aText);
		void SetFont(const Font& aFont);

	private:
		sf::Text* myText;
		FW_String myString;
	};
}