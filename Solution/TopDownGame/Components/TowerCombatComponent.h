#pragma once

#include <EntitySystem/Component.h>
#include <EntitySystem/EntityHandle.h>
#include <Core/Time.h>

class TowerCombatComponent : public Slush::Component
{
public:
	COMPONENT_HELPER("Tower Combat", "towercombat", 1);

	struct Data : public Slush::Component::BaseData
	{
		void OnParse(Slush::AssetParser::Handle aComponentHandle, unsigned int aVersion) override;
		void OnBuildUI() override;

		float myRange = 300.f;
		float myCooldown = 1.f;
		int myDamage = 10;
	};

	TowerCombatComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab);

	void Update() override;

private:
	bool IsTargetInRange(const Slush::Entity& anEntity) const;
	void AcquireTarget();
	void AttackTarget();

	float myRange = 0.f;
	float myCooldown = 0.f;
	int myDamage = 0;
	Slush::EntityHandle myTarget;
	Slush::Timer myAttackTimer;
};
