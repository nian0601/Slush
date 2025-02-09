#pragma once

#include "Component.h"

class DamageDealerComponent : public Component
{
public:
	DamageDealerComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab);

	void OnCollision(Entity& aOtherEntity) override;

	void SetDamage(int aDamage) { myDamage = aDamage; }

private:
	int myDamage;
};
