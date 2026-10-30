#pragma once

#include <EntitySystem/Component.h>
#include <FW_GrowingArray.h>

class MovementComponent : public Slush::Component
{
public:
	COMPONENT_HELPER("Movement", "movement", 2);

	struct Data : public Slush::Component::BaseData
	{
		void OnParse(Slush::AssetParser::Handle aComponentHandle, unsigned int aVersion) override;
		void OnBuildUI() override;

		float myMoveSpeed = 100.f;
		float myRadius = 16.f;
	};

	MovementComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab);

	void OnEnterWorld() override;
	void Update() override;
	float GetRemainingPathDistance() const;
	float GetRadius() const { return myRadius; }

private:
	float myMoveSpeed = 0.f;
	float myRadius = 0.f;
	FW_GrowingArray<Vector2f> myWaypoints;
	int myNextWaypointIndex = 0;
	int myGoalIndex = 0;
};
