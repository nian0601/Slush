#include "stdafx.h"

#include "PickupComponent.h"
#include "ExperienceComponent.h"
#include "HealthComponent.h"
#include "EntitySystem/Components/PhysicsComponent.h"
#include "CollisionFlag.h"
#include <EntitySystem\EntityPrefab.h>
#include "WeaponComponent.h"
#include <imgui\ImGuiWidgets.h>

void PickupComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle, unsigned int /*aVersion*/)
{
	aComponentHandle.ParseIntField("type", myType);
	aComponentHandle.ParseIntField("value", myValue);
}

void PickupComponent::Data::OnBuildUI()
{
	const char* pickupTypes[] = { "Experience", "Health", "Weapon Upgrade"};
	Slush::ImGuiWidgets::Combo("Type", &myType, pickupTypes, IM_ARRAYSIZE(pickupTypes));
	Slush::ImGuiWidgets::InputInt("Value", &myValue);
}

//////////////////////////////////////////////////////////////////////////

PickupComponent::PickupComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab)
	: Slush::Component(anEntity, anEntityPrefab)
	, myData(*static_cast<const PickupComponent::Data*>(anEntityPrefab.GetComponentBaseData<PickupComponent>()))
{
}

void PickupComponent::OnCollision(Slush::Entity& aOtherEntity, const Vector2f& /*aContactPosition*/)
{
	Slush::PhysicsComponent* otherPhysics = aOtherEntity.GetComponent<Slush::PhysicsComponent>();
	if (otherPhysics && otherPhysics->GetCollisionFlag() == CollisionUtils::COL_PLAYER)
	{
		myEntity.myIsMarkedForRemoval = true;

		if (myData.myType == Data::PickupType::EXPERIENCE)
		{
			if (ExperienceComponent* exp = aOtherEntity.GetComponent<ExperienceComponent>())
				exp->AddExperience(myData.myValue);
		}
		else if (myData.myType == Data::PickupType::HEALTH)
		{
			if (HealthComponent* health = aOtherEntity.GetComponent<HealthComponent>())
				health->RestoreHealth(myData.myValue);
		}
		else if (myData.myType == Data::PickupType::WEAPON_UPGRADE)
		{
			if (WeaponComponent* weapon = aOtherEntity.GetComponent<WeaponComponent>())
				weapon->AddPendingUpgrade();
		}
		else
		{
			FW_ASSERT_ALWAYS("Unhandled PickupType");
		}
	}
}