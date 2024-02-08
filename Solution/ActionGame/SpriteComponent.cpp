#include "SpriteComponent.h"

#include "Entity.h"

#include <Graphics/CircleSprite.h>

#include <FW_Includes.h>

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

void SpriteComponent::Render()
{
	FW_ASSERT(mySprite != nullptr, "Tried to render SpriteComponent without a sprite");
	mySprite->Render(myEntity.myPosition.x, myEntity.myPosition.y);
}