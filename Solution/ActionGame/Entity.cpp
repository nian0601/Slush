#include "Entity.h"

#include <FW_Includes.h>
#include "SpriteComponent.h"
#include "AnimationComponent.h"
#include "ProjectileShootingComponent.h"
#include "PlayerControllerComponent.h"
#include "NPCControllerComponent.h"
#include "HealthComponent.h"
#include "PhysicsComponent.h"
#include "RemoveOnCollisionComponent.h"
#include "TargetingComponent.h"
#include "WeaponComponent.h"
#include "EntityPrefab.h"
#include "ExperienceComponent.h"
#include "PickupComponent.h"
#include "EntityManager.h"

Entity::Entity(EntityManager& aEntityManager)
	: myEntityManager(aEntityManager)
{
}

Entity::~Entity()
{
	FW_SAFE_DELETE(mySpriteComponent);
	FW_SAFE_DELETE(myAnimationComponent);
	FW_SAFE_DELETE(myProjectileShootingComponent);
	FW_SAFE_DELETE(myPlayerControllerComponent);
	FW_SAFE_DELETE(myNPCControllerComponent);
	FW_SAFE_DELETE(myHealthComponent);
	FW_SAFE_DELETE(myPhysicsComponent);
	FW_SAFE_DELETE(myRemoveOnCollisionComponent);
	FW_SAFE_DELETE(myTargetingComponent);
	FW_SAFE_DELETE(myWeaponComponent);
	FW_SAFE_DELETE(myExperienceComponent);
	FW_SAFE_DELETE(myPickupComponent);
}

void Entity::PrePhysicsUpdate()
{
	if (myNPCControllerComponent)
		myNPCControllerComponent->Update();

	if (myPlayerControllerComponent)
		myPlayerControllerComponent->Update();
}

void Entity::Update()
{
	if (myPhysicsComponent)
		myPhysicsComponent->Update();

	if (myAnimationComponent)
		myAnimationComponent->Update();

	if (myTargetingComponent)
		myTargetingComponent->Update();

	if (myWeaponComponent)
		myWeaponComponent->Update();
}

void Entity::Render()
{
	if (mySpriteComponent)
		mySpriteComponent->Render();

	if (myHealthComponent)
		myHealthComponent->Render();

	if (myExperienceComponent)
		myExperienceComponent->Render();
}

void Entity::OnCollision(Entity& aOtherEntity)
{
	if (myHealthComponent)
		myHealthComponent->OnCollision(aOtherEntity);

	if (myRemoveOnCollisionComponent)
		myRemoveOnCollisionComponent->OnCollision(aOtherEntity);

	if (myPickupComponent)
		myPickupComponent->OnCollision(aOtherEntity);
}

void Entity::OnDeath()
{
	if (myNPCControllerComponent)
		myEntityManager.CreateEntity(myPosition, "ExpPickup");
}

void Entity::CreateComponents(const EntityPrefab& aPrefab, Slush::PhysicsWorld& aPhysicsWorld)
{
	if (aPrefab.mySprite.myEnabled)
		mySpriteComponent = new SpriteComponent(*this, aPrefab);

	if (aPrefab.myAnimation.myEnabled)
		myAnimationComponent = new AnimationComponent(*this, aPrefab);

	if (aPrefab.myProjectileShooting.myEnabled)
		myProjectileShootingComponent = new ProjectileShootingComponent(*this, aPrefab);

	if (aPrefab.myHealth.myEnabled)
		myHealthComponent = new HealthComponent(*this, aPrefab);

	if (aPrefab.myPlayerController.myEnabled)
		myPlayerControllerComponent = new PlayerControllerComponent(*this, aPrefab);

	if (aPrefab.myNPCController.myEnabled)
		myNPCControllerComponent = new NPCControllerComponent(*this, aPrefab);

	if (aPrefab.myPhysics.myEnabled)
		myPhysicsComponent = new PhysicsComponent(*this, aPrefab, aPhysicsWorld);

	if (aPrefab.myRemoveOnCollision.myEnabled)
		myRemoveOnCollisionComponent = new RemoveOnCollisionComponent(*this, aPrefab);

	if (aPrefab.myTargeting.myEnabled)
		myTargetingComponent = new TargetingComponent(*this, aPrefab);

	if (aPrefab.myWeaponComponent.myEnabled)
		myWeaponComponent = new WeaponComponent(*this, aPrefab);

	if (aPrefab.myExperience.myEnabled)
		myExperienceComponent = new ExperienceComponent(*this, aPrefab);

	if (aPrefab.myPickup.myEnabled)
		myPickupComponent = new PickupComponent(*this, aPrefab);
}