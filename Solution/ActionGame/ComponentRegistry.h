#pragma once

#include "Component.h"
#include "FW_StaticArray.h"

class IComponentFactory
{
public:
	virtual Component* CreateComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab) const = 0;
	virtual Component::BaseData* CreateComponentData() const = 0;
	virtual unsigned int GetID() const = 0;
};

template <typename ComponentType, typename ComponentData>
class ComponentFactory : public IComponentFactory
{
public:
	ComponentFactory()
	{
		myComponentID = GetComponentID<ComponentType>();
	}

	Component* CreateComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab) const override
	{
		return new ComponentType(anEntity, anEntityPrefab);
	}

	Component::BaseData* CreateComponentData() const override
	{
		Component::BaseData* data = new ComponentData();
		data->myComponentLabel = ComponentType::GetComponentLabel();
		data->myComponentDataName = ComponentType::GetComponentDataName();
		return data;
	}

	unsigned int GetID() const override
	{
		return myComponentID;
	}

private:
	unsigned int myComponentID;
};

class ComponentRegistry
{
public:
	static ComponentRegistry& GetInstance();
	static void Destroy();

	template <typename ComponentType>
	void RegisterComponent();

	template <typename ComponentType, typename ComponentDataType>
	void RegisterComponent();

	const FW_GrowingArray<IComponentFactory*>& GetFactories() const { return myComponentFactories; }

private:
	ComponentRegistry();
	~ComponentRegistry();
	static ComponentRegistry* ourInstance;

	FW_GrowingArray<IComponentFactory*> myComponentFactories;
};

template <typename ComponentType>
inline void ComponentRegistry::RegisterComponent()
{
	myComponentFactories.Add(new ComponentFactory<ComponentType, Component::BaseData>());
}

template <typename ComponentType, typename ComponentDataType>
inline void ComponentRegistry::RegisterComponent()
{
	myComponentFactories.Add(new ComponentFactory<ComponentType, ComponentDataType>());
}