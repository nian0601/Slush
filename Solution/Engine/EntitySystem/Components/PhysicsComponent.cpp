#include "stdafx.h"

#include "PhysicsComponent.h"

#include "SpriteComponent.h"
#include "EntitySystem/Entity.h"
#include "EntitySystem/EntityManager.h"
#include "EntitySystem/EntityPrefab.h"
#include <Physics/PhysicsWorld.h>

namespace Slush
{
	void PhysicsComponent::Data::OnParse(AssetParser::Handle aComponentHandle)
	{
		aComponentHandle.ParseBoolField("isStatic", myStatic);
		aComponentHandle.ParseBoolField("isSensor", mySensor);
		aComponentHandle.ParseBoolField("matchSprite", myMatchSprite);
		aComponentHandle.ParseFloatField("radius", myRadius);

		AssetParser::Handle sizeHandle = aComponentHandle.ParseChildElement("size");
		if (sizeHandle.IsValid())
		{
			sizeHandle.ParseFloatField("width", mySize.x);
			sizeHandle.ParseFloatField("height", mySize.y);
		}

		OnParseCollisionFlags(aComponentHandle);
	}

	void PhysicsComponent::Data::OnBuildUI()
	{
		ImGui::Checkbox("Is Static", &myStatic);
		ImGui::Checkbox("Is Sensor", &mySensor);
		ImGui::Checkbox("Match Sprite", &myMatchSprite);

		ImGui::InputFloat("Radius", &myRadius, 1.f, 10.f, "%.2f");
		ImGui::InputFloat2("Size", &mySize.x, "%.2f");

		OnBuildCollisionFlagsUI();
	}

	//////////////////////////////////////////////////////////////////////////

	PhysicsComponent::PhysicsComponent(Entity& aEntity, const EntityPrefab& anEntityPrefab)
		: Component(aEntity, anEntityPrefab)
		, myPhysicsWorld(aEntity.myEntityManager.GetPhysicsWorld())
	{
		const Data& physData = anEntityPrefab.GetComponentData<PhysicsComponent>();

		PhysicsShape* shape = nullptr;
		if (physData.myMatchSprite)
		{
			if (anEntityPrefab.Has<SpriteComponent>())
			{
				const SpriteComponent::Data& spriteData = anEntityPrefab.GetComponentData<SpriteComponent>();
				if (spriteData.mySize.x > 0.f)
					shape = new AABBShape(spriteData.mySize);
			}
			else
			{
				SLUSH_ERROR("Failed to match Physics to Sprite since EntityPrefab doesnt have Sprite enabled, creating a Unit-circle as default");
				shape = new CircleShape(1.f);
			}
		}
		else
		{
			if (physData.mySize.x > 0.f)
				shape = new AABBShape(physData.mySize);
			else
				shape = new CircleShape(physData.myRadius);
		}

		if (shape)
		{
			myObject = new PhysicsObject(shape);
			myObject->SetPosition(myEntity.myPosition);
			myObject->SetInertia(0.f);
			myObject->myUserData.Set<PhysicsComponent* const>(this);

			myObject->myCollisionMask = 1 << physData.myCollisionFlag;
			myObject->myCollidesWithMask = physData.myCollidesWithMask;
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

	int PhysicsComponent::GetCollisionFlag() const
	{
		return myEntityPrefab.GetComponentData<PhysicsComponent>().myCollisionFlag;
	}

	void PhysicsComponent::Update()
	{
		myEntity.myPosition = myObject->myPosition;
	}
}
