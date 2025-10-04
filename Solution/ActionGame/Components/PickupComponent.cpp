#include "stdafx.h"

#include "PickupComponent.h"
#include "ExperienceComponent.h"

void PickupComponent::OnCollision(Entity& aOtherEntity)
{
	if (aOtherEntity.myType == EntityType::PLAYER)
	{
		myEntity.myIsMarkedForRemoval = true;

		if (ExperienceComponent* exp = aOtherEntity.GetComponent<ExperienceComponent>())
		{
			exp->AddExperience(1);
		}
	}
}