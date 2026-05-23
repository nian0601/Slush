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

	void ResumeState() override;

	GameStateResult Update() override;
	void Render() override;

private:
	void UpdateCollisions();

	EntityManager* myEntityManager = nullptr;
	Slush::PhysicsWorld* myPhysicsWorld = nullptr;

	Level* myLevel = nullptr;
};
