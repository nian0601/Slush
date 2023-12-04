#include "Entity.h"

#include <Graphics\Animation\Animation.h>
#include <Graphics\BaseSprite.h>
#include <FW_Includes.h>

Entity::~Entity()
{
	FW_SAFE_DELETE(mySprite);
	FW_SAFE_DELETE(myAnimation);
}

void Entity::Update()
{
	myAnimation->Update(myAnimationRuntime);
	if (myAnimationRuntime.myPositionData.myIsActive)
		myPosition = myAnimationRuntime.myCurrentPosition;
}

void Entity::Render()
{
	mySprite->Render(myPosition.x, myPosition.y);
}
