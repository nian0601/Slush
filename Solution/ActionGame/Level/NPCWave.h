#pragma once
#include <FW_GrowingArray.h>
#include <FW_Vector2.h>

#include <Core\Time.h>

class Entity;
class EntityManager;

class NPCWave
{
public:
	NPCWave();

	void Update();

	void StartWave(int aNumberOfNPCs);

private:
	void CreateNPC(const Vector2f& aPosition);
	bool IsTooClose(const Vector2f& aPosition, const Vector2f& aTestPosition, float aTestClearance);

	EntityManager& myEntityManager;
	Slush::Timer myWaveTimer;

	FW_GrowingArray<EntityHandle> myNPCs;

	int myWaveCounter = 0;
	float myPlayerClearanceRadius = 350.f;
	float myNPCClearanceRadius = 100.f;
};