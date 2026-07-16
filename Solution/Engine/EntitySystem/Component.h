#pragma once

#include <FW_TypeID.h>
#include <Core/Assets/AssetParser.h>

namespace Slush
{
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

		virtual void OnEnterWorld() {};
		virtual void PrePhysicsUpdate() {};
		virtual void Update() {};
		virtual void Render() {};

		virtual void OnCollision(Entity& aOtherEntity, const Vector2f& aContactPosition) { aOtherEntity; aContactPosition; };
		virtual void OnDeath() {};

		Entity& myEntity;
		const EntityPrefab& myEntityPrefab;


		struct BaseData
		{
			BaseData()
				: myComponentLabel("Unknown")
				, myComponentDataName("UNKNOWN")
			{}

			virtual ~BaseData() {}

			// Returns true if the on-disk version was behind myCurrentComponentVersion (reading only; always false when writing).
			bool Parse(AssetParser::Handle aRootHandle);
			virtual void OnParse(AssetParser::Handle /*aComponentHandle*/, unsigned int /*aVersion*/) {}

			void BuildUI();
			virtual void OnBuildUI() {};

			bool myEnabled = false;
			const char* myComponentDataName = nullptr; // Used for seriallization, should not have any spaces
			const char* myComponentLabel = nullptr; // Used for UI-display, can be whatever
			unsigned int myCurrentComponentVersion = 1; // Set by ComponentFactory::CreateComponentData() from COMPONENT_HELPER's Version arg
		};
	};

	template <typename ComponentType>
	unsigned int GetComponentID()
	{
		return FW_TypeID<Component>::GetID<ComponentType>();
	}
}

#define MAX_COMPONENTS 32

#define COMPONENT_HELPER(Label, DataName, Version) \
static const char* GetComponentLabel() { return Label; }; \
static const char* GetComponentDataName() { return DataName; }; \
static unsigned int GetCurrentComponentVersion() { return Version; };
