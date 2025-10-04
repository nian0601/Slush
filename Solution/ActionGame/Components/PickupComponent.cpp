#include "stdafx.h"

#include "PickupComponent.h"
#include "ExperienceComponent.h"
#include <EntitySystem\EntityPrefab.h>

void PickupComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle.ParseIntField("type", myType);
	aComponentHandle.ParseIntField("value", myValue);
}

void PickupComponent::Data::OnBuildUI()
{
	const char* pickupTypes[] = { "Experience", "Health"};
	ImGui::Combo("Type", &myType, pickupTypes, IM_ARRAYSIZE(pickupTypes));
	ImGui::InputInt("Value", &myValue);
}

//////////////////////////////////////////////////////////////////////////

PickupComponent::PickupComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab)
	: Component(anEntity, anEntityPrefab)
	, myData(*static_cast<const PickupComponent::Data*>(anEntityPrefab.GetComponentBaseData<PickupComponent>()))
{
}

void PickupComponent::OnCollision(Entity& aOtherEntity)
{
	if (aOtherEntity.myType == EntityType::PLAYER)
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
		else
		{
			FW_ASSERT_ALWAYS("Unhandled PickupType");
		}
	}
}