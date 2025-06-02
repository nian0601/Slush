#pragma once

#include "Component.h"

namespace Slush
{
	class BaseSprite;
}

class SpriteComponent : public Component
{
public:
	COMPONENT_HELPER("Sprite", "sprite");

	struct Data : public Component::BaseData
	{
		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;
		void OnBuildUI() override;

		float myRadius = 10.f;
		Vector2f mySize;
		int myColor = 0xFFFF3399;
		FW_String myTextureID;
		Vector2i myTextureRectPos;
		Vector2i myTextureRectSize;
		float myFloatColor[4] = { 1.f, 51.f / 255.f, 153.f / 255.f, 1.f };
	};

public:
	SpriteComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab);
	~SpriteComponent();

	void MakeCircle(float aRadius, int aColor);
	void MakeRect(float aWidth, float aHeight, int aColor);

	void Render() override;

	Slush::BaseSprite& GetSprite() { return *mySprite; }
	const Slush::BaseSprite& GetSprite() const { return *mySprite; }

private:
	Slush::BaseSprite* mySprite = nullptr;
};
