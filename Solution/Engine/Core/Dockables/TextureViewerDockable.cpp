#include "stdafx.h"
#include "TextureViewerDockable.h"

#include "Core/Engine.h"
#include "Graphics/Window.h"

namespace Slush
{

	TextureViewerDockable::TextureViewerDockable(const Slush::AssetStorage<Slush::Texture>& aTextureStorage)
		: myTextureStorage(aTextureStorage)
	{
	}

	void TextureViewerDockable::OnBuildUI()
	{
		const FW_GrowingArray<Slush::Asset*> assets = myTextureStorage.GetAllAssets();
		for (Slush::Asset* asset : assets)
		{
			Slush::Texture* texture = static_cast<Slush::Texture*>(asset);
			if (ImGui::CollapsingHeader(texture->GetAssetName().GetBuffer()))
			{
				ImGui::Text("File: %s", texture->GetFilePath().GetBuffer());
				ImGui::Text("Width: %i, Height: %i", texture->GetSize().x, texture->GetSize().y);
				ImGui::Image(*texture->GetSFMLTexture(), { 100.f, 100.f });
			}
		}
	}
}