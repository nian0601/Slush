#include "stdafx.h"

#include "Entity.h"

#include "EntityManager.h"
#include "EntityPrefab.h"

#include <FW_TypeID.h>
#include "ComponentRegistry.h"

namespace Slush
{
	Entity::Entity(EntityManager& aEntityManager)
		: myEntityManager(aEntityManager)
	{
		myComponents.Fill(nullptr);
	}

	Entity::~Entity()
	{
		myComponents.DeleteAll();
	}

	void Entity::OnEnterWorld()
	{
		for (Component* component : myPackedComponents)
			component->OnEnterWorld();
	}

	void Entity::PrePhysicsUpdate()
	{
		for (Component* component : myPackedComponents)
			component->PrePhysicsUpdate();
	}

	void Entity::Update()
	{
		for (Component* component : myPackedComponents)
			component->Update();
	}

	void Entity::Render()
	{
		for (Component* component : myPackedComponents)
			component->Render();
	}

	void Entity::OnCollision(Entity& aOtherEntity, const Vector2f& aContactPosition)
	{
		for (Component* component : myPackedComponents)
			component->OnCollision(aOtherEntity, aContactPosition);
	}

	void Entity::OnDeath()
	{
		for (Component* component : myPackedComponents)
			component->OnDeath();
	}

	void Entity::CreateComponents(const EntityPrefab& aPrefab)
	{
		for (const IComponentFactory* factory : ComponentRegistry::GetInstance().GetFactories())
		{
			if (aPrefab.Has(factory->GetID()))
			{
				int index = factory->GetID();
				myComponents[index] = factory->CreateComponent(*this, aPrefab);
				myPackedComponents.Add(myComponents[index]);
			}
		}
	}
}
