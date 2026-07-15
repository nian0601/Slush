#pragma once

#include "StateStack/IGameState.h"

namespace Slush
{
	class EntityManager;
	class PhysicsWorld;
}

class CharacterInfo;
class Level;
class LevelState : public Slush::IGameState
{
public:
	LevelState(const CharacterInfo& aCharacterInfo);
	~LevelState();

	void ResumeState() override;

	GameStateResult Update() override;
	void Render() override;

private:
	void UpdateCollisions();

	Slush::EntityManager* myEntityManager = nullptr;
	Slush::PhysicsWorld* myPhysicsWorld = nullptr;

	Level* myLevel = nullptr;
	const CharacterInfo& myCharacterInfo;
};
