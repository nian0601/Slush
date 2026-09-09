#include "stdafx.h"

#include "TargetableComponent.h"

TargetableComponent::TargetableComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab)
	: Slush::Component(anEntity, anEntityPrefab)
{
}
