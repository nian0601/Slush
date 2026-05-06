#pragma once

#include "Component.h"

namespace Slush
{
	class Animation;
	class BaseSprite;

	struct AnimationRuntime;
}

class SpriteComponent : public Component
{
public:
	COMPONENT_HELPER("Sprite", "sprite");

	struct Data : public Component::BaseData
	{
		Data();
		~Data();

		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;
		void OnBuildUI() override;

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

		Slush::Animation* myAnimation;
		bool myLoopAnimation = true;
		bool myRemoveEntityAfterAnimation = true;

		FW_String myTextureID;
		Vector2i myTextureRectPos;
		Vector2i myTextureRectSize;

		
	};

public:
	SpriteComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab);
	~SpriteComponent();

	void MakeRect(float aWidth, float aHeight, int aColor);

	void Update() override;
	void Render() override;

	Slush::BaseSprite& GetSprite() { return *mySprite; }
	const Slush::BaseSprite& GetSprite() const { return *mySprite; }

private:
	Slush::BaseSprite* mySprite = nullptr;
	Slush::Animation* myAnimation = nullptr;
	Slush::AnimationRuntime* myAnimationRuntime = nullptr;
};
