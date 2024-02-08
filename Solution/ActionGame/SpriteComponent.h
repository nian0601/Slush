#pragma once

#include "Component.h"

namespace Slush
{
	class BaseSprite;
}

class SpriteComponent : public Component
{
public:
	using Component::Component;
	~SpriteComponent();

	void MakeCircle(float aRadius, int aColor);

	void Render();

	Slush::BaseSprite& GetSprite() { return *mySprite; }
	const Slush::BaseSprite& GetSprite() const { return *mySprite; }

private:
	Slush::BaseSprite* mySprite = nullptr;
};
