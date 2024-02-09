#pragma once

#include "Component.h"
#include <FW_Vector2.h>

class CollisionComponent : public Component
{
public:
	using Component::Component;

	void SetSize(float aRadius) { mySize = aRadius; }

	bool CollidesWithCircle(const Vector2f& aCenterPosition, float aRadius) const;

private:
	float mySize = 0.f;
};
