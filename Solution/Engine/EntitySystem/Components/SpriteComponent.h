#pragma once

#include "EntitySystem/Component.h"
#include "Core/Assets/AssetReference.h"

namespace Slush
{
	class Animation;
	class BaseSprite;
	class Texture;

	struct AnimationRuntime;

	class SpriteComponent : public Component
	{
	public:
		COMPONENT_HELPER("Sprite", "sprite", 1);

		struct Data : public Component::BaseData
		{
			Data();
			~Data();

			void OnParse(AssetParser::Handle aComponentHandle, unsigned int aVersion) override;
			void OnBuildUI() override;
			void ResolveDependencies() override;

			Vector2f mySize;
			int myColor = 0xFFFFFFFF;
			float myFloatColor[4] = { 1.f, 1.f, 1.f, 1.f };

			enum SpriteType
			{
				None,
				Static,
				Animated,
			};
			SpriteType mySpriteType = None;

			Animation* myAnimation;
			bool myLoopAnimation = true;
			bool myRemoveEntityAfterAnimation = true;

			AssetReference<Texture> myTexture;
			Vector2i myTextureRectPos;
			Vector2i myTextureRectSize;


		};

	public:
		SpriteComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab);
		~SpriteComponent();

		void MakeRect(float aWidth, float aHeight, int aColor);

		void Update() override;
		void Render() override;

		BaseSprite& GetSprite() { return *mySprite; }
		const BaseSprite& GetSprite() const { return *mySprite; }

	private:
		BaseSprite* mySprite = nullptr;
		Animation* myAnimation = nullptr;
		AnimationRuntime* myAnimationRuntime = nullptr;
	};
}
