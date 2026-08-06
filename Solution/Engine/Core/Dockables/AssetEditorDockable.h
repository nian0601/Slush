#pragma once

#include "Core/Dockables/Dockable.h"
#include "Core/Assets/AssetStorage.h"

namespace Slush
{
	class AssetEditorDockable : public Dockable
	{
	public:
		AssetEditorDockable();
		const char* GetName() const override { return "Asset Editor"; }

	protected:
		void OnUpdate() override;
		void OnBuildUI() override;

		bool HasUnsavedChanges() const override;
		void OnCloseRequested() override;
		void OnBuildModals() override;

	private:
		void HandleCreatingNewAsset();
		void HandleSavingAsNewAsset();

		bool VerifyUniqueNameForNewAsset(bool aIsSaveAsNew) const;

		// Removes each of someAssets' tabs from myAssets without saving - shared by the popup's
		// "Save All" (after saving) and "Close Without Saving" outcomes.
		void CloseTabsForAssets(const FW_GrowingArray<Slush::Asset*>& someAssets);

		Slush::Asset* mySelectedAsset = nullptr;

		struct AssetData
		{
			Slush::Asset* myAsset = nullptr;
			bool myShouldKeep = true;
		};
		FW_GrowingArray<AssetData> myAssets;

		FW_String myNewAssetNameStorage;
		int myNewAssetStorageIndex = 0;
		bool myHasUniqueAssetName = false;

		bool myWantToOpenNewAssetDialogue = false;
		bool myWantToOpenSaveAsDialogue = false;

		// Assets the "unsaved changes" popup is currently asking about - a single asset when triggered
		// by a tab's X, every currently-unsaved asset when triggered by OnCloseRequested() (app-close).
		FW_GrowingArray<Slush::Asset*> myAssetsPendingCloseConfirmation;
		bool myWantToOpenUnsavedChangesPopup = false;

		// Only app-close's Cancel should abort the owning IAppLayout's close attempt - a tab's X isn't
		// part of one, and calling CancelCloseRequest() outside of one would poison a later real attempt.
		bool myIsConfirmingAppClose = false;
	};
}
