#include "Card.h"

#include <Graphics/RectSprite.h>
#include <Graphics/Text.h>

#include <FW_Includes.h>
#include <FW_FileParser.h>
#include <Core/AssetStorage.h>

Card::Card(const Slush::Font* aFont)
{
	myScale = 3.f;

	myBackground = new Slush::RectSprite();
	myBackground->SetFillColor(0xFF222222);
	myBackground->SetSize(100.f * myScale, 150.f * myScale);

	myTitleBackground = new Slush::RectSprite();
	myTitleBackground->SetFillColor(0xFFAAAAAA);
	myTitleBackground->SetSize(90.f * myScale, 20.f * myScale);

	myImageBackground = new Slush::RectSprite();
	myImageBackground->SetFillColor(0xFFAAAAAA);
	myImageBackground->SetSize(90.f * myScale, 60.f * myScale);

	myImage = new Slush::RectSprite();
	myImage->SetSize(90.f * myScale, 60.f * myScale);

	myDescriptionBackground = new Slush::RectSprite();
	myDescriptionBackground->SetFillColor(0xFFAAAAAA);
	myDescriptionBackground->SetSize(90.f * myScale, 35.f * myScale);

	myTitleText = new Slush::Text();
	myTitleText->SetText("Unknown");
	myTitleText->SetFont(*aFont);
	myTitleText->SetColor(0xFF000000);
	myTitleText->SetHorizontalAlignment(Slush::Text::HorizontalAlignment::CENTER);
	myTitleText->SetVerticalAlignment(Slush::Text::VerticalAlignment::CENTER);
	myTitleText->SetMaxWidth(myTitleBackground->GetSize().x);

	myDescriptionText = new Slush::Text();
	myDescriptionText->SetText("Unknown");
	myDescriptionText->SetFont(*aFont);
	myDescriptionText->SetColor(0xFF000000);
	myDescriptionText->SetHorizontalAlignment(Slush::Text::HorizontalAlignment::CENTER);
	myDescriptionText->SetVerticalAlignment(Slush::Text::VerticalAlignment::CENTER);
	myDescriptionText->SetMaxWidth(myDescriptionBackground->GetSize().x);

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

void Card::Load(const char* aFilePath, const Slush::AssetStorage<Slush::Texture>& someTextureStorage)
{
	FW_FileParser parser(aFilePath);

	FW_String line;
	FW_String fieldName;
	while (parser.ReadLine(line))
	{
		parser.TrimBeginAndEnd(line);
		fieldName = parser.TakeFirstWord(line);

		if (fieldName == "#version")
		{

		}
		else if (fieldName == "#texture")
		{
			if(const Slush::Texture* texture = someTextureStorage.GetAsset(line.GetBuffer()))
				myImage->SetTexture(*texture);
		}
		else if (fieldName == "#title")
		{
			myTitle = line;
			myTitleText->SetText(myTitle);
		}
		else if (fieldName == "#description")
		{
			myDescription = line;
			myDescriptionText->SetText(myDescription);
		}
		else
		{
			OnLoadField(fieldName, line, parser);
		}
	}
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
