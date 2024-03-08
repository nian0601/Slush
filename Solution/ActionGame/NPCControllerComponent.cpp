#include "NPCControllerComponent.h"

#include "Entity.h"
#include "ProjectileShootingComponent.h"

#include <FW_Assert.h>
#include <Core\Log.h>

void NPCControllerComponent::Update()
{
	if (!myTargetHandle.IsValid())
	{
		SLUSH_WARNING("NPCController is missing a target");
		return;
	}

	if (!myEntity.myProjectileShootingComponent)
		return;

	Vector2f toTarget = myTargetHandle.Get()->myPosition - myEntity.myPosition;
	float distance = Length(toTarget);
	if (distance < myMaxDistance)
		myEntity.myProjectileShootingComponent->TryShoot(GetNormalized(toTarget));
}
