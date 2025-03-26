#pragma once

class EntityManager;

namespace Slush
{
	class PhysicsWorld;
}

class ActionGameGlobals
{
public:
	static ActionGameGlobals& GetInstance();
	static void Destroy();

	void SetEntityManager(EntityManager* anEntityManager) { myEntityManager = anEntityManager; }
	void SetPhysicsWorld(Slush::PhysicsWorld* aPhysicsWorld) { myPhysicsWorld = aPhysicsWorld; }

	EntityManager& GetEntityManager();
	Slush::PhysicsWorld& GetPhysicsWorld();

private:
	ActionGameGlobals();
	~ActionGameGlobals();
	static ActionGameGlobals* ourInstance;

	EntityManager* myEntityManager = nullptr;
	Slush::PhysicsWorld* myPhysicsWorld = nullptr;
};