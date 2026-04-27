#pragma once

#include "Component.h"
#include <FW_Vector2.h>

class CharacterAnimationSet;
class EntityPrefab;
class CharacterAnimationComponent : public Component
{
public:
	COMPONENT_HELPER("Character Animation", "characteranimation");

	struct Data : public Component::BaseData
	{
		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;
		void OnBuildUI() override;

		FW_String myAnimationSetID;
	};

public:
	CharacterAnimationComponent(Entity& anEntity, const EntityPrefab& anEntityPrefab);
	~CharacterAnimationComponent();

	void PlayMovementAnimation();
	void PlayAttackAnimation();

	//void OnEnterWorld() override;
	//void Update() override;
	
private:
	const CharacterAnimationSet* myAnimationSet;
};
