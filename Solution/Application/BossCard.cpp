#include "BossCard.h"

#include <Graphics/Text.h>
#include <Graphics/Sprite.h>
#include <FW_Includes.h>

BossCard::BossCard(const char* aTitle, const char* aDescription, const Slush::Texture* aImageTexture, const Slush::Font* aFont, int aXPAmount)
	: Card(aTitle, aDescription, aImageTexture, aFont)
	, myXPAmount(aXPAmount)
{
	myXPString += myXPAmount;
	myXPString += " XP";

	myXPText = new Slush::Text();
	myXPText->SetFont(*aFont);
	myXPText->SetText(myXPString.GetBuffer());
	myXPText->SetColor(0xFFFFFFFF);
}

BossCard::~BossCard()
{
	FW_SAFE_DELETE(myXPText);
}

void BossCard::OnSetPosition()
{
	Vector2f edgeMargin = { 5.f, 5.f };
	edgeMargin *= myScale;

	Vector2f textPosition = myDescriptionBackground->GetPosition();
	textPosition.y += myDescriptionBackground->GetSize().y + edgeMargin.y;
	myXPText->SetPosition(textPosition.x, textPosition.y);
}

void BossCard::OnRenderCardDetails()
{
	myXPText->Render();
}

