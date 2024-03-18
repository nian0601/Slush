#include "NPCControllerComponent.h"

#include "Entity.h"
#include "ProjectileShootingComponent.h"

#include <FW_Assert.h>
#include <Core\Log.h>
#include "PhysicsComponent.h"
#include <Physics\PhysicsWorld.h>

void NPCControllerComponent::Update()
{
	if (!myTargetHandle.IsValid())
	{
		SLUSH_WARNING("NPCController is missing a target");
		return;
	}

	Vector2f toTarget = myTargetHandle.Get()->myPosition - myEntity.myPosition;

	if (PhysicsComponent* phys = myEntity.myPhysicsComponent)
	{
		phys->myObject->myVelocity = GetNormalized(toTarget) * 100.f;
	}

	if (ProjectileShootingComponent* projShooter = myEntity.myProjectileShootingComponent)
	{
		float distance = Length(toTarget);
		if (distance < myMaxDistance)
			projShooter->TryShoot(GetNormalized(toTarget));
	}
}
