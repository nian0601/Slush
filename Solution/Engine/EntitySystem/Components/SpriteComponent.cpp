#include "stdafx.h"

#include "SpriteComponent.h"

#include "AnimationComponent.h"

#include "EntitySystem/Entity.h"
#include "EntitySystem/EntityPrefab.h"

#include "Graphics/Animation/Animation.h"

#include "Graphics/CircleSprite.h"
#include "Graphics/RectSprite.h"
#include "Graphics/Texture.h"
#include "Graphics/Animation/AnimationRuntime.h"
#include <imgui\ImGuiWidgets.h>

namespace Slush
{
	SpriteComponent::Data::Data()
		: Component::BaseData()
	{
		// TODO: Change Slush::Asset to have empty ctor?
		// and instead have AssetStorage call functions for setting name and assetID?
		// Just to make it a bit less hacky when assets needs to be created from code?
		myAnimation = new Animation("spritecomponent_anim", 0);
	}

	SpriteComponent::Data::~Data()
	{
		FW_SAFE_DELETE(myAnimation);
	}

	void SpriteComponent::Data::OnParse(AssetParser::Handle aComponentHandle, unsigned int /*aVersion*/)
	{
		aComponentHandle.ParseIntField("color", myColor);

		AssetParser::Handle sizeHandle = aComponentHandle.ParseChildElement("size");
		if (sizeHandle.IsValid())
		{
			sizeHandle.ParseFloatField("width", mySize.x);
			sizeHandle.ParseFloatField("height", mySize.y);
		}

		AssetParser::Handle colorHandle = aComponentHandle.ParseChildElement("floatColor");
		if (colorHandle.IsValid())
		{
			colorHandle.ParseFloatField("r", myFloatColor[0]);
			colorHandle.ParseFloatField("g", myFloatColor[1]);
			colorHandle.ParseFloatField("b", myFloatColor[2]);
			colorHandle.ParseFloatField("a", myFloatColor[3]);
		}


		if (aComponentHandle.IsReading())
		{
			if (aComponentHandle.HasField("textureID"))
				mySpriteType = SpriteType::Static;
			else if (aComponentHandle.HasChildElement("animation"))
				mySpriteType = SpriteType::Animated;
		}

		if (mySpriteType == SpriteType::Static)
		{
			myTexture.Parse(aComponentHandle, "textureID");

			AssetParser::Handle textureRectHandle = aComponentHandle.ParseChildElement("textureRect");
			if (textureRectHandle.IsValid())
			{
				textureRectHandle.ParseIntField("x", myTextureRectPos.x);
				textureRectHandle.ParseIntField("y", myTextureRectPos.y);
				textureRectHandle.ParseIntField("width", myTextureRectSize.x);
				textureRectHandle.ParseIntField("height", myTextureRectSize.y);
			}
		}
		else if (mySpriteType == SpriteType::Animated)
		{
			AssetParser::Handle animationHandle = aComponentHandle.ParseChildElement("animation");
			animationHandle.ParseBoolField("islooping", myLoopAnimation);
			animationHandle.ParseBoolField("removeentityafteranimation", myRemoveEntityAfterAnimation);
			myAnimation->OnParse(animationHandle, 0); // embedded inline, not a standalone loaded Animation asset, no version context of its own
		}
	}

	void SpriteComponent::Data::ResolveDependencies()
	{
		myTexture.ResolveDependency();
	}

