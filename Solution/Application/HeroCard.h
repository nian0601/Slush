#pragma once

#include "Card.h"

namespace Slush
{
	class Text;
}

class HeroCard : public Card
{
public:
	HeroCard(const char* aTitle, const char* aDescription, const Slush::Texture* aImageTexture, const Slush::Font* aFont, int aHealthValue, int aWoundValue, int aSoulValue);
	~HeroCard();

protected:
	void OnSetPosition() override;
	void OnRenderCardDetails() override;

private:
	int myHealthValue;
	FW_String myHealthString;
	Slush::Text* myHealthText;

	int myWoundValue;
	FW_String myWoundString;
	Slush::Text* myWoundText;

	int mySoulValue;
	FW_String mySoulString;
	Slush::Text* mySoulText;
};
