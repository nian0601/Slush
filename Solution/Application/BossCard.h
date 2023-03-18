#pragma once

#include "Card.h"

namespace Slush
{
	class Text;
}

class BossCard : public Card
{
public:
	BossCard(const char* aTitle, const char* aDescription, const Slush::Texture* aImageTexture, const Slush::Font* aFont, int aXPAmount);
	~BossCard();

protected:
	void OnSetPosition() override;
	void OnRenderCardDetails() override;

private:
	int myXPAmount;
	FW_String myXPString;
	Slush::Text* myXPText;
};
