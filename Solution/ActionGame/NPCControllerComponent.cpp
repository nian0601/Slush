#include "stdafx.h"

#include "NPCControllerComponent.h"
#include "PhysicsComponent.h"
#include "ProjectileShootingComponent.h"
#include "TargetingComponent.h"

#include <Physics\PhysicsWorld.h>

void NPCControllerComponent::PrePhysicsUpdate()
{
	TargetingComponent* targeting = myEntity.GetComponent<TargetingComponent>();
	if (!targeting)
	{
		SLUSH_ERROR("Entity with 'NPCControllerComponent' is missing a 'TargetingComponent'");
		return;
	}

	EntityHandle target = targeting->GetTarget();
	if (!target.IsValid())
		return;

	Vector2f toTarget = target.Get()->myPosition - myEntity.myPosition;

	if (PhysicsComponent* phys = myEntity.GetComponent<PhysicsComponent>())
	{
		phys->myObject->myVelocity = GetNormalized(toTarget) * 100.f;
	}

	if (ProjectileShootingComponent* projShooter = myEntity.GetComponent<ProjectileShootingComponent>())
	{
		float distance = Length(toTarget);
		if (distance < myMaxDistance)
			projShooter->TryShoot(GetNormalized(toTarget));
	}
}
