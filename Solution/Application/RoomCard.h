#pragma once

#include "Card.h"

namespace Slush
{
	class Text;
	class Sprite;
}

class RoomCard : public Card
{
public:
	RoomCard(const Slush::Font* aFont);
	~RoomCard();

protected:
	void OnLoadField(const FW_String& aFieldName, const FW_String& aFieldData, FW_FileParser& aFileParser) override;
	void OnSetPosition() override;
	void OnRenderCardDetails() override;

private:
	int myDamageValue;
	FW_String myDamageString;
	Slush::Text* myDamageText;

	TreasureType myTreasureType;
	Slush::Sprite* myTreasureSprite;
};
