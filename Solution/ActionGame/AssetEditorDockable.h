#pragma once

#include "Core/Dockables/Dockable.h"
#include "Core/Assets/AssetStorage.h"

#include "EntitySystem/EntityPrefab.h"

class AssetEditorDockable : public Slush::Dockable
{
public:
	AssetEditorDockable();
	const char* GetName() const override { return "Asset Editor"; }

protected:
	void OnUpdate() override;
	void OnBuildUI() override;

private:
	void HandleCreatingNewAsset();
	void HandleSavingAsNewAsset();

	bool VerifyUniqueNameForNewAsset() const;

	Slush::Asset* mySelectedAsset = nullptr;

	struct AssetData
	{
		Slush::Asset* myAsset = nullptr;
		bool myShouldKeep = true;
	};
	FW_GrowingArray<AssetData> myAssets;

	FW_String myNewAssetNameStorage;
	bool myHasUniqueAssetName = false;

	bool myWantToOpenNewAssetDialogue = false;
	bool myWantToOpenSaveAsDialogue = false;
};