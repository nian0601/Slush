#pragma once

#include "Component.h"
#include <FW_String.h>
#include <FW_FileProcessor.h>
#include <FW_FileParser.h>
#include <Core/Assets/AssetParser.h>
#include <Core/Assets/DataAsset.h>

namespace Slush
{
	class EntityPrefab : public DataAsset
	{
	private:
		struct MissingComponent
		{
			FW_String myLabel;
			bool* myEnabledFlag;
		};

		struct ComponentData
		{
			ComponentData(const char* aUIName, const char* aDataName)
				: myComponentLabel(aUIName)
				, myComponentDataName(aDataName)
			{}

			void Parse(AssetParser::Handle aRootHandle);
			virtual void OnParse(AssetParser::Handle aComponentHandle) { aComponentHandle; }

			void BuildUI();
			virtual void OnBuildUI() {};

			bool myEnabled = false;
			const char* myComponentDataName; // Used for seriallization, should not have any spaces
			const char* myComponentLabel; // Used for UI-display, can be whatever
		};

	public:
		DEFINE_ASSET("Entity Prefab", "prefab", "data/entityprefabs");

		EntityPrefab(const char* aName, unsigned int aAssetID);
		~EntityPrefab();

		void OnParse(AssetParser::Handle aRootHandle) override;

		void BuildUI() override;
		void BuildMissingComponentsUI(const FW_GrowingArray<MissingComponent>& someMissingComponents);

		template <typename ComponentType>
		bool Has() const;

		bool Has(int aComponentID) const;

		template <typename ComponentType>
		const Component::BaseData* GetComponentBaseData() const;

		template <typename ComponentType>
		const ComponentType::Data& GetComponentData() const;

		FW_String myName;

		FW_StaticArray<Component::BaseData*, 32> myComponentBaseDatas;
	};

	template <typename ComponentType>
	bool EntityPrefab::Has() const
	{
		unsigned int id = GetComponentID<ComponentType>();
		return Has(id);
	}

	template <typename ComponentType>
	const Component::BaseData* EntityPrefab::GetComponentBaseData() const
	{
		unsigned int id = GetComponentID<ComponentType>();
		return myComponentBaseDatas[id];
	}

	template <typename ComponentType>
	const ComponentType::Data& EntityPrefab::GetComponentData() const
	{
		const Component::BaseData* data = GetComponentBaseData<ComponentType>();
		return reinterpret_cast<const ComponentType::Data&>(*data);
	}
}
