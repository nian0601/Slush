#pragma once

#include "Component.h"
#include "FW_StaticArray.h"

class IComponentFactory
{
public:
	virtual Component* CreateComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab) const = 0;
	virtual unsigned int GetID() const = 0;
};

template <typename ComponentType>
class ComponentFactory : public IComponentFactory
{
public:
	ComponentFactory(const char* aComponentDataName)
	{
		myComponentID = GetComponentID<ComponentType>();
		myComponentDataName = aComponentDataName;
	}

	Component* CreateComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab) const override
	{
		return new ComponentType(anEntity, anEntityPrefab);
	}

	unsigned int GetID() const override
	{
		return myComponentID;
	}

private:
	unsigned int myComponentID;
	FW_String myComponentDataName;
};

class ComponentRegistry
{
public:
	static ComponentRegistry& GetInstance();
	static void Destroy();

	template <typename ComponentType>
	void RegisterComponent(const char* aComponentDataName);

	const FW_GrowingArray<IComponentFactory*>& GetFactories() const { return myComponentFactories; }

private:
	ComponentRegistry();
	~ComponentRegistry();
	static ComponentRegistry* ourInstance;

	FW_GrowingArray<IComponentFactory*> myComponentFactories;
	//FW_StaticArray<IComponentFactory*, MAX_COMPONENTS> myComponentFactories;
};

template <typename ComponentType>
inline void ComponentRegistry::RegisterComponent(const char* aComponentDataName)
{
	myComponentFactories.Add(new ComponentFactory<ComponentType>(aComponentDataName));
	//myComponentFactories[GetComponentID<ComponentType>()] = new ComponentFactory<ComponentType>(aComponentDataName);
}