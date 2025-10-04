#pragma once

#include <FW_String.h>
#include <FW_FileSystem.h>

#include "Dockable.h"

#include "Core/Assets/AssetStorage.h"
#include "Graphics/Texture.h"

namespace Slush
{
	class TextureViewerDockable : public Slush::Dockable
	{
	public:
		TextureViewerDockable(const Slush::AssetStorage<Slush::Texture>& aTextureStorage);
		const char* GetName() const override { return "Texture Viewer"; }

	protected:
		void OnBuildUI() override;

	private:
		const Slush::AssetStorage<Slush::Texture>& myTextureStorage;
		bool myShowTexCoordHelper = true;
		Vector2i myTexCoordHelperSpriteSize = { 48, 48 };
	};
}