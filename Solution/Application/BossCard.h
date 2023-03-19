#pragma once

#include "Card.h"

namespace Slush
{
	class Text;
}

class BossCard : public Card
{
public:
	BossCard(const Slush::Font* aFont);
	~BossCard();

protected:
	void OnLoadField(const FW_String& aFieldName, const FW_String& aFieldData, FW_FileParser& aFileParser) override;
	void OnSetPosition() override;
	void OnRenderCardDetails() override;

private:
	int myXPAmount;
	FW_String myXPString;
	Slush::Text* myXPText;
};
