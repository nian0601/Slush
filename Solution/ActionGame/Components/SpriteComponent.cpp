#include "stdafx.h"

#include "SpriteComponent.h"

#include <Graphics/CircleSprite.h>
#include <Graphics/RectSprite.h>
#include "ActionGameGlobals.h"
#include <Core/Assets/AssetStorage.h>
#include <Graphics/Texture.h>

void SpriteComponent::Data::OnParse(Slush::AssetParser::Handle aComponentHandle)
{
	aComponentHandle.ParseFloatField("radius", myRadius);
	aComponentHandle.ParseIntField("color", myColor);

	Slush::AssetParser::Handle sizeHandle = aComponentHandle.ParseChildElement("size");
	if (sizeHandle.IsValid())
	{
		sizeHandle.ParseFloatField("width", mySize.x);
		sizeHandle.ParseFloatField("height", mySize.y);
	}

	Slush::AssetParser::Handle colorHandle = aComponentHandle.ParseChildElement("floatColor");
	if (colorHandle.IsValid())
	{
		colorHandle.ParseFloatField("r", myFloatColor[0]);
		colorHandle.ParseFloatField("g", myFloatColor[1]);
		colorHandle.ParseFloatField("b", myFloatColor[2]);
		colorHandle.ParseFloatField("a", myFloatColor[3]);
	}

	if (aComponentHandle.HasField("textureID") || !aComponentHandle.IsReading())
		aComponentHandle.ParseStringField("textureID", myTextureID);

	Slush::AssetParser::Handle textureRectHandle = aComponentHandle.ParseChildElement("textureRect");
	if (textureRectHandle.IsValid())
	{
		textureRectHandle.ParseIntField("x", myTextureRectPos.x);
		textureRectHandle.ParseIntField("y", myTextureRectPos.y);
		textureRectHandle.ParseIntField("width", myTextureRectSize.x);
		textureRectHandle.ParseIntField("height", myTextureRectSize.y);
	}
}

void SpriteComponent::Data::OnBuildUI()
{
	ImGui::ColorEdit4("Color", myFloatColor);
	ImGui::InputFloat("Radius", &myRadius, 1.f, 10.f, "%.2f");
	ImGui::InputFloat2("Size", &mySize.x, "%.2f");
	ImGui::InputText("TextureID", &myTextureID);

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(Slush::Texture::GetAssetTypeName()))
		{
			int textureIndex = *(const int*)payload->Data;
			const Slush::Texture* texture = static_cast<Slush::Texture*>(ActionGameGlobals::GetInstance().GetTextureStorage().GetAllAssets()[textureIndex]);
			myTextureID = texture->GetAssetName();
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::InputInt2("TextureRectPos", &myTextureRectPos.x);
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TextureRect"))
		{
			Recti texRect = *static_cast<Recti*>(payload->Data);
			myTextureRectPos = texRect.myTopLeft;
			myTextureRectSize = texRect.myExtents;
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::InputInt2("TextureRectSize", &myTextureRectSize.x);
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TextureRect"))
		{
			Recti texRect = *static_cast<Recti*>(payload->Data);
			myTextureRectPos = texRect.myTopLeft;
			myTextureRectSize = texRect.myExtents;
		}
		ImGui::EndDragDropTarget();
	}
	
	if (!myTextureID.Empty())
	{
		if (const Slush::Texture* texture = ActionGameGlobals::GetInstance().GetTextureStorage().GetAsset(myTextureID))
		{
			sf::FloatRect texRect;
			texRect.left = static_cast<float>(myTextureRectPos.x);
			texRect.top = static_cast<float>(myTextureRectPos.y);
			texRect.width = static_cast<float>(mySize.x);
			texRect.height = static_cast<float>(mySize.y);

			ImGui::Image(*texture->GetSFMLTexture(), { texRect.width, texRect.height }, texRect);
		}
	}

	myColor = FW_Float_To_ARGB(myFloatColor[3], myFloatColor[0], myFloatColor[1], myFloatColor[2]);
}

//////////////////////////////////////////////////////////////////////////

SpriteComponent::SpriteComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab)
	: Component(anEntity, anEntityPrefab)
{
	const SpriteComponent::Data& spriteData = anEntityPrefab.GetSpriteData();
	if (spriteData.mySize.x > 0.f)
		MakeRect(spriteData.mySize.x, spriteData.mySize.y, spriteData.myColor);
	else
		MakeCircle(spriteData.myRadius, spriteData.myColor);

	if (!spriteData.myTextureID.Empty())
	{
		const Slush::Texture* texture = ActionGameGlobals::GetInstance().GetTextureStorage().GetAsset(spriteData.myTextureID);
		if (texture)
		{
			mySprite->SetTexture(*texture);
			mySprite->SetOutlineColor(0x00000000);
		}
		else
		{
			SLUSH_ERROR("[Entity] %i failed to load texture '%s'", anEntityPrefab.GetAssetName().GetBuffer(), spriteData.myTextureID.GetBuffer());
		}

		if (spriteData.myTextureRectSize.x > 0)
			mySprite->SetTextureRect(
				spriteData.myTextureRectPos.x,
				spriteData.myTextureRectPos.y,
				spriteData.myTextureRectSize.x,
				spriteData.myTextureRectSize.y);
	}
}

SpriteComponent::~SpriteComponent()
{
	FW_SAFE_DELETE(mySprite);
}

void SpriteComponent::MakeCircle(float aRadius, int aColor)
{
	FW_ASSERT(mySprite == nullptr, "Tried to MakeCircle on a SpriteComponent that already has a sprite");

	Slush::CircleSprite* circle = new Slush::CircleSprite(aRadius);
	circle->SetFillColor(aColor);
	circle->SetOutlineColor(0xFF440000);
	circle->SetOutlineThickness(1.f);
	mySprite = circle;
}

void SpriteComponent::MakeRect(float aWidth, float aHeight, int aColor)
{
	Slush::RectSprite* rect = new Slush::RectSprite();
	rect->SetSize(aWidth, aHeight);
	rect->SetFillColor(aColor);
	rect->SetOutlineColor(0xFF440000);
	rect->SetOutlineThickness(1.f);
	mySprite = rect;
}

void SpriteComponent::Render()
{
	FW_ASSERT(mySprite != nullptr, "Tried to render SpriteComponent without a sprite");
	mySprite->Render(myEntity.myPosition.x, myEntity.myPosition.y);
}