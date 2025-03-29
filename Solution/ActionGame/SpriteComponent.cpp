#include "stdafx.h"

#include "EntityPrefab.h"
#include "SpriteComponent.h"

#include <Graphics/CircleSprite.h>
#include <Graphics/RectSprite.h>

SpriteComponent::Data::Data()
	: Component::BaseData("Sprite", "sprite")
{}

void SpriteComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle.ParseFloatField("radius", myRadius);
	aComponentHandle.ParseIntField("color", myColor);

	Slush::AssetParser::Handle sizeHandle = aComponentHandle.ParseChildElement("size");
	if (sizeHandle.IsValid())
	{
		sizeHandle.ParseFloatField("width", mySize.x);
		sizeHandle.ParseFloatField("height", mySize.y);
	}

	Slush::AssetParser::Handle colorHandle = aComponentHandle.ParseChildElement("floatColor");
	if (colorHandle.IsValid())
	{
		colorHandle.ParseFloatField("r", myFloatColor[0]);
		colorHandle.ParseFloatField("g", myFloatColor[1]);
		colorHandle.ParseFloatField("b", myFloatColor[2]);
		colorHandle.ParseFloatField("a", myFloatColor[3]);
	}
}

void SpriteComponent::Data::OnBuildUI()
{
	ImGui::ColorEdit4("Color", myFloatColor);
	ImGui::InputFloat("Radius", &myRadius, 1.f, 10.f, "%.2f");
	ImGui::InputFloat2("Size", &mySize.x, "%.2f");

	myColor = FW_Float_To_ARGB(myFloatColor[3], myFloatColor[0], myFloatColor[1], myFloatColor[2]);
}

//////////////////////////////////////////////////////////////////////////

SpriteComponent::SpriteComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab)
	: Component(anEntity, anEntityPrefab)
{
	const SpriteComponent::Data& spriteData = anEntityPrefab.GetSpriteData();
	if (spriteData.mySize.x > 0.f)
		MakeRect(spriteData.mySize.x, spriteData.mySize.y, spriteData.myColor);
	else
		MakeCircle(spriteData.myRadius, spriteData.myColor);
}

SpriteComponent::~SpriteComponent()
{
	FW_SAFE_DELETE(mySprite);
}

void SpriteComponent::MakeCircle(float aRadius, int aColor)
{
	FW_ASSERT(mySprite == nullptr, "Tried to MakeCircle on a SpriteComponent that already has a sprite");

	Slush::CircleSprite* circle = new Slush::CircleSprite(aRadius);
	circle->SetFillColor(aColor);
	circle->SetOutlineColor(0xFF440000);
	circle->SetOutlineThickness(1.f);
	mySprite = circle;
}

void SpriteComponent::MakeRect(float aWidth, float aHeight, int aColor)
{
	Slush::RectSprite* rect = new Slush::RectSprite();
	rect->SetSize(aWidth, aHeight);
	rect->SetFillColor(aColor);
	rect->SetOutlineColor(0xFF440000);
	rect->SetOutlineThickness(1.f);
	mySprite = rect;
}

void SpriteComponent::Render()
{
	FW_ASSERT(mySprite != nullptr, "Tried to render SpriteComponent without a sprite");
	mySprite->Render(myEntity.myPosition.x, myEntity.myPosition.y);
}