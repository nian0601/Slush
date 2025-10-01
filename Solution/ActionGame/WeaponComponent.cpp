#include "stdafx.h"

#include "DamageDealerComponent.h"
#include "EntityManager.h"
#include "EntityPrefab.h"
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

Weapon::Weapon(Entity& anEntity)
	: myEntity(anEntity)
{
}

void Weapon::Update()
{
	if (!myActivationCooldown.IsStarted() || myActivationCooldown.HasExpired())
	{
		float cooldown = GetBaseCooldown();
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
	Entity* projectile = myEntity.myEntityManager.CreateEntity(myEntity.myPosition + aDirection * 35.f, myData.myProjectilePrefab.GetBuffer());
	projectile->GetComponent<PhysicsComponent>()->myObject->myVelocity = aDirection * myData.myBaseProjectileSpeed;
	projectile->GetComponent<SpriteComponent>()->GetSprite().SetRotation(FW_SignedAngle(aDirection));
	if (DamageDealerComponent* projDamage = projectile->GetComponent<DamageDealerComponent>())
	{
		int damage = myData.myBaseDamage;
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

float LineShooter::GetAdditionalCooldownReduction(StatsComponent* aStatsComponent) const
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

void WeaponComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	if (aComponentHandle.IsReading())
		myProjectileShooterDatas.Reserve(aComponentHandle.GetNumChildElements());
	
	for (int i = 0; i < myProjectileShooterDatas.Count(); ++i)
	{
		ProjectileShooter::Data& projShooterData = myProjectileShooterDatas[i];
		
		Slush::AssetParser::Handle weaponHandle;
		if (aComponentHandle.IsReading())
			weaponHandle = aComponentHandle.GetChildElementAtIndex(i);
		else
			weaponHandle = aComponentHandle.ParseChildElement("Weapon");

		int typeAsInt = projShooterData.myType;
		weaponHandle.ParseIntField("type", typeAsInt);
		projShooterData.myType = ProjectileShooter::Data::Type(typeAsInt);

		weaponHandle.ParseFloatField("basecooldown", projShooterData.myBaseCooldown);
		weaponHandle.ParseFloatField("baseprojectilespeed", projShooterData.myBaseProjectileSpeed);
		weaponHandle.ParseIntField("basedamage", projShooterData.myBaseDamage);
		weaponHandle.ParseStringField("projectilePrefab", projShooterData.myProjectilePrefab);
	}
}

void WeaponComponent::Data::OnBuildUI()
{
	for (ProjectileShooter::Data& projShooterData : myProjectileShooterDatas)
	{
		const char* shooterType = projShooterData.myType == ProjectileShooter::Data::W_LineShooter ? "LineShooter" : "SpreadShooter";
		if (ImGui::CollapsingHeader(shooterType))
		{
			ImGui::SetNextItemWidth(150.f);
			ImGui::InputFloat("Base Cooldown", &projShooterData.myBaseCooldown, 0.05f, 0.1f, "%.2f");

			ImGui::SetNextItemWidth(150.f);
			ImGui::InputFloat("Base Projectile Speed", &projShooterData.myBaseProjectileSpeed, 1.f, 100.f, "%.2f");

			ImGui::SetNextItemWidth(150.f);
			ImGui::InputInt("Base Damage", &projShooterData.myBaseDamage);

			ImGui::InputText("ProjectilePrefab", &projShooterData.myProjectilePrefab);
			if (ImGui::BeginDragDropTarget())
			{
				ImGuiDragDropFlags target_flags = 0;
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(EntityPrefab::GetAssetTypeName(), target_flags))
				{
					int prefabIndex = *(const int*)payload->Data;
					const EntityPrefab* prefab = static_cast<EntityPrefab*>(ActionGameGlobals::GetInstance().GetEntityPrefabStorage().GetAllAssets()[prefabIndex]);
					projShooterData.myProjectilePrefab = prefab->GetAssetName();
				}
				ImGui::EndDragDropTarget();
			}
		}
	}

	if (ImGui::Button("Add Weapon"))
		ImGui::OpenPopup("add_weapon_popup");

	if (ImGui::BeginPopup("add_weapon_popup"))
	{
		const char* weaponTypes[] = { "Line Shooter", "Spread Shooter" };
		for (int i = 0; i < IM_ARRAYSIZE(weaponTypes); ++i)
		{
			if (ImGui::Selectable(weaponTypes[i]))
			{
				ProjectileShooter::Data& data = myProjectileShooterDatas.Add();
				data.myType = static_cast<ProjectileShooter::Data::Type>(i);
				ImGui::CloseCurrentPopup();
			}
		}

		ImGui::EndPopup();
	}
}


WeaponComponent::WeaponComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab)
	: Component(anEntity, anEntityPrefab)
{
	const Data& weaponData = anEntityPrefab.GetWeaponData();

	for (const ProjectileShooter::Data& projShooterData : weaponData.myProjectileShooterDatas)
	{
		ProjectileShooter* shooter = nullptr;
		switch (projShooterData.myType)
		{
		case ProjectileShooter::Data::W_LineShooter: shooter = new LineShooter(anEntity); break;
		case ProjectileShooter::Data::W_SpreadShooter: shooter = new SpreadShooter(anEntity); break;
		}

		shooter->myData = projShooterData;
		myProjectileShooters.Add(shooter);
	}
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