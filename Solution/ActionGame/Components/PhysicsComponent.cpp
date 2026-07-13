#include "stdafx.h"

#include "PhysicsComponent.h"

#include "SpriteComponent.h"
#include <Physics\PhysicsWorld.h>
#include "ActionGameGlobals.h"

namespace
{
	const char* ourCollisionFlagNames[] = { "Environment", "Player", "NPC", "Player Projectile", "NPC Projectile", "Pickup" };
	const char* ourCollisionFlagSerializationNames[] = { "environment", "player", "npc", "playerprojectile", "npcprojectile", "pickup" };
	static_assert(IM_ARRAYSIZE(ourCollisionFlagNames) == CollisionFlag::COLLISIONFLAG_COUNT);
	static_assert(IM_ARRAYSIZE(ourCollisionFlagSerializationNames) == CollisionFlag::COLLISIONFLAG_COUNT);
}

void PhysicsComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle.ParseBoolField("isStatic", myStatic);
	aComponentHandle.ParseBoolField("isSensor", mySensor);
	aComponentHandle.ParseBoolField("matchSprite", myMatchSprite);
	aComponentHandle.ParseFloatField("radius", myRadius);

	Slush::AssetParser::Handle sizeHandle = aComponentHandle.ParseChildElement("size");
	if (sizeHandle.IsValid())
	{
		sizeHandle.ParseFloatField("width", mySize.x);
		sizeHandle.ParseFloatField("height", mySize.y);
	}

	aComponentHandle.ParseIntField("collisionflag", myCollisionFlag);

	Slush::AssetParser::Handle collidesWithHandle = aComponentHandle.ParseChildElement("collideswith");
	if (collidesWithHandle.IsValid())
	{
		for (int i = 0; i < CollisionFlag::COLLISIONFLAG_COUNT; ++i)
			collidesWithHandle.ParseBoolField(ourCollisionFlagSerializationNames[i], myCollidesWithFlags[i]);
	}
}

void PhysicsComponent::Data::OnBuildUI()
{
	ImGui::Checkbox("Is Static", &myStatic);
	ImGui::Checkbox("Is Sensor", &mySensor);
	ImGui::Checkbox("Match Sprite", &myMatchSprite);

	ImGui::InputFloat("Radius", &myRadius, 1.f, 10.f, "%.2f");
	ImGui::InputFloat2("Size", &mySize.x, "%.2f");

	ImGui::Combo("Collision Flag", &myCollisionFlag, ourCollisionFlagNames, CollisionFlag::COLLISIONFLAG_COUNT);

	ImGui::Text("Collides With:");
	for (int i = 0; i < CollisionFlag::COLLISIONFLAG_COUNT; ++i)
		ImGui::Checkbox(ourCollisionFlagNames[i], &myCollidesWithFlags[i]);
}

//////////////////////////////////////////////////////////////////////////

PhysicsComponent::PhysicsComponent(Slush::Entity& aEntity, const Slush::EntityPrefab& anEntityPrefab)
	: Slush::Component(aEntity, anEntityPrefab)
	, myPhysicsWorld(ActionGameGlobals::GetInstance().GetPhysicsWorld())
{
	const Data& physData = anEntityPrefab.GetComponentData<PhysicsComponent>();

	Slush::PhysicsShape* shape = nullptr;
	if (physData.myMatchSprite)
	{
		if (anEntityPrefab.Has<SpriteComponent>())
		{
			const SpriteComponent::Data& spriteData = anEntityPrefab.GetComponentData<SpriteComponent>();
			if (spriteData.mySize.x > 0.f)
				shape = new Slush::AABBShape(spriteData.mySize);
		}
		else
		{
			SLUSH_ERROR("Failed to match Physics to Sprite since EntityPrefab doesnt have Sprite enabled, creating a Unit-circle as default");
			shape = new Slush::CircleShape(1.f);
		}
	}
	else
	{
		if (physData.mySize.x > 0.f)
			shape = new Slush::AABBShape(physData.mySize);
		else
			shape = new Slush::CircleShape(physData.myRadius);
	}

	if (shape)
	{
		myObject = new Slush::PhysicsObject(shape);
		myObject->SetPosition(myEntity.myPosition);
		myObject->SetInertia(0.f);
		myObject->myUserData.Set<PhysicsComponent* const>(this);

		myObject->myCollisionMask = 1 << physData.myCollisionFlag;

		unsigned int collidesWithMask = 0;
		for (int i = 0; i < CollisionFlag::COLLISIONFLAG_COUNT; ++i)
		{
			if (physData.myCollidesWithFlags[i])
				collidesWithMask |= 1 << i;
		}
		myObject->myCollidesWithMask = collidesWithMask;
		myObject->myReportCollisionsWith = myObject->myCollidesWithMask;

		myPhysicsWorld.AddObject(myObject);

		if (physData.myStatic)
			myObject->MakeStatic();

		if (physData.mySensor)
			myObject->MakeSensor();
	}
	else
	{
		SLUSH_ERROR("Failed to create PhysicsShape, wont create PhysicsComponent for the entity");
	}
}

PhysicsComponent::~PhysicsComponent()
{
	myPhysicsWorld.RemoveObject(myObject);
	FW_SAFE_DELETE(myObject);
}

void PhysicsComponent::Update()
{	
	myEntity.myPosition = myObject->myPosition;
}