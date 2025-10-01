#pragma once

#include "Core/Dockables/Dockable.h"
#include "Core/Assets/AssetStorage.h"

#include "EntityPrefab.h"

class EntityPrefabDockable : public Slush::Dockable
{
public:
	EntityPrefabDockable(Slush::AssetStorage<EntityPrefab>& aPrefabStorage);
	const char* GetName() const override { return "Entity Prefabs"; }

protected:
	void OnUpdate() override;
	void OnBuildUI() override;

private:
	EntityPrefab* mySelectedPrefab = nullptr;

	Slush::AssetStorage<EntityPrefab>& myPrefabStorage;
	FW_String myNewPrefabNameStorage;
	bool myHasUniquePrefabName = false;
};