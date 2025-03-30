#pragma once

#include "EntityComponentEvents.h"
#include <FW_TypeID.h>
#include <Core/Assets/AssetParser.h>

class Entity;
class EntityPrefab;

class Component
{
public:
	Component(Entity& anEntity, const EntityPrefab& anEntityPrefab)
		: myEntity(anEntity)
		, myEntityPrefab(anEntityPrefab)
	{}
	virtual ~Component() {}

	virtual void PrePhysicsUpdate() {};
	virtual void Update() {};
	virtual void Render() {};

	virtual void OnCollision(Entity& aOtherEntity) { aOtherEntity; };
	virtual void OnDeath() {};

	Entity& myEntity;
	const EntityPrefab& myEntityPrefab;


	struct BaseData
	{
		BaseData()
			: myComponentLabel("Unknown")
			, myComponentDataName("UNKNOWN")
		{}

		void Parse(Slush::AssetParser::Handle aRootHandle);
		virtual void OnParse(Slush::AssetParser::Handle aComponentHandle) { aComponentHandle; }
		
		void BuildUI();
		virtual void OnBuildUI() {};

		bool myEnabled = false;
		const char* myComponentDataName = nullptr; // Used for seriallization, should not have any spaces
		const char* myComponentLabel = nullptr; // Used for UI-display, can be whatever
	};
};

template <typename ComponentType>
unsigned int GetComponentID()
{
	return FW_TypeID<Component>::GetID<ComponentType>();
}

#define MAX_COMPONENTS 32

#define COMPONENT_HELPER(Label, DataName) \
static const char* GetComponentLabel() { return Label; }; \
static const char* GetComponentDataName() { return DataName; };