#include "stdafx.h"


#include "ActionGameGlobals.h"
#include "AnimationComponent.h"
#include "DamageDealerComponent.h"
#include "PhysicsComponent.h"
#include "SpriteComponent.h"
#include "StatsComponent.h"
#include "TargetingComponent.h"
#include "WeaponComponent.h"

#include "EntitySystem/EntityManager.h"

#include <Graphics\BaseSprite.h>
#include <Graphics\Texture.h>

#include <Physics\PhysicsWorld.h>


void WeaponData::OnParse(Slush::AssetParser::Handle aRootHandle)
{
	aRootHandle.ParseFloatField("basecooldown", myBaseCooldown);
	aRootHandle.ParseIntField("basedamage", myBaseDamage);

	Slush::AssetParser::Handle projectileHandle = aRootHandle.ParseChildElement("projectileData");
	projectileHandle.ParseOptionalBoolField("enable", myProjectileData.myEnable, true);
	projectileHandle.ParseOptionalFloatField("baseprojectilespeed", myProjectileData.myBaseProjectileSpeed, myProjectileData.myEnable);
	projectileHandle.ParseOptionalIntField("baseprojectilcount", myProjectileData.myBaseProjectileCount, myProjectileData.myEnable);
	projectileHandle.ParseOptionalStringField("projectilePrefab", myProjectileData.myProjectilePrefab, myProjectileData.myEnable);
}

void WeaponData::BuildUI()
{
	if (ImGui::CollapsingHeader("Base Data", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::SetNextItemWidth(150.f);
		ImGui::InputFloat("Base Cooldown", &myBaseCooldown, 0.05f, 0.1f, "%.2f");

		ImGui::SetNextItemWidth(150.f);
		ImGui::InputInt("Base Damage", &myBaseDamage);
	}

	if (ImGui::CollapsingHeader("Projectile Data"))
	{
		ImGui::SetNextItemWidth(150.f);
		ImGui::Checkbox("Enable", &myProjectileData.myEnable);

		if (myProjectileData.myEnable)
		{
			ImGui::SetNextItemWidth(150.f);
			ImGui::InputFloat("Base Projectile Speed", &myProjectileData.myBaseProjectileSpeed, 1.f, 100.f, "%.2f");

			ImGui::SetNextItemWidth(150.f);
			ImGui::InputInt("Base Projectile Count", &myProjectileData.myBaseProjectileCount);
			myProjectileData.myBaseProjectileCount = FW_Clamp(myProjectileData.myBaseProjectileCount, 1, 20),

			ImGui::SetNextItemWidth(150.f);
			ImGui::InputText("ProjectilePrefab", &myProjectileData.myProjectilePrefab);
			if (ImGui::BeginDragDropTarget())
			{
				if (Slush::Asset* asset = ImGui::AcceptDraggedAsset(Slush::GetAssetID<EntityPrefab>()))
					myProjectileData.myProjectilePrefab = asset->GetAssetName();

				ImGui::EndDragDropTarget();
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////

Weapon::Weapon(Entity& anEntity, const WeaponData& aWeaponData)
	: myEntity(anEntity)
	, myWeaponData(aWeaponData)
{
}

void Weapon::Update()
{
	if (!myActivationCooldown.IsStarted() || myActivationCooldown.HasExpired())
	{
		float cooldown = myWeaponData.myBaseCooldown;
		if (StatsComponent* stats = myEntity.GetComponent<StatsComponent>())
		{
			cooldown /= stats->GetCooldownReduction();
		}

		myActivationCooldown.Start(cooldown);

		if (myWeaponData.myProjectileData.myEnable)
			RunProjectileLogic();
	}
}

void Weapon::RunProjectileLogic()
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

	const WeaponData::ProjectileData& projectileData = myWeaponData.myProjectileData;

	const bool oddNumProjectiles = (projectileData.myBaseProjectileCount % 2) == 1;
	int projectilesToSpawn = projectileData.myBaseProjectileCount;

	Vector2f direction = GetNormalized(target.Get()->myPosition - myEntity.myPosition);

	if (oddNumProjectiles)
	{
		ShootProjectile(direction);
		--projectilesToSpawn;
	}

	for (int i = 0; i < projectilesToSpawn; i += 2)
	{
		float spread = 0.1f * i + 0.1f;
		ShootProjectile(Rotate(direction, FW_PI * spread));
		ShootProjectile(Rotate(direction, -FW_PI * spread));
	}
}

void Weapon::ShootProjectile(const Vector2f& aDirection)
{
	Entity* projectile = myEntity.myEntityManager.CreateEntity(myEntity.myPosition + aDirection * 35.f, myWeaponData.myProjectileData.myProjectilePrefab.GetBuffer());
	projectile->GetComponent<PhysicsComponent>()->myObject->myVelocity = aDirection * myWeaponData.myProjectileData.myBaseProjectileSpeed;
	projectile->GetComponent<SpriteComponent>()->GetSprite().SetRotation(FW_SignedAngle(aDirection));
	if (DamageDealerComponent* projDamage = projectile->GetComponent<DamageDealerComponent>())
	{
		int damage = myWeaponData.myBaseDamage;
		if (StatsComponent* stats = myEntity.GetComponent<StatsComponent>())
			damage = static_cast<int>(damage * stats->GetDamageModifier());

		projDamage->SetDamage(damage);
	}

	if (AnimationComponent* anim = myEntity.GetComponent<AnimationComponent>())
		anim->PlaySpritesheetAnimation();
}

//////////////////////////////////////////////////////////////////////////

void WeaponComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle.ParseStringField("weapondata", myWeaponDataAsset);
}

void WeaponComponent::Data::OnBuildUI()
{
	ImGui::InputText("WeaponData", &myWeaponDataAsset);
	if (ImGui::BeginDragDropTarget())
	{
		if (Slush::Asset* asset = ImGui::AcceptDraggedAsset(Slush::GetAssetID<WeaponData>()))
			myWeaponDataAsset = asset->GetAssetName();

		ImGui::EndDragDropTarget();
	}
}


WeaponComponent::WeaponComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab)
	: Component(anEntity, anEntityPrefab)
{
	Slush::AssetRegistry& assets = Slush::AssetRegistry::GetInstance();
	WeaponData* data = assets.GetAsset<WeaponData>(anEntityPrefab.GetWeaponData().myWeaponDataAsset.GetBuffer());

	myWeapons.Add(new Weapon(myEntity, *data));
}

WeaponComponent::~WeaponComponent()
{
	myWeapons.DeleteAll();
}

void WeaponComponent::Update()
{
	for (Weapon* weapon : myWeapons)
		weapon->Update();
}

void WeaponComponent::UpgradeWeapon(const WeaponData& someData)
{
	myWeapons.Add(new Weapon(myEntity, someData));
}
