#pragma once

#include "Core/Dockables/Dockable.h"
#include "Core/Assets/AssetStorage.h"

#include "EntitySystem/EntityPrefab.h"

class EntityPrefabDockable : public Slush::Dockable
{
public:
	EntityPrefabDockable(Slush::AssetStorage<EntityPrefab>& aPrefabStorage);
	const char* GetName() const override { return "Entity Prefab Editor"; }

protected:
	void OnUpdate() override;
	void OnBuildUI() override;

private:
	void HandleCreatingNewAsset();
	void HandleSavingAsNewAsset();

	bool VerifyUniqueNameForNewAsset() const;
	EntityPrefab* mySelectedPrefab = nullptr;

	Slush::AssetStorage<EntityPrefab>& myPrefabStorage;
	FW_String myNewPrefabNameStorage;
	bool myHasUniquePrefabName = false;

	bool myWantToOpenNewAssetDialogue = false;
	bool myWantToOpenSaveAsDialogue = false;
};