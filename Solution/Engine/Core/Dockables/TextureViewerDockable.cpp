#include "stdafx.h"
#include "TextureViewerDockable.h"

#include "Core/Engine.h"
#include "Graphics/Window.h"

namespace Slush
{
	TextureViewerDockable::TextureViewerDockable()
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

		Slush::AssetRegistry& assetRegistry = Slush::AssetRegistry::GetInstance();
		const FW_GrowingArray<Slush::Asset*> assets = assetRegistry.GetAllAssets<Slush::Texture>();
		for (Slush::Asset* asset : assets)
		{
			Slush::Texture* texture = static_cast<Slush::Texture*>(asset);
			if (ImGui::CollapsingHeader(texture->GetAssetName().GetBuffer()))
			{
				ImGui::Text("File: %s", texture->GetFilePath().GetBuffer());
				ImGui::Text("Width: %i, Height: %i", texture->GetSize().x, texture->GetSize().y);

				ImGuiIO& io = ImGui::GetIO();
				ImVec2 pos = ImGui::GetCursorScreenPos(); // Needs to be BEFORE we render the image, else the cursor-position is wrong

				const float textureWidth = static_cast<float>(texture->GetSize().x);
				const float textureHeight = static_cast<float>(texture->GetSize().y);
				ImGui::Image(*texture->GetSFMLTexture(), { textureWidth, textureHeight });

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
					texRect.position.x = static_cast<float>(texCoord.x);
					texRect.position.y = static_cast<float>(texCoord.y);
					texRect.size.x = static_cast<float>(myTexCoordHelperSpriteSize.x);
					texRect.size.y = static_cast<float>(myTexCoordHelperSpriteSize.y);

					if (myShowTexCoordHelper && ImGui::BeginTooltip())
					{
						ImGui::Text("Tex (%i, %i)", static_cast<int>(texRect.position.x), static_cast<int>(texRect.position.y));
						ImGui::Image(*texture->GetSFMLTexture(), { texRect.size.x, texRect.size.y }, texRect);

						ImGui::EndTooltip();
					}
				}
			}
		}
	}
}