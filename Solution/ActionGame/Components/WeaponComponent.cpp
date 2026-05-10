#include "stdafx.h"

#include "DamageDealerComponent.h"
#include "EntitySystem/EntityManager.h"
#include "PhysicsComponent.h"
#include "StatsComponent.h"
#include "TargetingComponent.h"
#include "WeaponComponent.h"
#include "SpriteComponent.h"

#include <Physics\PhysicsWorld.h>
#include <Graphics\BaseSprite.h>
#include "AnimationComponent.h"
#include <Graphics\Texture.h>
#include "ActionGameGlobals.h"


void WeaponData::OnParse(Slush::AssetParser::Handle aRootHandle)
{
	aRootHandle.ParseFloatField("basecooldown", myBaseCooldown);
	aRootHandle.ParseIntField("basedamage", myBaseDamage);

	Slush::AssetParser::Handle projectileHandle = aRootHandle.ParseChildElement("projectileData");
	int typeAsInt = myProjectileData.myType;
	projectileHandle.ParseIntField("type", typeAsInt);
	myProjectileData.myType = WeaponData::ProjectileData::Type(typeAsInt);
	projectileHandle.ParseOptionalFloatField("baseprojectilespeed", myProjectileData.myBaseProjectileSpeed, myProjectileData.myType != WeaponData::ProjectileData::W_None);
	projectileHandle.ParseOptionalStringField("projectilePrefab", myProjectileData.myProjectilePrefab, myProjectileData.myType != WeaponData::ProjectileData::W_None);
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
		const char* projectileTypes[] = { "None", "Line Shooter", "Spread Shooter" };
		if (ImGui::BeginCombo("Projectile Type", projectileTypes[myProjectileData.myType]))
		{
			for (int i = 0; i < IM_ARRAYSIZE(projectileTypes); ++i)
			{
				if (ImGui::Selectable(projectileTypes[i]))
					myProjectileData.myType = static_cast<WeaponData::ProjectileData::Type>(i);
			}

			ImGui::EndCombo();
		}

		if (myProjectileData.myType != WeaponData::ProjectileData::Type::W_None)
		{
			ImGui::SetNextItemWidth(150.f);
			ImGui::InputFloat("Base Projectile Speed", &myProjectileData.myBaseProjectileSpeed, 1.f, 100.f, "%.2f");

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
	, myWeaponData(&aWeaponData)
{
}

void Weapon::Update()
{
	if (!myActivationCooldown.IsStarted() || myActivationCooldown.HasExpired())
	{
		float cooldown = myWeaponData->myBaseCooldown;
		if (StatsComponent* stats = myEntity.GetComponent<StatsComponent>())
		{
			cooldown /= stats->GetCooldownReduction();
		}

		myActivationCooldown.Start(cooldown);

		OnActivate();
	}
}

//////////////////////////////////////////////////////////////////////////

void ProjectileShooter::ShootProjectile(const Vector2f& aDirection)
{
	Entity* projectile = myEntity.myEntityManager.CreateEntity(myEntity.myPosition + aDirection * 35.f, myWeaponData->myProjectileData.myProjectilePrefab.GetBuffer());
	projectile->GetComponent<PhysicsComponent>()->myObject->myVelocity = aDirection * myWeaponData->myProjectileData.myBaseProjectileSpeed;
	projectile->GetComponent<SpriteComponent>()->GetSprite().SetRotation(FW_SignedAngle(aDirection));
	if (DamageDealerComponent* projDamage = projectile->GetComponent<DamageDealerComponent>())
	{
		int damage = myWeaponData->myBaseDamage;
		if (StatsComponent* stats = myEntity.GetComponent<StatsComponent>())
			damage = static_cast<int>(damage * stats->GetDamageModifier());

		projDamage->SetDamage(damage);
	}

	if (AnimationComponent* anim = myEntity.GetComponent<AnimationComponent>())
		anim->PlaySpritesheetAnimation();
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

	ProjectileShooter* shooter = nullptr;
	switch (data->myProjectileData.myType)
	{
	case WeaponData::ProjectileData::W_LineShooter: shooter = new LineShooter(anEntity, *data); break;
	case WeaponData::ProjectileData::W_SpreadShooter: shooter = new SpreadShooter(anEntity, *data); break;
	}
	myProjectileShooters.Add(shooter);
}

WeaponComponent::~WeaponComponent()
{
	myProjectileShooters.DeleteAll();
}

void WeaponComponent::Update()
{
	for (ProjectileShooter* projShooter : myProjectileShooters)
		projShooter->Update();
}