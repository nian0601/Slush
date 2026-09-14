#pragma once

#include <EntitySystem/Component.h>
#include <EntitySystem/EntityHandle.h>

class ProjectileComponent : public Slush::Component
{
public:
	COMPONENT_HELPER("Projectile", "projectile", 1);

	struct Data : public Slush::Component::BaseData
	{
		void OnParse(Slush::AssetParser::Handle aComponentHandle, unsigned int aVersion) override;
		void OnBuildUI() override;

		float mySpeed = 500.f;
		int myDamage = 10;
		float mySplashRadius = 0.f;
	};

	ProjectileComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab);

	void Update() override;
	void SetTarget(const Slush::EntityHandle& aTarget);

private:
	float mySpeed = 0.f;
	int myDamage = 0;
	float mySplashRadius = 0.f;
	Slush::EntityHandle myTarget;
};
