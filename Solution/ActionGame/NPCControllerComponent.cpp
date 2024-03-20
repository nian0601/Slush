#include "NPCControllerComponent.h"

#include "Entity.h"
#include "ProjectileShootingComponent.h"
#include "PhysicsComponent.h"
#include "TargetingComponent.h"

#include <Core\Log.h>
#include <Physics\PhysicsWorld.h>

void NPCControllerComponent::Update()
{
	TargetingComponent* targeting = myEntity.myTargetingComponent;
	if (!targeting)
	{
		SLUSH_ERROR("Entity with 'NPCControllerComponent' is missing a 'TargetingComponent'");
		return;
	}

	EntityHandle target = targeting->GetTarget();
	if (!target.IsValid())
		return;

	Vector2f toTarget = target.Get()->myPosition - myEntity.myPosition;

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
