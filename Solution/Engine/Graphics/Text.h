#pragma once
#include <float.h>

class FW_String;
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
		void SetText(const FW_String& aString);
		void SetFont(const Font& aFont);

		void SetCharacterSize(int aSizeInPixesl);

		void SetColor(int argb);
		void SetColor(float a, float r, float g, float b);

		void SetPosition(float x, float y);
		void SetHorizontalAlignment(HorizontalAlignment anAlignment);
		void SetVerticalAlignment(VerticalAlignment anAlignment);
		void SetMaxWidth(float aWidth);

		Vector2f CalculateBounds(const char* aText);

		void Render();
		void Render(float x, float y);

	private:
		sf::Text* myText;
		FW_String myString;
		Vector2f myPosition;
		float myMaxWidth = FLT_MAX;
		HorizontalAlignment myHorizontalAlignment = HorizontalAlignment::LEFT;
		VerticalAlignment myVerticalAlignment = VerticalAlignment::TOP;
	};
}