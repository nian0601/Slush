#include "NPCControllerComponent.h"

#include "Entity.h"
#include "ProjectileShootingComponent.h"

#include <FW_Assert.h>

void NPCControllerComponent::Update()
{
	FW_ASSERT(myTarget != nullptr, "NPCController requires a target");

	if (!myEntity.myProjectileShootingComponent)
		return;

	Vector2f toTarget = myTarget->myPosition - myEntity.myPosition;
	float distance = Length(toTarget);
	if (distance < myMaxDistance)
		myEntity.myProjectileShootingComponent->TryShoot(GetNormalized(toTarget));
}
