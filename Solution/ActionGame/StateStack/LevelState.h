#pragma once

#include "GameState.h"

namespace Slush
{
	class PhysicsWorld;
}

class CharacterInfo;
class Level;
class LevelState : public GameState
{
public:
	LevelState(const CharacterInfo& aCharacterInfo);
	~LevelState();

	void ResumeState() override;

	GameStateResult Update() override;
	void Render() override;

private:
	void UpdateCollisions();

	EntityManager* myEntityManager = nullptr;
	Slush::PhysicsWorld* myPhysicsWorld = nullptr;

	Level* myLevel = nullptr;
	const CharacterInfo& myCharacterInfo;
};
