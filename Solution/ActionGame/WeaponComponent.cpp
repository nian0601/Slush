#include "stdafx.h"

#include "DamageDealerComponent.h"
#include "EntityManager.h"
#include "EntityPrefab.h"
#include "PhysicsComponent.h"
#include "StatsComponent.h"
#include "TargetingComponent.h"
#include "WeaponComponent.h"

#include <Physics\PhysicsWorld.h>

WeaponComponent::WeaponComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab)
	: Component(anEntity, anEntityPrefab)
{
	Weapon& wep = myWeapons.Add();
	wep.myProjectilePrefab = "PlayerProjectile";
	wep.myBaseCooldown = myEntityPrefab.myWeaponComponent.myBaseCooldown;
	wep.myBaseProjectileSpeed = myEntityPrefab.myWeaponComponent.myBaseProjectileSpeed;
	wep.myBaseDamage = myEntityPrefab.myWeaponComponent.myBaseDamage;
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
		float cooldown = myBaseCooldown;
		StatsComponent* stats = anEntity.myStatsComponent;
		if (stats)
			cooldown /= stats->GetCooldownReduction();

		myActivationCooldown.Start(cooldown);

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
		projectile->myPhysicsComponent->myObject->myVelocity = direction * myBaseProjectileSpeed;

		if (DamageDealerComponent* projDamage = projectile->myDamageDealerComponent)
		{
			int damage = myBaseDamage;
			if (stats)
				damage = static_cast<int>(damage * stats->GetDamageModifier());

			projDamage->SetDamage(damage);
		}
	}
}
