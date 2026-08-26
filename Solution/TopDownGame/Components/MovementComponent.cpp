#include "stdafx.h"

#include "MovementComponent.h"

#include <Core/Time.h>
#include <EntitySystem/Entity.h>
#include <imgui/ImGuiWidgets.h>

#include "../Level/Level.h"
#include "TopDownGameGlobals.h"

void MovementComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle, unsigned int /*aVersion*/)
{
	aComponentHandle.ParseFloatField("movespeed", myMoveSpeed);
}

void MovementComponent::Data::OnBuildUI()
{
	Slush::ImGuiWidgets::InputFloat("Move Speed", &myMoveSpeed);
}

MovementComponent::MovementComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab)
	: Slush::Component(anEntity, anEntityPrefab)
{
	myMoveSpeed = anEntityPrefab.GetComponentData<MovementComponent>().myMoveSpeed;
}

void MovementComponent::OnEnterWorld()
{
	Level& level = TopDownGameGlobals::GetInstance().GetLevel();
	Navmesh::PathCorridor corridor;
	const bool foundPath = level.GetNavmesh().FindPath(myEntity.myPosition, level.GetLevelDataAsset().myGoalPosition, myWaypoints, corridor);
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

	if (myNextWaypointIndex == myWaypoints.Count())
	{
		myEntity.myIsMarkedForRemoval = true;
	}
}
