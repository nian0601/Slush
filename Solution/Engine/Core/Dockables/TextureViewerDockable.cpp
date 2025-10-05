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

				const float localX = io.MousePos.x - pos.x;
				const float localY = io.MousePos.y - pos.y;

				const int tileX = static_cast<int>(localX / myTexCoordHelperSpriteSize.x);
				const int tileY = static_cast<int>(localY / myTexCoordHelperSpriteSize.y);

				Vector2i texCoord = { tileX * myTexCoordHelperSpriteSize.x , tileY * myTexCoordHelperSpriteSize.y };
				
				ImGuiDragDropFlags src_flags = ImGuiDragDropFlags_SourceAllowNullID;
				if (ImGui::BeginDragDropSource(src_flags))
				{
					if (!(src_flags & ImGuiDragDropFlags_SourceNoPreviewTooltip))
						ImGui::Text("Dragging TextureRect");

					Recti texRect = MakeRectFromTopLeft(texCoord, myTexCoordHelperSpriteSize);
					ImGui::SetDragDropPayload("TextureRect", &texRect, sizeof(Recti), ImGuiCond_Once);
					ImGui::EndDragDropSource();
				}

				if (ImGui::IsItemHovered())
				{
					sf::FloatRect texRect;
					texRect.left = static_cast<float>(texCoord.x);
					texRect.top = static_cast<float>(texCoord.y);
					texRect.width = static_cast<float>(myTexCoordHelperSpriteSize.x);
					texRect.height = static_cast<float>(myTexCoordHelperSpriteSize.y);

					if (myShowTexCoordHelper && ImGui::BeginTooltip())
					{
						ImGui::Text("Tex (%i, %i)", static_cast<int>(texRect.left), static_cast<int>(texRect.top));
						ImGui::Image(*texture->GetSFMLTexture(), { texRect.width, texRect.height }, texRect);

						ImGui::EndTooltip();
					}
				}
			}
		}
	}
}