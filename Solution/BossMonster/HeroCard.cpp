#include "HeroCard.h"

#include <Graphics/Text.h>
#include <Graphics/RectSprite.h>
#include <FW_Includes.h>
#include <FW_FileParser.h>

HeroCard::HeroCard(const Slush::Font* aFont)
	: Card(aFont)
{
	myHealthText = new Slush::Text(*aFont);
	myHealthText->SetColor(0xFFFF0000);
	myHealthText->SetText("-");

	mySoulText = new Slush::Text(*aFont);
	mySoulText->SetColor(0xFF00FF00);
	mySoulText->SetText("-");
	mySoulText->SetHorizontalAlignment(Slush::Text::HorizontalAlignment::CENTER);

	myWoundText = new Slush::Text(*aFont);
	myWoundText->SetColor(0xFF0000FF);
	myWoundText->SetText("-");
	myWoundText->SetHorizontalAlignment(Slush::Text::HorizontalAlignment::RIGHT);
}

HeroCard::~HeroCard()
{
	FW_SAFE_DELETE(myHealthText);
	FW_SAFE_DELETE(mySoulText);
	FW_SAFE_DELETE(myWoundText);
}

void HeroCard::OnLoadField(const FW_String& aFieldName, const FW_String& aFieldData, FW_FileParser& aFileParser)
{
	if (aFieldName == "#health")
	{
		myHealthValue = aFileParser.GetInt(aFieldData);

		myHealthString.Clear();
		myHealthString += myHealthValue;

		myHealthText->SetText(myHealthString);
	}
	else if (aFieldName == "#soul")
	{
		mySoulValue = aFileParser.GetInt(aFieldData);

		mySoulString.Clear();
		mySoulString += mySoulValue;

		mySoulText->SetText(mySoulString);
	}
	else if (aFieldName == "#wound")
	{
		myWoundValue = aFileParser.GetInt(aFieldData);

		myWoundString.Clear();
		myWoundString += myHealthValue;

		myWoundText->SetText(myWoundString);
	}
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

