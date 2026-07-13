#pragma once

#include "EntitySystem/Component.h"
#include <FW_Vector2.h>

class CharacterAnimationSet;
class CharacterAnimationComponent : public Slush::Component
{
public:
	COMPONENT_HELPER("Character Animation", "characteranimation");

	struct Data : public Slush::Component::BaseData
	{
		void OnParse(Slush::AssetParser::Handle aComponentHandle) override;
		void OnBuildUI() override;

		FW_String myAnimationSetID;
	};

public:
	CharacterAnimationComponent(Slush::Entity& anEntity, const Slush::EntityPrefab& anEntityPrefab);
	~CharacterAnimationComponent();

	void PlayMovementAnimation();
	void PlayAttackAnimation();

	//void OnEnterWorld() override;
	//void Update() override;
	
private:
	const CharacterAnimationSet* myAnimationSet;
};
