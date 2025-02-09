#include "stdafx.h"

#include "EntityPrefab.h"
#include "SpriteComponent.h"

#include <Graphics/CircleSprite.h>
#include <Graphics/RectSprite.h>

SpriteComponent::SpriteComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab)
	: Component(anEntity, anEntityPrefab)
{
	const EntityPrefab::Sprite& spriteData = anEntityPrefab.GetSpriteData();
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