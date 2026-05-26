#include "stdafx.h"

#include "CharacterInfo.h"

#include "Graphics/Texture.h"
#include "Core/Assets/AssetStorage.h"

void CharacterInfo::OnParse(Slush::AssetParser::Handle aRootHandle)
{
	aRootHandle.ParseStringField("charactername", myName);
	aRootHandle.ParseStringField("characterentityprefab", myCharacterEntityPrefabID);
	aRootHandle.ParseStringField("portrait", myPortaitTextureID);
	Slush::AssetParser::Handle textureRectHandle = aRootHandle.ParseChildElement("portraitTextureRect");
	if (textureRectHandle.IsValid())
	{
		textureRectHandle.ParseIntField("x", myPortaitTextureRect.myTopLeft.x);
		textureRectHandle.ParseIntField("y", myPortaitTextureRect.myTopLeft.y);
		textureRectHandle.ParseIntField("width", myPortaitTextureRect.myExtents.x);
		textureRectHandle.ParseIntField("height", myPortaitTextureRect.myExtents.y);

		if (aRootHandle.IsReading())
			myPortaitTextureRect = MakeRectFromTopLeft(myPortaitTextureRect.myTopLeft, myPortaitTextureRect.myExtents);
	}
}

void CharacterInfo::BuildUI()
{
	ImGui::BeginGroup();

	if (myPortaitTextureID.Empty())
	{
		ImGui::BeginDisabled(true);
		ImGui::Button("Icon", { 64, 64 });
		ImGui::EndDisabled();
	}
	else
	{
		Slush::AssetRegistry& assets = Slush::AssetRegistry::GetInstance();
		if (const Slush::Texture* texture = assets.GetAsset<Slush::Texture>(myPortaitTextureID))
		{
			sf::FloatRect texRect;
			texRect.position.x = static_cast<float>(myPortaitTextureRect.myTopLeft.x);
			texRect.position.y = static_cast<float>(myPortaitTextureRect.myTopLeft.y);
			texRect.size.x = static_cast<float>(myPortaitTextureRect.myExtents.x);
			texRect.size.y = static_cast<float>(myPortaitTextureRect.myExtents.y);

			sf::Vector2f size;
			size.x = static_cast<float>(FW_Max(myPortaitTextureRect.myExtents.x, 48));
			size.y = static_cast<float>(FW_Max(myPortaitTextureRect.myExtents.y, 48));

			ImGui::Image(*texture->GetSFMLTexture(), size, texRect);
		}
	}

	ImGui::InputText("Character Name", &myName);
	ImGui::InputText("Character Prefab", &myCharacterEntityPrefabID);

	ImGui::EndGroup();

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* imguiPayload = ImGui::AcceptDragDropPayload("TextureDragPayload"))
		{
			Slush::TextureDragPayload* payload = static_cast<Slush::TextureDragPayload*>(imguiPayload->Data);
			myPortaitTextureID = payload->myTexture->GetAssetName();
			myPortaitTextureRect = payload->myTextureRect;
		}

		if (Slush::Asset* asset = ImGui::AcceptDraggedAsset(Slush::GetAssetID<EntityPrefab>()))
			myCharacterEntityPrefabID = asset->GetAssetName();

		ImGui::EndDragDropTarget();
	}
}