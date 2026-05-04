#include "stdafx.h"

#include "AnimationComponent.h"
#include "HealthComponent.h"
#include "PhysicsComponent.h"
#include "PlayerControllerComponent.h"
#include "SpriteComponent.h"

#include "Core\Engine.h"
#include "Core\Input.h"
#include "Core\Assets\AssetStorage.h"
#include "Physics\PhysicsWorld.h"
#include "Graphics\BaseSprite.h"
#include "Graphics\Animation\Animation.h"
#include "Graphics\Animation\AnimationRuntime.h"
#include "ActionGameGlobals.h"
#include "EntitySystem\EntityManager.h"


void PlayerControllerComponent::OnEnterWorld()
{
	Slush::AssetRegistry& assets = Slush::AssetRegistry::GetInstance();
	myDashAnimation = assets.GetAsset<Slush::Animation>("Dash");
	myBlinkAnimation = assets.GetAsset<Slush::Animation>("Blink");
	mySpriteSheetAnimation = assets.GetAsset<Slush::Animation>("SpriteSheet");
}

void PlayerControllerComponent::PrePhysicsUpdate()
{
	AnimationComponent* anim = myEntity.GetComponent<AnimationComponent>();
	if (!anim || !anim->IsPlayingDash())
	{
		myDirection = { 0.f, 0.f };

		Slush::Engine& engine = Slush::Engine::GetInstance();
		const Slush::Input& input = engine.GetInput();
		if (input.IsKeyDown(Slush::Input::A))
			myDirection.x -= 1.f;
		if (input.IsKeyDown(Slush::Input::D))
			myDirection.x += 1.f;
		if (input.IsKeyDown(Slush::Input::W))
			myDirection.y -= 1.f;
		if (input.IsKeyDown(Slush::Input::S))
			myDirection.y += 1.f;

		Normalize(myDirection);

		if (PhysicsComponent* phys = myEntity.GetComponent<PhysicsComponent>())
			phys->myObject->myVelocity = myDirection * mySpeed;

		if (SpriteComponent* sprite = myEntity.GetComponent<SpriteComponent>())
		{
			if (myDirection.x > 0.f)
				sprite->GetSprite().SetHorizontalFlip(false);
			else if (myDirection.x < 0.f)
				sprite->GetSprite().SetHorizontalFlip(true);
		}

		if (anim && input.WasKeyPressed(Slush::Input::SPACE))
		{
			Slush::AnimationRuntime* animData = anim->PlayAnimation(*myDashAnimation);
			animData->myEndPosition = myEntity.myPosition + myDirection * 500.f;
		}
		
		if (anim && input.WasKeyPressed(Slush::Input::Q))
		{
			//Slush::AnimationRuntime* animData = anim->PlayAnimation(*myBlinkAnimation);
			//animData->myEndColor = 0xFFFF0000;

			ActionGameGlobals::GetInstance().GetEntityManager().CreateEntity(myEntity.myPosition, "Magic_Spawn_Large");
		}
			
		if (anim && input.WasKeyPressed(Slush::Input::V))
			anim->PlayAnimation(*mySpriteSheetAnimation);
	}
}