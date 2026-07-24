#include "stdafx.h"

#include "CharacterInfo.h"

#include "Graphics/Texture.h"
#include "Core/Assets/AssetStorage.h"

void CharacterInfo::OnParse(Slush::AssetParser::Handle aRootHandle, unsigned int /*aVersion*/)
{
	aRootHandle.ParseStringField("charactername", myName);
	myCharacterEntityPrefab.Parse(aRootHandle, "characterentityprefab");
	myPortaitTexture.Parse(aRootHandle, "portrait");
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

void CharacterInfo::ResolveDependencies()
{
	myPortaitTexture.ResolveDependency();
	myCharacterEntityPrefab.ResolveDependency();
}

void CharacterInfo::BuildUI()
{
	ImGui::BeginGroup();

	if (const Slush::Texture* texture = myPortaitTexture.Get())
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
	else
	{
		ImGui::BeginDisabled(true);
		ImGui::Button("Icon", { 64, 64 });
		ImGui::EndDisabled();
	}

	ImGui::InputText("Character Name", &myName);
	ImGui::Text("Character Prefab: %s", myCharacterEntityPrefab.GetName().GetBuffer());

	ImGui::EndGroup();

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* imguiPayload = ImGui::AcceptDragDropPayload("TextureDragPayload"))
		{
			Slush::TextureDragPayload* payload = static_cast<Slush::TextureDragPayload*>(imguiPayload->Data);
			myPortaitTexture.Set(payload->myTexture);
			myPortaitTextureRect = payload->myTextureRect;
		}

		if (Slush::Asset* asset = ImGui::AcceptDraggedAsset(Slush::GetAssetID<Slush::EntityPrefab>()))
			myCharacterEntityPrefab.Set(static_cast<Slush::EntityPrefab*>(asset));

		ImGui::EndDragDropTarget();
	}
}