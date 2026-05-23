#pragma once

#include "GameState.h"

namespace Slush
{
	class PhysicsWorld;
}

class Level;
class LevelState : public GameState
{
public:
	LevelState();
	~LevelState();

	GameStateResult Update() override;
	void Render() override;

private:
	void UpdateCollisions();

	EntityManager* myEntityManager = nullptr;
	Slush::PhysicsWorld* myPhysicsWorld = nullptr;

	Level* myLevel = nullptr;
};
