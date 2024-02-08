#include "Entity.h"

#include <Graphics\Animation\Animation.h>
#include <Graphics\BaseSprite.h>
#include <FW_Includes.h>
#include <Core\Engine.h>
#include <Core\Input.h>
#include <Core\Time.h>
#include <Graphics\CircleSprite.h>
#include "ProjectileManager.h"

Entity::Entity(bool aIsPlayer)
	: myIsPlayer(aIsPlayer)
{
	mySprite = new Slush::CircleSprite();

	if (myIsPlayer)
	{
		mySprite->SetFillColor(0xFFFF0000);
		mySprite->SetOutlineColor(0xFF440000);
		mySprite->SetOutlineThickness(1.f);

		myAnimation = new Slush::Animation(*mySprite);
		myAnimation->MakeOneShot();
		myAnimation->myScaleTrack
			.Linear(0.1f, 1.f, 0.2f)
			.Wait(0.1f)
			.Linear(0.1f, 0.2f, 1.f);
		myAnimation->myPositionTrack
			.Wait(0.1f)
			.Linear(0.25f, 0.f, 1.f);
	}
	else
	{
		mySprite->SetFillColor(0xFF0000FF);
		mySprite->SetOutlineColor(0xFF440000);
		mySprite->SetOutlineThickness(1.f);

		myShootingCooldown = Slush::Time::ConvertGameTimeToTimeUnit(1.f);
	}
}

Entity::~Entity()
{
	FW_SAFE_DELETE(mySprite);
	FW_SAFE_DELETE(myAnimation);
}

void Entity::UpdateAsEnemy(Entity& aPlayerEntity, ProjectileManager& aProjectileManager)
{
	Vector2f toPlayer = aPlayerEntity.myPosition - myPosition;
	float distance = Length(toPlayer);
	if (distance < 800.f)
		TryShoot(GetNormalized(toPlayer), aProjectileManager);
}

void Entity::UpdateAsPlayer(ProjectileManager& aProjectileManager)
{
	if (myAnimationRuntime.myState != Slush::AnimationRuntime::Running)
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

		if (input.WasKeyPressed(Slush::Input::SPACE))
		{
			myAnimationRuntime.myStartPosition = myPosition;
			myAnimationRuntime.myEndPosition = myPosition + myDirection * 500.f;
			myAnimationRuntime.Start();
		}
		else
		{
			myPosition += myDirection * mySpeed * Slush::Time::GetDelta();
		}

		
		if (input.WasMousePressed(Slush::Input::LEFTMB))
		{
			Vector2f toCursor = input.GetMousePositionf() - myPosition;
			TryShoot(GetNormalized(toCursor), aProjectileManager);
			//TryShoot(myDirection, aProjectileManager);
		}
	}

	myAnimation->Update(myAnimationRuntime);
	if (myAnimationRuntime.myPositionData.myIsActive)
		myPosition = myAnimationRuntime.myCurrentPosition;
}

void Entity::Render()
{
	mySprite->Render(myPosition.x, myPosition.y);
}

void Entity::TryShoot(const Vector2f& aDirection, ProjectileManager& aProjectileManager)
{
	if (Slush::Time::GetCurrentExactTime() < myShootingReadyTimestamp)
		return;

	myShootingReadyTimestamp = Slush::Time::GetCurrentExactTime() + myShootingCooldown;
	aProjectileManager.AddProjectile(myPosition, aDirection);
}
