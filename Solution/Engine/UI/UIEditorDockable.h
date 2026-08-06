#pragma once

#include <FW_String.h>
#include <FW_FileSystem.h>

#include "Core/Dockables/Dockable.h"

#include "Core/Assets/AssetStorage.h"
#include "UILayout.h"

namespace Slush
{
	class UIEditorDockable : public Slush::Dockable
	{
	public:
		UIEditorDockable(Slush::AssetStorage<Slush::UILayout>& aUILayoutStorage);
		const char* GetName() const override { return "UI Editor"; }

	protected:
		void OnBuildUI() override;

		bool HasUnsavedChanges() const override;
		void OnCloseRequested() override;
		void OnBuildModals() override;

	private:
		Slush::AssetStorage<Slush::UILayout>& myUILayoutStorage;

		bool myWantToOpenUnsavedChangesPopup = false;

		// "Close Without Saving" deliberately never clears UILayout::HasUnsavedChanges() - reopening it
		// must still show it as unsaved. This is what actually lets a later close attempt proceed instead.
		bool myUnsavedChangesAcknowledged = false;
	};
}