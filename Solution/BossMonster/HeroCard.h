#pragma once

#include "Card.h"

namespace Slush
{
	class Text;
}

class HeroCard : public Card
{
public:
	HeroCard(const Slush::Font* aFont);
	~HeroCard();

protected:
	void OnLoadField(const FW_String& aFieldName, const FW_String& aFieldData, FW_FileParser& aFileParser) override;
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
