#pragma once

#include "Core/Dockables/Dockable.h"
#include "Core/Assets/AssetStorage.h"

#include "EntitySystem/EntityPrefab.h"

class AssetEditorDockable : public Slush::Dockable
{
public:
	AssetEditorDockable(Slush::IAssetStorage& aAssetStorage, const char* aDockableName);
	const char* GetName() const override { return myDockableName.GetBuffer(); }

protected:
	void OnUpdate() override;
	void OnBuildUI() override;

private:
	void HandleCreatingNewAsset();
	void HandleSavingAsNewAsset();

	bool VerifyUniqueNameForNewAsset() const;

	FW_String myDockableName;

	Slush::IAssetStorage& myAssetStorage;
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