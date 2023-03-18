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
	RoomCard(const char* aTitle, const char* aDescription, const Slush::Texture* aImageTexture, const Slush::Font* aFont, int aDamageValue, TreasureType aTreasureType);
	~RoomCard();

protected:
	void OnSetPosition() override;
	void OnRenderCardDetails() override;

private:
	int myDamageValue;
	FW_String myDamageString;
	Slush::Text* myDamageText;

	TreasureType myTreasureType;
	Slush::Sprite* myTreasureSprite;
};
