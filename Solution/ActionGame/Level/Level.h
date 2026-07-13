#pragma once

#include <EntitySystem/EntityHandle.h>
#include <Core/Time.h>

class CharacterInfo;
class Tilemap;
class LevelData;
class StateStack;

struct EnemyWaveData;

namespace Slush
{
	class EntityManager;
}

class Level
{
public:
	Level(const CharacterInfo& aCharacterInfo);
	~Level();

	void Update(StateStack& aStateStack);
	void Restart();

	void RenderGame();

	bool IsPlayerDead() const;
	Slush::EntityHandle GetPlayer() const { return myPlayerHandle; }

private:
	void HandleEnemyWaves();

	bool IsTooClose(const Vector2f& aPosition, const Vector2f& aTestPosition, float aTestClearance);	

	const CharacterInfo& myPlayerCharacterInfo;
	LevelData* myLevelData = nullptr;
	EnemyWaveData* myEnemyWaveData = nullptr;
	Slush::EntityManager& myEntityManager;
	Slush::EntityHandle myPlayerHandle;

	Tilemap* myTilemap;

	Slush::Timer myWaveTimer;
	int myWaveCounter = 0;
	float myPlayerClearanceRadius = 350.f;
	float myEnemyClearanceRadius = 100.f;
};
