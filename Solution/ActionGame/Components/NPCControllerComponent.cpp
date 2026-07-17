#include "stdafx.h"

#include "NPCControllerComponent.h"
#include "ProjectileShootingComponent.h"
#include "TargetingComponent.h"

#include <Physics\PhysicsWorld.h>
#include "Graphics\Animation\Animation.h"
#include "CharacterAnimationComponent.h"
#include "Graphics\BaseSprite.h"
#include "EntitySystem\Components\AnimationComponent.h"
#include "EntitySystem\Components\PhysicsComponent.h"
#include "EntitySystem\Components\SpriteComponent.h"

void NPCControllerComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle, unsigned int /*aVersion*/)
{
	aComponentHandle.ParseFloatField("movementspeed", myMovementSpeed);
	aComponentHandle.ParseFloatField("maxshootingdistance", myMaxShootingDistance);
	mySpawnAnimationID.Parse(aComponentHandle, "spawnanimation");
}

void NPCControllerComponent::Data::ResolveDependencies()
{
	mySpawnAnimationID.ResolveDependency();
}

void NPCControllerComponent::Data::OnBuildUI()
{
	ImGui::InputFloat("Movement Speed", &myMovementSpeed);
	ImGui::InputFloat("Shooting Distance", &myMaxShootingDistance);
	ImGui::Text("Spawning Animation: %s", mySpawnAnimationID.GetName().GetBuffer());

	if (ImGui::BeginDragDropTarget())
	{
		if (Slush::Asset* asset = ImGui::AcceptDraggedAsset(Slush::GetAssetID<Slush::Animation>()))
			mySpawnAnimationID.Set(static_cast<Slush::Animation*>(asset));

		ImGui::EndDragDropTarget();
	}
}

//////////////////////////////////////////////////////////////////////////

NPCControllerComponent::NPCControllerComponent(Slush::Entity& aEntity, const Slush::EntityPrefab& anEntityPrefab)
	: Slush::Component(aEntity, anEntityPrefab)
	, myData(static_cast<const Data&>(*anEntityPrefab.GetComponentBaseData<NPCControllerComponent>()))
{
	mySpawnAnimation = myData.mySpawnAnimationID.Get();
}

void NPCControllerComponent::OnEnterWorld()
{
	myHasFinishedSpawning = true;

	if (Slush::AnimationComponent* anim = myEntity.GetComponent<Slush::AnimationComponent>())
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

	Slush::EntityHandle target = targeting->GetTarget();
	if (!target.IsValid())
		return;

	CharacterAnimationComponent* characterAnimation = myEntity.GetComponent<CharacterAnimationComponent>();

	Vector2f toTarget = target.Get()->myPosition - myEntity.myPosition;

	if (Slush::PhysicsComponent* phys = myEntity.GetComponent<Slush::PhysicsComponent>())
	{
		Vector2f direction = GetNormalized(toTarget);
		phys->myObject->myVelocity = direction * myData.myMovementSpeed;

		if (characterAnimation)
			characterAnimation->PlayMovementAnimation();

		if (Slush::SpriteComponent* sprite = myEntity.GetComponent<Slush::SpriteComponent>())
		{
			if (direction.x > 0.f)
				sprite->GetSprite().SetHorizontalFlip(false);
			else if (direction.x < 0.f)
				sprite->GetSprite().SetHorizontalFlip(true);
		}
	}

	if (ProjectileShootingComponent* projShooter = myEntity.GetComponent<ProjectileShootingComponent>())
	{
		float distance = Length(toTarget);
		if (distance > myData.myMaxShootingDistance)
			return;

		bool success = projShooter->TryShoot(GetNormalized(toTarget));
		if (!success)
			return;

		if (characterAnimation)
			characterAnimation->PlayAttackAnimation();
	}
}

void NPCControllerComponent::Update()
{
	if (myHasFinishedSpawning)
		return;

	if (Slush::AnimationComponent* anim = myEntity.GetComponent<Slush::AnimationComponent>())
	{
		if (mySpawnAnimation)
			myHasFinishedSpawning = !anim->IsAnimationPlaying(*mySpawnAnimation);
	}
}