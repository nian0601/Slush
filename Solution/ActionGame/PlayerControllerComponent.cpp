#include "PlayerControllerComponent.h"

#include "Entity.h"
#include "AnimationComponent.h"
#include "ProjectileShootingComponent.h"

#include <Core\Engine.h>
#include <Core\Input.h>
#include "HealthComponent.h"
#include <Physics\PhysicsWorld.h>
#include "PhysicsComponent.h"

void PlayerControllerComponent::Update()
{
	if (!myEntity.myAnimationComponent || !myEntity.myAnimationComponent->AnimationIsPlaying())
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

		if (myEntity.myPhysicsComponent)
			myEntity.myPhysicsComponent->myObject->myVelocity = myDirection * mySpeed;

		if (myEntity.myAnimationComponent && input.WasKeyPressed(Slush::Input::SPACE))
			myEntity.myAnimationComponent->PlayDash(myEntity.myPosition + myDirection * 500.f);

		if (myEntity.myProjectileShootingComponent && input.WasMousePressed(Slush::Input::LEFTMB))
		{
			Vector2f toCursor = input.GetMousePositionf() - myEntity.myPosition;
			myEntity.myProjectileShootingComponent->TryShoot(GetNormalized(toCursor));
		}
	}
}