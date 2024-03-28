#include "WeaponComponent.h"
#include "Entity.h"
#include "EntityPrefab.h"
#include "EntityManager.h"
#include "TargetingComponent.h"
#include "PhysicsComponent.h"
#include <Physics\PhysicsWorld.h>
#include "StatsComponent.h"

WeaponComponent::WeaponComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab)
	: Component(anEntity, anEntityPrefab)
{
	Weapon& wep = myWeapons.Add();
	wep.myProjectilePrefab = "PlayerProjectile";
}

void WeaponComponent::Update()
{
	for (Weapon& wep : myWeapons)
		wep.Update(myEntity);
}

void WeaponComponent::Weapon::Update(Entity& anEntity)
{
	if (!myActivationCooldown.IsStarted() || myActivationCooldown.HasExpired())
	{
		float baseCooldown = 0.5f;
		if (StatsComponent* stats = anEntity.myStatsComponent)
			baseCooldown /= stats->GetCooldownReduction();

		myActivationCooldown.Start(baseCooldown);

		TargetingComponent* targeting = anEntity.myTargetingComponent;
		if (!targeting)
		{
			SLUSH_ERROR("Entity with 'WeaponComponent' is missing a 'TargetingComponent'");
			return;
		}

		EntityHandle target = targeting->GetTarget();
		if (!target.IsValid())
			return;

		Vector2f direction = GetNormalized(target.Get()->myPosition - anEntity.myPosition);

		Entity* projectile = anEntity.myEntityManager.CreateEntity(anEntity.myPosition + direction * 35.f, myProjectilePrefab.GetBuffer());
		projectile->myPhysicsComponent->myObject->myVelocity = direction * 1000.f;
	}
}
