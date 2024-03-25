#include "stdafx.h"

#include "PickupComponent.h"
#include "ExperienceComponent.h"
#include "Entity.h"

void PickupComponent::OnCollision(Entity& aOtherEntity)
{
	if (aOtherEntity.myType == Entity::PLAYER)
	{
		myEntity.myIsMarkedForRemoval = true;

		if (ExperienceComponent* exp = aOtherEntity.myExperienceComponent)
		{
			exp->AddExperience(1);
		}
	}
}