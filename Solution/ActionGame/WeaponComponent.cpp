#include "stdafx.h"

#include "DamageDealerComponent.h"
#include "EntityManager.h"
#include "EntityPrefab.h"
#include "PhysicsComponent.h"
#include "StatsComponent.h"
#include "TargetingComponent.h"
#include "WeaponComponent.h"

#include <Physics\PhysicsWorld.h>

Weapon::Weapon(Entity& anEntity)
	: myEntity(anEntity)
{
}

void Weapon::Update()
{
	if (!myActivationCooldown.IsStarted() || myActivationCooldown.HasExpired())
	{
		float cooldown = myBaseCooldown;
		if (StatsComponent* stats = myEntity.GetComponent<StatsComponent>())
		{
			cooldown /= stats->GetCooldownReduction();
			cooldown /= GetAdditionalCooldownReduction(stats);
		}

		myActivationCooldown.Start(cooldown);

		OnActivate();
	}
}

//////////////////////////////////////////////////////////////////////////

void ProjectileShooter::ShootProjectile(const Vector2f& aDirection)
{
	Entity* projectile = myEntity.myEntityManager.CreateEntity(myEntity.myPosition + aDirection * 35.f, myProjectilePrefab.GetBuffer());
	projectile->GetComponent<PhysicsComponent>()->myObject->myVelocity = aDirection * myBaseProjectileSpeed;

	if (DamageDealerComponent* projDamage = projectile->GetComponent<DamageDealerComponent>())
	{
		int damage = myBaseDamage;
		if (StatsComponent* stats = myEntity.GetComponent<StatsComponent>())
			damage = static_cast<int>(damage * stats->GetDamageModifier());

		projDamage->SetDamage(damage);
	}
}

//////////////////////////////////////////////////////////////////////////

void LineShooter::OnActivate()
{
	TargetingComponent* targeting = myEntity.GetComponent<TargetingComponent>();
	if (!targeting)
	{
		SLUSH_ERROR("Entity with 'WeaponComponent' is missing a 'TargetingComponent'");
		return;
	}

	EntityHandle target = targeting->GetTarget();
	if (!target.IsValid())
		return;

	Vector2f direction = GetNormalized(target.Get()->myPosition - myEntity.myPosition);
	ShootProjectile(direction);
}

float LineShooter::GetAdditionalCooldownReduction(StatsComponent* aStatsComponent)
{
	return static_cast<float>(aStatsComponent->GetAdditionalProjectiles());
}

//////////////////////////////////////////////////////////////////////////

void SpreadShooter::OnActivate()
{
	TargetingComponent* targeting = myEntity.GetComponent<TargetingComponent>();
	if (!targeting)
	{
		SLUSH_ERROR("Entity with 'WeaponComponent' is missing a 'TargetingComponent'");
		return;
	}

	EntityHandle target = targeting->GetTarget();
	if (!target.IsValid())
		return;

	Vector2f direction = GetNormalized(target.Get()->myPosition - myEntity.myPosition);
	ShootProjectile(direction);

	if (StatsComponent* stats = myEntity.GetComponent<StatsComponent>())
	{
		int additionalProj = stats->GetAdditionalProjectiles();
		for (int i = 0; i < additionalProj; ++i)
		{
			float spread = 0.1f * i;
			ShootProjectile(Rotate(direction, FW_PI * spread));
			ShootProjectile(Rotate(direction, -FW_PI * spread));
		}
	}
	
}

//////////////////////////////////////////////////////////////////////////

WeaponComponent::Data::Data()
	: Component::BaseData("Weapon", "weaponcomponent")
{}

void WeaponComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle.ParseFloatField("basecooldown", myBaseCooldown);
	aComponentHandle.ParseFloatField("baseprojectilespeed", myBaseProjectileSpeed);
	aComponentHandle.ParseIntField("basedamage", myBaseDamage);
}

void WeaponComponent::Data::OnBuildUI()
{
	ImGui::SetNextItemWidth(100.f);
	ImGui::InputFloat("Base Cooldown", &myBaseCooldown, 0.05f, 0.1f, "%.2f");

	ImGui::SetNextItemWidth(100.f);
	ImGui::InputFloat("Base Projectile Speed", &myBaseProjectileSpeed, 1.f, 100.f, "%.2f");

	ImGui::SetNextItemWidth(100.f);
	ImGui::InputInt("Base Damage", &myBaseDamage);
}


WeaponComponent::WeaponComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab)
	: Component(anEntity, anEntityPrefab)
{
	const Data& weaponData = anEntityPrefab.GetWeaponData();

	LineShooter* lineShooter = new LineShooter(anEntity);
	lineShooter->myProjectilePrefab = "PlayerProjectile";
	lineShooter->myBaseCooldown = weaponData.myBaseCooldown;
	lineShooter->myBaseProjectileSpeed = weaponData.myBaseProjectileSpeed;
	lineShooter->myBaseDamage = weaponData.myBaseDamage;
	myWeapons.Add(lineShooter);

	SpreadShooter* spreadShooter = new SpreadShooter(anEntity);
	spreadShooter->myProjectilePrefab = "PlayerProjectile";
	spreadShooter->myBaseCooldown = weaponData.myBaseCooldown;
	spreadShooter->myBaseProjectileSpeed = weaponData.myBaseProjectileSpeed;
	spreadShooter->myBaseDamage = weaponData.myBaseDamage;
	myWeapons.Add(spreadShooter);
}

WeaponComponent::~WeaponComponent()
{
	myWeapons.DeleteAll();
}

void WeaponComponent::Update()
{
	for (Weapon* wep : myWeapons)
		wep->Update();
}