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
		enum class HorizontalAlignment
		{
			LEFT,
			CENTER,
			RIGHT
		};

		enum class VerticalAlignment
		{
			TOP,
			CENTER,
			BOTTOM,
		};

		Text();
		~Text();

		void SetText(const char* aText);
		void SetFont(const Font& aFont);

		void SetColor(int argb);
		void SetColor(float a, float r, float g, float b);

		void SetPosition(float x, float y);
		void SetHorizontalAlignment(HorizontalAlignment anAlignment);
		void SetVerticalAlignment(VerticalAlignment anAlignment);

		void Render();
		void Render(float x, float y);

	private:
		sf::Text* myText;
		FW_String myString;
		Vector2f myPosition;
		HorizontalAlignment myHorizontalAlignment = HorizontalAlignment::LEFT;
		VerticalAlignment myVerticalAlignment = VerticalAlignment::TOP;
	};
}