#include "RoomCard.h"

#include <FW_Includes.h>
#include <FW_Math.h>
#include <Graphics/Text.h>
#include <Graphics/RectSprite.h>
#include <FW_FileParser.h>

RoomCard::RoomCard(const Slush::Font* aFont)
	: Card(aFont)
{
	myDamageText = new Slush::Text();
	myDamageText->SetFont(*aFont);
	myDamageText->SetText("-");
	myDamageText->SetColor(0xFFFFFFFF);

	myTreasureSprite = new Slush::RectSprite();
	myTreasureSprite->SetSize(25.f, 25.f);
	myTreasureSprite->SetColor(0xFFFFFFFF);
	myTreasureSprite->SetRotation(FW_DegreesToRadians(45.f));
}

RoomCard::~RoomCard()
{
	FW_SAFE_DELETE(myDamageText);
	FW_SAFE_DELETE(myTreasureSprite);
}

void RoomCard::OnLoadField(const FW_String& aFieldName, const FW_String& aFieldData, FW_FileParser& aFileParser)
{
	if (aFieldName == "#damage")
	{
		myDamageValue = aFileParser.GetInt(aFieldData);

		myDamageString.Clear();
		myDamageString += myDamageValue;

		myDamageText->SetText(myDamageString);
	}
	else if (aFieldName == "#treasure")
	{
		if (aFieldData == "CLERIC")
			myTreasureType = CLERIC;
		else if (aFieldData == "FIGHTER")
			myTreasureType = FIGHTER;
		else if (aFieldData == "MAGE")
			myTreasureType = MAGE;
		else if (aFieldData == "THIEF")
			myTreasureType = THIEF;

		myTreasureSprite->SetColor(GetTreasureColor(myTreasureType));
	}
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
