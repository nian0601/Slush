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
}

void Entity::Render()
{
	if (mySpriteComponent)
		mySpriteComponent->Render();

	if (myHealthComponent)
		myHealthComponent->Render();
}

void Entity::OnCollision(Entity& aOtherEntity)
{
	if (myHealthComponent)
		myHealthComponent->OnCollision(aOtherEntity);

	if (myRemoveOnCollisionComponent)
		myRemoveOnCollisionComponent->OnCollision(aOtherEntity);
}
