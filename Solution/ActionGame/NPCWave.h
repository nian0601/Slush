#pragma once
#include <FW_GrowingArray.h>
#include <FW_Vector2.h>

class Entity;
class ProjectileManager;

class NPCWave
{
public:
	NPCWave(Entity& aPlayerEntity, ProjectileManager& aProjectileManager);
	~NPCWave();

	void Update();
	void Render();

	void StartWave(int aNumberOfNPCs);

private:
	void CreateNPC(const Vector2f& aPosition);
	bool IsTooClose(const Vector2f& aPosition, const Vector2f& aTestPosition, float aTestClearance);

	Entity& myPlayerEntity;
	ProjectileManager& myProjectileManager;

	FW_GrowingArray<Entity*> myNPCs;
	float myPlayerClearanceRadius = 350.f;
	float myNPCClearanceRadius = 100.f;
};