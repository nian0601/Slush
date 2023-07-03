#pragma once

#include <FW_String.h>
#include <FW_Vector2.h>
#include <FW_Rect.h>

class FW_FileParser;

namespace Slush
{
	class Texture;
	class RectSprite;

	class Text;
	class Font;

	template<typename T>
	class AssetStorage;
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
	Card(const Slush::Font* aFont);
	virtual ~Card();

	void Load(const char* aFilePath, const Slush::AssetStorage<Slush::Texture>& someTextureStorage);

	void Render();

	void SetPosition(int aX, int aY);

protected:
	virtual void OnLoadField(const FW_String& aFieldName, const FW_String& aFieldData, FW_FileParser& aFileParser) { aFieldName; aFieldData; aFileParser; }
	virtual void OnSetPosition() {};
	virtual void OnRenderCardDetails() {};

	Slush::RectSprite* myBackground;
	Slush::RectSprite* myTitleBackground;
	Slush::RectSprite* myImageBackground;
	Slush::RectSprite* myImage;
	Slush::RectSprite* myDescriptionBackground;

	Slush::Text* myTitleText;
	Slush::Text* myDescriptionText;

	FW_String myTitle;
	FW_String myDescription;

	float myScale;
};