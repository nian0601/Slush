#include "HeroCard.h"

#include <Graphics/Text.h>
#include <Graphics/Sprite.h>
#include <FW_Includes.h>

HeroCard::HeroCard(const char* aTitle, const char* aDescription, const Slush::Texture* aImageTexture, const Slush::Font* aFont, int aHealthValue, int aWoundValue, int aSoulValue)
	: Card(aTitle, aDescription, aImageTexture, aFont)
	, myHealthValue(aHealthValue)
	, myWoundValue(aWoundValue)
	, mySoulValue(aSoulValue)
{
	myHealthString += myHealthValue;
	myWoundString += myWoundValue;
	mySoulString += mySoulValue;

	myHealthText = new Slush::Text();
	myHealthText->SetFont(*aFont);
	myHealthText->SetColor(0xFFFF0000);
	myHealthText->SetText(myHealthString.GetBuffer());

	mySoulText = new Slush::Text();
	mySoulText->SetFont(*aFont);
	mySoulText->SetColor(0xFF00FF00);
	mySoulText->SetText(mySoulString.GetBuffer());

	myWoundText = new Slush::Text();
	myWoundText->SetFont(*aFont);
	myWoundText->SetColor(0xFF0000FF);
	myWoundText->SetText(myWoundString.GetBuffer());
}

HeroCard::~HeroCard()
{
	FW_SAFE_DELETE(myHealthText);
	FW_SAFE_DELETE(mySoulText);
	FW_SAFE_DELETE(myWoundText);
}

void HeroCard::OnSetPosition()
{
	Vector2f edgeMargin = { 5.f, 5.f };
	edgeMargin *= myScale;
	
	Vector2f textPosition;
	textPosition.x = myDescriptionBackground->GetPosition().x;
	textPosition.y = myDescriptionBackground->GetPosition().y + myDescriptionBackground->GetSize().y + edgeMargin.y;
	myHealthText->SetPosition(textPosition.x, textPosition.y);
	
	textPosition.x += myDescriptionBackground->GetSize().x * 0.5f;
	mySoulText->SetPosition(textPosition.x, textPosition.y);
	
	textPosition.x += myDescriptionBackground->GetSize().x * 0.5f;
	myWoundText->SetPosition(textPosition.x, textPosition.y);
}

void HeroCard::OnRenderCardDetails()
{
	myHealthText->Render();
	mySoulText->Render();
	myWoundText->Render();
}

