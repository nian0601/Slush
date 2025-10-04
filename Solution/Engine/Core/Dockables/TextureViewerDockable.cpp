#include "stdafx.h"
#include "TextureViewerDockable.h"

#include "Core/Engine.h"
#include "Graphics/Window.h"

namespace Slush
{

	TextureViewerDockable::TextureViewerDockable(const Slush::AssetStorage<Slush::Texture>& aTextureStorage)
		: myTextureStorage(aTextureStorage)
	{
		myEnableHorizontalScrolling = true;
	}

	void TextureViewerDockable::OnBuildUI()
	{
		ImGui::Checkbox("TexCoord Helper", &myShowTexCoordHelper);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(150.f);
		ImGui::InputInt2("Sprite Size", &myTexCoordHelperSpriteSize.x);
		ImGui::Separator();

		const FW_GrowingArray<Slush::Asset*> assets = myTextureStorage.GetAllAssets();
		for (Slush::Asset* asset : assets)
		{
			Slush::Texture* texture = static_cast<Slush::Texture*>(asset);
			if (ImGui::CollapsingHeader(texture->GetAssetName().GetBuffer()))
			{
				ImGui::Text("File: %s", texture->GetFilePath().GetBuffer());
				ImGui::Text("Width: %i, Height: %i", texture->GetSize().x, texture->GetSize().y);

				ImGuiIO& io = ImGui::GetIO();
				ImVec2 pos = ImGui::GetCursorScreenPos();

				float displayWidth = FW_Max(static_cast<float>(texture->GetSize().x), 320.f);
				float displayHeight = FW_Max(static_cast<float>(texture->GetSize().y), 180.f);
				ImGui::Image(*texture->GetSFMLTexture(), { displayWidth, displayHeight });

				if (myShowTexCoordHelper && ImGui::IsItemHovered() && ImGui::BeginTooltip())
				{
					float localX = io.MousePos.x - pos.x;
					float localY = io.MousePos.y - pos.y;

					//ImGui::Text("Pos (%.2f, %.2f)", pos.x, pos.y);
					//ImGui::Text("Mouse (%.2f, %.2f)", io.MousePos.x, io.MousePos.y);
					//ImGui::Text("Local (%.2f, %.2f)", localX, localY);

					int tileX = static_cast<int>(localX / myTexCoordHelperSpriteSize.x);
					int tileY = static_cast<int>(localY / myTexCoordHelperSpriteSize.y);
					ImGui::Text("Tile (%i, %i)", tileX, tileY);
					ImGui::Text("Tex (%i, %i)", tileX * myTexCoordHelperSpriteSize.x, tileY * myTexCoordHelperSpriteSize.y);

					ImGui::EndTooltip();
				}
			}
		}
	}
}