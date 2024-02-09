#include "CollisionComponent.h"
#include "Entity.h"
#include <FW_Math.h>

bool CollisionComponent::CollidesWithCircle(const Vector2f& aCenterPosition, float aRadius) const
{
	float distance2 = Length2(myEntity.myPosition - aCenterPosition);
	return distance2 < FW_Square(mySize + aRadius);
}