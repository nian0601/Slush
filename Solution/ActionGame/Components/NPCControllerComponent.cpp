#include "stdafx.h"

#include "NPCControllerComponent.h"
#include "PhysicsComponent.h"
#include "ProjectileShootingComponent.h"
#include "TargetingComponent.h"

#include <Physics\PhysicsWorld.h>
#include "AnimationComponent.h"
#include "Core\Assets\AssetStorage.h"
#include "Graphics\Animation\Animation.h"

void NPCControllerComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle.ParseFloatField("movementspeed", myMovementSpeed);
	aComponentHandle.ParseFloatField("maxshootingdistance", myMaxShootingDistance);
	aComponentHandle.ParseStringField("spawnanimation", mySpawnAnimationID);

}

void NPCControllerComponent::Data::OnBuildUI()
{
	ImGui::InputFloat("Movement Speed", &myMovementSpeed);
	ImGui::InputFloat("Shooting Distance", &myMaxShootingDistance);
	ImGui::InputText("Spawning Animation", &mySpawnAnimationID);

	if (ImGui::BeginDragDropTarget())
	{
		if (Slush::Asset* asset = ImGui::AcceptDraggedAsset(Slush::GetAssetID<Slush::Animation>()))
			mySpawnAnimationID = asset->GetAssetName();

		ImGui::EndDragDropTarget();
	}
}

//////////////////////////////////////////////////////////////////////////

NPCControllerComponent::NPCControllerComponent(Entity& aEntity, const EntityPrefab& anEntityPrefab)
	: Component(aEntity, anEntityPrefab)
	, myData(static_cast<const Data&>(*anEntityPrefab.GetComponentBaseData<NPCControllerComponent>()))
{
	if (!myData.mySpawnAnimationID.Empty())
	{
		Slush::AssetRegistry& assets = Slush::AssetRegistry::GetInstance();
		mySpawnAnimation = assets.GetAsset<Slush::Animation>(myData.mySpawnAnimationID.GetBuffer());
	}
}

void NPCControllerComponent::OnEnterWorld()
{
	myHasFinishedSpawning = true;

	if (AnimationComponent* anim = myEntity.GetComponent<AnimationComponent>())
	{
		if (mySpawnAnimation)
		{
			Slush::AnimationRuntime* runtime = anim->PlayAnimation(*mySpawnAnimation);
			myHasFinishedSpawning = runtime == nullptr;
		}
	}
}

void NPCControllerComponent::PrePhysicsUpdate()
{
	if (!myHasFinishedSpawning)
		return;

	TargetingComponent* targeting = myEntity.GetComponent<TargetingComponent>();
	if (!targeting)
	{
		SLUSH_ERROR("Entity with 'NPCControllerComponent' is missing a 'TargetingComponent'");
		return;
	}

	EntityHandle target = targeting->GetTarget();
	if (!target.IsValid())
		return;

	Vector2f toTarget = target.Get()->myPosition - myEntity.myPosition;

	if (PhysicsComponent* phys = myEntity.GetComponent<PhysicsComponent>())
	{
		phys->myObject->myVelocity = GetNormalized(toTarget) * myData.myMovementSpeed;
	}

	if (ProjectileShootingComponent* projShooter = myEntity.GetComponent<ProjectileShootingComponent>())
	{
		float distance = Length(toTarget);
		if (distance < myData.myMaxShootingDistance)
			projShooter->TryShoot(GetNormalized(toTarget));
	}
}

void NPCControllerComponent::Update()
{
	if (myHasFinishedSpawning)
		return;

	if (AnimationComponent* anim = myEntity.GetComponent<AnimationComponent>())
	{
		if (mySpawnAnimation)
			myHasFinishedSpawning = !anim->IsAnimationPlaying(*mySpawnAnimation);
	}
}