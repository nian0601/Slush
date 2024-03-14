#pragma once

#include "Component.h"

namespace Slush
{
	class BaseSprite;
}

class SpriteComponent : public Component
{
public:
	SpriteComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab);
	~SpriteComponent();

	void MakeCircle(float aRadius, int aColor);
	void MakeRect(float aWidth, float aHeight, int aColor);

	void Render();

	Slush::BaseSprite& GetSprite() { return *mySprite; }
	const Slush::BaseSprite& GetSprite() const { return *mySprite; }

private:
	Slush::BaseSprite* mySprite = nullptr;
};
