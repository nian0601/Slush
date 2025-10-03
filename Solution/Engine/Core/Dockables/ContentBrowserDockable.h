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
		ContentBrowserDockable(bool aShowByDefault = false);
		const char* GetName() const override { return "Content Browser"; }

		void AddAssetStorage(IAssetStorage* aStorage);

	protected:
		void OnUpdate() override;
		void OnBuildUI() override;

	private:
		struct StorageData
		{
			IAssetStorage* myAssetStorage = nullptr;
			bool myShouldShowAssets = true;
		};
		FW_GrowingArray<StorageData> myAssetStorages;
	};
}