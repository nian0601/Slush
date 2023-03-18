#pragma once

#include <FW_String.h>
#include <FW_Vector2.h>
#include <FW_Rect.h>

namespace Slush
{
	class Texture;
	class Sprite;

	class Text;
	class Font;
}

enum TreasureType
{
	CLERIC,
	FIGHTER,
	MAGE,
	THIEF,
};

inline int GetTreasureColor(TreasureType aTreasureType)
{
	switch (aTreasureType)
	{
	case CLERIC: return 0xFFDDDDDD;
	case FIGHTER: return 0xFFFF5555;
	case MAGE: return 0xFF5555FF;
	case THIEF: return 0xFF55FF55;
	default:
		return 0xFF559911;
	}
}

class Card
{
public:
	Card(const char* aTitle, const char* aDescription, const Slush::Texture* aImageTexture, const Slush::Font* aFont);
	virtual ~Card();

	void Render();

	//Vector2i GetCardSize() const { return ToInt(myBackgroundRect.myExtents); }
	//const Rectf& GetCardRect() const { return myBackgroundRect; }

	void SetPosition(int aX, int aY);

protected:
	virtual void OnSetPosition() {};
	virtual void OnRenderCardDetails() {};

	Slush::Sprite* myBackground;
	Slush::Sprite* myTitleBackground;
	Slush::Sprite* myImageBackground;
	Slush::Sprite* myImage;
	Slush::Sprite* myDescriptionBackground;

	Slush::Text* myTitleText;
	Slush::Text* myDescriptionText;

	FW_String myTitle;
	FW_String myDescription;

	float myScale;
};