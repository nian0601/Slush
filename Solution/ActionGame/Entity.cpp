#include "Entity.h"

#include <FW_Includes.h>
#include "ProjectileManager.h"
#include "SpriteComponent.h"
#include "AnimationComponent.h"
#include "ProjectileShootingComponent.h"
#include "PlayerControllerComponent.h"
#include "NPCControllerComponent.h"
#include "CollisionComponent.h"
#include "HealthComponent.h"
#include "PhysicsComponent.h"

Entity::~Entity()
{
	FW_SAFE_DELETE(mySpriteComponent);
	FW_SAFE_DELETE(myAnimationComponent);
	FW_SAFE_DELETE(myProjectileShootingComponent);
	FW_SAFE_DELETE(myPlayerControllerComponent);
	FW_SAFE_DELETE(myNPCControllerComponent);
	FW_SAFE_DELETE(myCollisionComponent);
	FW_SAFE_DELETE(myHealthComponent);
	FW_SAFE_DELETE(myPhysicsComponent);
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
}

void Entity::Render()
{
	if (mySpriteComponent)
		mySpriteComponent->Render();

	if (myHealthComponent)
		myHealthComponent->Render();
}