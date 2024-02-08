#include "Entity.h"

#include <FW_Includes.h>
#include "ProjectileManager.h"
#include "SpriteComponent.h"
#include "AnimationComponent.h"
#include "ProjectileShootingComponent.h"
#include "PlayerControllerComponent.h"
#include "NPCControllerComponent.h"

Entity::~Entity()
{
	FW_SAFE_DELETE(mySpriteComponent);
	FW_SAFE_DELETE(myAnimationComponent);
	FW_SAFE_DELETE(myProjectileShootingComponent);
	FW_SAFE_DELETE(myPlayerControllerComponent);
	FW_SAFE_DELETE(myNPCControllerComponent);
}

void Entity::Update()
{
	if (myNPCControllerComponent)
		myNPCControllerComponent->Update();

	if (myPlayerControllerComponent)
		myPlayerControllerComponent->Update();

	if (myAnimationComponent)
		myAnimationComponent->Update();
}

void Entity::Render()
{
	if (mySpriteComponent)
		mySpriteComponent->Render();
}