#include "Card.h"

#include <Graphics/Sprite.h>
#include <Graphics/Text.h>
#include <FW_Includes.h>

Card::Card(const char* aTitle, const char* aDescription, const Slush::Texture* aImageTexture, const Slush::Font* aFont)
	: myTitle(aTitle)
	, myDescription(aDescription)
{
	myScale = 3.f;

	myBackground = new Slush::Sprite();
	myBackground->SetColor(0xFF222222);
	myBackground->SetSize(100.f * myScale, 150.f * myScale);

	myTitleBackground = new Slush::Sprite();
	myTitleBackground->SetColor(0xFFAAAAAA);
	myTitleBackground->SetSize(90.f * myScale, 20.f * myScale);

	myImageBackground = new Slush::Sprite();
	myImageBackground->SetColor(0xFFAAAAAA);
	myImageBackground->SetSize(90.f * myScale, 60.f * myScale);

	myImage = new Slush::Sprite();
	myImage->SetTexture(*aImageTexture);
	myImage->SetSize(90.f * myScale, 60.f * myScale);

	myDescriptionBackground = new Slush::Sprite();
	myDescriptionBackground->SetColor(0xFFAAAAAA);
	myDescriptionBackground->SetSize(90.f * myScale, 35.f * myScale);

	myTitleText = new Slush::Text();
	myTitleText->SetText(aTitle);
	myTitleText->SetFont(*aFont);
	myTitleText->SetColor(0xFF000000);
	myTitleText->SetHorizontalAlignment(Slush::Text::HorizontalAlignment::CENTER);
	myTitleText->SetVerticalAlignment(Slush::Text::VerticalAlignment::CENTER);

	myDescriptionText = new Slush::Text();
	myDescriptionText->SetText(aDescription);
	myDescriptionText->SetFont(*aFont);
	myDescriptionText->SetColor(0xFF000000);
	myDescriptionText->SetHorizontalAlignment(Slush::Text::HorizontalAlignment::CENTER);
	myDescriptionText->SetVerticalAlignment(Slush::Text::VerticalAlignment::CENTER);

	SetPosition(0, 0);
}

Card::~Card()
{
	FW_SAFE_DELETE(myBackground);
	FW_SAFE_DELETE(myTitleBackground);
	FW_SAFE_DELETE(myImageBackground);
	FW_SAFE_DELETE(myImage);
	FW_SAFE_DELETE(myDescriptionBackground);
	FW_SAFE_DELETE(myTitleText);
	FW_SAFE_DELETE(myDescriptionText);
}

void Card::Render()
{
	myBackground->Render();
	myTitleBackground->Render();

	myTitleText->Render();

	myImageBackground->Render();
	myImage->Render();
	myDescriptionBackground->Render();

	myDescriptionText->Render();

	OnRenderCardDetails();
}

void Card::SetPosition(int aX, int aY)
{
	Vector2f edgeMargin = { 5.f, 5.f };
	edgeMargin *= myScale;

	Vector2f topLeft = { static_cast<float>(aX), static_cast<float>(aY) };
	myBackground->SetPosition(topLeft.x, topLeft.y);

	topLeft += edgeMargin;
	myTitleBackground->SetPosition(topLeft.x, topLeft.y);

	Vector2f titleTextPos = topLeft;
	titleTextPos.x += myTitleBackground->GetSize().x * 0.5f;
	titleTextPos.y += myTitleBackground->GetSize().y * 0.5f;
	myTitleText->SetPosition(titleTextPos.x, titleTextPos.y);

	topLeft.y += myTitleBackground->GetSize().y;
	topLeft.y += edgeMargin.y;

	myImageBackground->SetPosition(topLeft.x, topLeft.y);
	myImage->SetPosition(topLeft.x, topLeft.y);
	

	topLeft.y += myImage->GetSize().y;
	topLeft.y += edgeMargin.y;

	myDescriptionBackground->SetPosition(topLeft.x, topLeft.y);

	Vector2f descriptionTextPos = topLeft;
	descriptionTextPos.x += myDescriptionBackground->GetSize().x * 0.5f;
	descriptionTextPos.y += myDescriptionBackground->GetSize().y * 0.5f;
	myDescriptionText->SetPosition(descriptionTextPos.x, descriptionTextPos.y);

	OnSetPosition();
}
