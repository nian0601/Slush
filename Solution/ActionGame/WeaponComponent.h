#pragma once

#include "Component.h"

#include <Core\Time.h>
#include <FW_String.h>
#include <FW_GrowingArray.h>


class WeaponComponent : public Component
{
public:
	WeaponComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab);

	void Update();

private:
	struct Weapon
	{
		void Update(Entity& anEntity);

		Slush::Timer myActivationCooldown;
		FW_String myProjectilePrefab;
	};
	FW_GrowingArray<Weapon> myWeapons;
};