#include "RoomCard.h"

#include <FW_Includes.h>
#include <FW_Math.h>
#include <Graphics/Text.h>
#include <Graphics/Sprite.h>

RoomCard::RoomCard(const char* aTitle, const char* aDescription, const Slush::Texture* aImageTexture, const Slush::Font* aFont, int aDamageValue, TreasureType aTreasureType)
	: Card(aTitle, aDescription, aImageTexture, aFont)
	, myDamageValue(aDamageValue)
	, myTreasureType(aTreasureType)
{
	myDamageString += myDamageValue;

	myDamageText = new Slush::Text();
	myDamageText->SetFont(*aFont);
	myDamageText->SetText(myDamageString.GetBuffer());
	myDamageText->SetColor(0xFFFFFFFF);

	myTreasureSprite = new Slush::Sprite();
	myTreasureSprite->SetSize(25.f, 25.f);
	myTreasureSprite->SetColor(GetTreasureColor(myTreasureType));
	myTreasureSprite->SetRotation(FW_DegreesToRadians(45.f));
}

RoomCard::~RoomCard()
{
	FW_SAFE_DELETE(myDamageText);
	FW_SAFE_DELETE(myTreasureSprite);
}

void RoomCard::OnSetPosition()
{
	Vector2f edgeMargin = { 5.f, 5.f };
	edgeMargin *= myScale;

	Vector2f textPosition = myDescriptionBackground->GetPosition();
	textPosition.y += myDescriptionBackground->GetSize().y + edgeMargin.y;
	myDamageText->SetPosition(textPosition.x, textPosition.y);

	Vector2f treasurePosition = myDescriptionBackground->GetPosition() + myDescriptionBackground->GetSize();
	treasurePosition.x -= myTreasureSprite->GetSize().x * 0.5f;
	treasurePosition.y += edgeMargin.y * 0.5f;
	myTreasureSprite->SetPosition(treasurePosition.x, treasurePosition.y);
}

void RoomCard::OnRenderCardDetails()
{
	myDamageText->Render();
	myTreasureSprite->Render();
}