	void SpriteComponent::Data::OnBuildUI()
	{
		Slush::ImGuiWidgets::ColorEdit4("Color", myFloatColor);
		myColor = FW_Float_To_ARGB(myFloatColor[3], myFloatColor[0], myFloatColor[1], myFloatColor[2]);
		Slush::ImGuiWidgets::InputFloat2("Size", &mySize.x, "%.2f");

		const char* spriteTypeNames[] = { "None", "Static", "Animation" };

		if (ImGui::BeginCombo("Sprite Type", spriteTypeNames[mySpriteType]))
		{
			for (int i = 0; i < IM_ARRAYSIZE(spriteTypeNames); ++i)
			{
				const bool is_selected = (mySpriteType== i);
				if (Slush::ImGuiWidgets::Selectable(spriteTypeNames[i], is_selected))
					mySpriteType = static_cast<SpriteType>(i);

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		if (mySpriteType == SpriteType::Static)
		{
			ImGui::BeginGroup();
			ImGui::Text("TextureID: %s", myTexture.GetName().GetBuffer());
			Slush::ImGuiWidgets::InputInt2("TextureRectPos", &myTextureRectPos.x);
			Slush::ImGuiWidgets::InputInt2("TextureRectSize", &myTextureRectSize.x);

			ImGui::EndGroup();

			if (ImGui::BeginDragDropTarget())
			{
				if (Asset* asset = ImGui::AcceptDraggedAsset(GetAssetID<Texture>()))
					myTexture.Set(static_cast<Texture*>(asset));

				if (const ImGuiPayload* imguiPayload = ImGui::AcceptDragDropPayload("TextureDragPayload"))
				{
					TextureDragPayload* payload = static_cast<TextureDragPayload*>(imguiPayload->Data);
					myTextureRectPos = payload->myTextureRect.myTopLeft;
					myTextureRectSize = payload->myTextureRect.myExtents;
					myTexture.Set(payload->myTexture);
				}

				ImGui::EndDragDropTarget();
			}

			if (const Texture* texture = myTexture.Get())
			{
				sf::FloatRect texRect;
				texRect.position.x = static_cast<float>(myTextureRectPos.x);
				texRect.position.y = static_cast<float>(myTextureRectPos.y);
				texRect.size.x = static_cast<float>(mySize.x);
				texRect.size.y = static_cast<float>(mySize.y);

				ImGui::Image(*texture->GetSFMLTexture(), { texRect.size.x, texRect.size.y }, texRect);
			}
		}
		else if (mySpriteType == SpriteType::Animated)
		{
			Slush::ImGuiWidgets::Checkbox("Loop Animation", &myLoopAnimation);
			ImGui::SameLine();
			Slush::ImGuiWidgets::Checkbox("Remove Entity After Animation", &myRemoveEntityAfterAnimation);
			myAnimation->BuildUI();
		}
	}

	//////////////////////////////////////////////////////////////////////////

	SpriteComponent::SpriteComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab)
		: Component(anEntity, anEntityPrefab)
	{
		const SpriteComponent::Data& spriteData = anEntityPrefab.GetComponentData<SpriteComponent>();
		if (spriteData.mySize.x <= 0.f)
		{
			SLUSH_ERROR("Trying to create SpriteComponent with 0 size");
			return;
		}

		MakeRect(spriteData.mySize.x, spriteData.mySize.y, spriteData.myColor);

		if (spriteData.mySpriteType == Data::Static)
		{
			if (const Texture* texture = spriteData.myTexture.Get())
			{
				mySprite->SetTexture(*texture);
				mySprite->SetOutlineColor(0x00000000);
			}
			else
			{
				SLUSH_ERROR("[Entity] %i failed to load texture '%s'", anEntityPrefab.GetAssetName().GetBuffer(), spriteData.myTexture.GetName().GetBuffer());
			}

			if (spriteData.myTextureRectSize.x > 0)
				mySprite->SetTextureRect(
					spriteData.myTextureRectPos.x,
					spriteData.myTextureRectPos.y,
					spriteData.myTextureRectSize.x,
					spriteData.myTextureRectSize.y);
		}
		else if (spriteData.mySpriteType == Data::Animated)
		{
			myAnimation = spriteData.myAnimation;
			RectSprite* rect = static_cast<RectSprite*>(mySprite);
			rect->SetOutlineColor(0xFF000000);
			rect->SetOutlineThickness(0.f);

			if (const AnimationClip* clip = myAnimation->FindFirstSpriteSheetClip())
			{
				Recti texRect = static_cast<const SpriteSheetClip*>(clip)->myFrameRect;
				mySprite->SetTextureRect(texRect.myTopLeft.x, texRect.myTopLeft.y, texRect.myExtents.x, texRect.myExtents.y);
			}
			else
			{
				SLUSH_ERROR("%s uses a Animated SpriteComponent without a SpriteSheet-clip, expect weird first-frame texturing", myEntityPrefab.GetAssetName().GetBuffer());
			}
		}
		else
		{
			FW_ASSERT_ALWAYS("Invalid SpriteType when creating SpriteComponent");
		}
	}

	SpriteComponent::~SpriteComponent()
	{
		FW_SAFE_DELETE(mySprite);
	}

	void SpriteComponent::MakeRect(float aWidth, float aHeight, int aColor)
	{
		RectSprite* rect = new RectSprite();
		rect->SetSize(aWidth, aHeight);
		rect->SetFillColor(aColor);
		rect->SetOutlineColor(0xFF440000);
		rect->SetOutlineThickness(1.f);
		mySprite = rect;
	}

	void SpriteComponent::Update()
	{
		if (!myAnimation)
			return;

		AnimationComponent* animComponent = myEntity.GetComponent<AnimationComponent>();
		if (!animComponent)
		{
			SLUSH_ERROR("%s uses Animated SpriteComponent but doenst have an AnimationComponent", myEntityPrefab.GetAssetName().GetBuffer());
			return;
		}

		if (!myAnimationRuntime)
		{
			myAnimationRuntime = animComponent->PlayAnimation(*myAnimation);
			myAnimationRuntime->myIsLooping = myEntityPrefab.GetComponentData<SpriteComponent>().myLoopAnimation;
		}

		if (!animComponent->IsAnimationPlaying(*myAnimation) && myEntityPrefab.GetComponentData<SpriteComponent>().myRemoveEntityAfterAnimation)
		{
			myEntity.myIsMarkedForRemoval = true;
		}
	}

	void SpriteComponent::Render()
	{
		FW_ASSERT(mySprite != nullptr, "Tried to render SpriteComponent without a sprite");
		mySprite->Render(myEntity.myPosition.x, myEntity.myPosition.y);
	}
}
