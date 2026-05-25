#pragma once

class EntityManager;
class Tilemap;
class LevelData;
class StateStack;

struct EnemyWaveData;

class Level
{
public:
	Level();
	~Level();

	void Update(StateStack& aStateStack);
	void Restart();

	void RenderGame();

	bool IsPlayerDead() const;
	EntityHandle GetPlayer() const { return myPlayerHandle; }

private:
	void HandleEnemyWaves();

	bool IsTooClose(const Vector2f& aPosition, const Vector2f& aTestPosition, float aTestClearance);	

	LevelData* myLevelData = nullptr;
	EnemyWaveData* myEnemyWaveData = nullptr;
	EntityManager& myEntityManager;
	EntityHandle myPlayerHandle;

	Tilemap* myTilemap;

	Slush::Timer myWaveTimer;
	int myWaveCounter = 0;
	float myPlayerClearanceRadius = 350.f;
	float myEnemyClearanceRadius = 100.f;
};
