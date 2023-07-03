#include "BossCard.h"

#include <Graphics/Text.h>
#include <Graphics/RectSprite.h>
#include <FW_Includes.h>
#include <FW_FileParser.h>

BossCard::BossCard(const Slush::Font* aFont)
	: Card(aFont)
{
	myXPText = new Slush::Text();
	myXPText->SetFont(*aFont);
	myXPText->SetText("-");
	myXPText->SetColor(0xFFFFFFFF);
}

BossCard::~BossCard()
{
	FW_SAFE_DELETE(myXPText);
}

void BossCard::OnLoadField(const FW_String& aFieldName, const FW_String& aFieldData, FW_FileParser& aFileParser)
{
	if (aFieldName == "#xp")
	{
		myXPAmount = aFileParser.GetInt(aFieldData);

		myXPString.Clear();
		myXPString += myXPAmount;
		myXPString += " XP";

		myXPText->SetText(myXPString);
	}
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

