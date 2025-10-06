#include "stdafx.h"

#include "AnimationComponent.h"
#include "HealthComponent.h"
#include "PhysicsComponent.h"
#include "PlayerControllerComponent.h"

#include <Core\Engine.h>
#include <Core\Input.h>
#include <Physics\PhysicsWorld.h>

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

		if (anim && input.WasKeyPressed(Slush::Input::SPACE))
			anim->PlayDash(myEntity.myPosition + myDirection * 500.f);

		if (anim && input.WasKeyPressed(Slush::Input::Q))
			anim->PlayBlink();

		if (anim && input.WasKeyPressed(Slush::Input::V))
			anim->PlaySpritesheetAnimation();
	}
}