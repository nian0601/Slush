#pragma once

#include <FW_String.h>
#include <FW_FileSystem.h>

#include "Dockable.h"

#include "Core/Assets/AssetStorage.h"
#include "Graphics/Texture.h"

namespace Slush
{
	class ContentBrowserDockable : public Slush::Dockable
	{
	public:
		enum class ViewMode
		{
			Grid,
			List,
		};

		// aDefaultViewMode is per instance and not persisted; the user can switch it with the Grid | List toggle.
		ContentBrowserDockable(ViewMode aDefaultViewMode = ViewMode::List);
		const char* GetName() const override { return "Content Browser"; }

	protected:
		void OnBuildUI() override;

	private:
		void BuildFilterSidebar(const FW_GrowingArray<IAssetStorage*>& someStorages);
		void BuildListView(const FW_GrowingArray<IAssetStorage*>& someStorages);
		void BuildGridView(const FW_GrowingArray<IAssetStorage*>& someStorages);

		// Drag-out and the right-click context menu, shared by both views. Acts on the last submitted item.
		void BuildAssetInteraction(Asset& anAsset, int anAssetIndex);

		bool IsStorageVisible(int aStorageIndex) const;

		FW_GrowingArray<bool> myAssetFilters;
		ViewMode myViewMode = ViewMode::List;
	};
}
