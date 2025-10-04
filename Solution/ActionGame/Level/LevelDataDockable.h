#pragma once

#include "Core/Dockables/Dockable.h"
#include "Core/Assets/AssetStorage.h"

#include "LevelData.h"

class LevelDataDockable : public Slush::Dockable
{
public:
	LevelDataDockable();
	const char* GetName() const override { return "LevelData Editor"; }

protected:
	void OnUpdate() override;
	void OnBuildUI() override;

private:
	//EntityPrefab* mySelectedPrefab = nullptr;
	//
	//Slush::AssetStorage<EntityPrefab>& myPrefabStorage;
	//FW_String myNewPrefabNameStorage;
	//bool myHasUniquePrefabName = false;
};