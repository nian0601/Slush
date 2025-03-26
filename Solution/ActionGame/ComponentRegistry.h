#pragma once

#include "Component.h"
#include "FW_StaticArray.h"

//class IComponentFactory
//{
//public:
//	virtual Component* CreateComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab) const = 0;
//};
//
//template <typename ComponentType>
//class ComponentFactory : public IComponentFactory
//{
//public:
//	ComponentFactory(const char* aComponentDataName)
//	{
//		myComponentID = GetComponentID<ComponentType>();
//		myComponentDataName = aComponentDataName;
//	}
//
//	Component* CreateComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab) const override
//	{
//		return new ComponentType(anEntity, anEntityPrefab);
//	}
//
//	unsigned int GetID() const
//	{
//		return myComponentID;
//	}
//
//private:
//	unsigned int myComponentID;
//	FW_String myComponentDataName;
//};

class ComponentRegistry
{
public:
	static ComponentRegistry& GetInstance();
	static void Destroy();

	template <typename ComponentType>
	void RegisterComponent(const char* aComponentDataName);

private:
	ComponentRegistry();
	~ComponentRegistry();
	static ComponentRegistry* ourInstance;

	//FW_StaticArray<IComponentFactory*, MAX_COMPONENTS> myComponentFactories;
};

template <typename ComponentType>
inline void ComponentRegistry::RegisterComponent(const char* aComponentDataName)
{
	GetComponentID<ComponentType>();
	aComponentDataName;
	//myComponentFactories[GetComponentID<ComponentType>()] = new ComponentFactory<ComponentType>(aComponentDataName);
}