#include "stdafx.h"

#include "MovementComponent.h"

#include <Core/Time.h>
#include <EntitySystem/Entity.h>
#include <imgui/ImGuiWidgets.h>

#include "Level/Level.h"
#include "TopDownGameGlobals.h"

void MovementComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle, unsigned int aVersion)
{
	aComponentHandle.ParseFloatField("movespeed", myMoveSpeed);

	// Version 1 files predate the radius and keep the default until they're next saved.
	if (aVersion >= 2)
		aComponentHandle.ParseFloatField("radius", myRadius);
}

void MovementComponent::Data::OnBuildUI()
{
	Slush::ImGuiWidgets::InputFloat("Move Speed", &myMoveSpeed);

	if (Slush::ImGuiWidgets::InputFloat("Radius", &myRadius))
		myRadius = FW_Max(myRadius, 0.f);
}

MovementComponent::MovementComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab)
	: Slush::Component(anEntity, anEntityPrefab)
{
	const Data& data = anEntityPrefab.GetComponentData<MovementComponent>();
	myMoveSpeed = data.myMoveSpeed;
	myRadius = data.myRadius;
}

void MovementComponent::OnEnterWorld()
{
	Level& level = TopDownGameGlobals::GetInstance().GetLevel();
	const bool foundPath = level.GetNavmesh().FindPath(myEntity.myPosition, level.GetLevelDataAsset().myGoalPosition, myRadius, myWaypoints);
	if (!foundPath)
	{
		SLUSH_ERROR("Enemy '%s' could not find a path from the level start to its goal", myEntityPrefab.GetAssetName().GetBuffer());
		myEntity.myIsMarkedForRemoval = true;
		return;
	}

}

void MovementComponent::Update()
{
	float remainingDistance = myMoveSpeed * Slush::Time::GetDelta();
	while (remainingDistance > 0.f && myNextWaypointIndex < myWaypoints.Count())
	{
		const Vector2f toWaypoint = myWaypoints[myNextWaypointIndex] - myEntity.myPosition;
		const float distanceToWaypoint = Length(toWaypoint);
		if (distanceToWaypoint <= remainingDistance)
		{
			myEntity.myPosition = myWaypoints[myNextWaypointIndex];
			remainingDistance -= distanceToWaypoint;
			++myNextWaypointIndex;
		}
		else
		{
			myEntity.myPosition += GetNormalized(toWaypoint) * remainingDistance;
			break;
		}
	}

	// The removal guard skips enemies OnEnterWorld already discarded for having no path (empty
	// waypoint list, so 0 == 0 here), and stops a reached-goal enemy from counting twice.
	if (myNextWaypointIndex == myWaypoints.Count() && !myEntity.myIsMarkedForRemoval)
	{
		TopDownGameGlobals::GetInstance().GetLevel().OnEnemyReachedGoal(myEntityPrefab);
		myEntity.myIsMarkedForRemoval = true;
	}
}

float MovementComponent::GetRemainingPathDistance() const
{
	if (myNextWaypointIndex >= myWaypoints.Count())
		return 0.f;

	float remainingDistance = Length(myWaypoints[myNextWaypointIndex] - myEntity.myPosition);
	for (int waypointIndex = myNextWaypointIndex + 1; waypointIndex < myWaypoints.Count(); ++waypointIndex)
	{
		remainingDistance += Length(myWaypoints[waypointIndex] - myWaypoints[waypointIndex - 1]);
	}

	return remainingDistance;
}
