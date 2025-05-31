#pragma once

class EntityManager;

namespace Slush
{
	template<typename T>
	class AssetStorage;

	class Texture;
	class PhysicsWorld;
}

class ActionGameGlobals
{
public:
	static ActionGameGlobals& GetInstance();
	static void Destroy();

	void SetEntityManager(EntityManager* anEntityManager) { myEntityManager = anEntityManager; }
	void SetPhysicsWorld(Slush::PhysicsWorld* aPhysicsWorld) { myPhysicsWorld = aPhysicsWorld; }
	void SetTextureStorage(Slush::AssetStorage<Slush::Texture>& aStorage) { myTextureStorage = &aStorage; }

	EntityManager& GetEntityManager();
	Slush::PhysicsWorld& GetPhysicsWorld();
	Slush::AssetStorage<Slush::Texture>& GetTextureStorage();

private:
	ActionGameGlobals();
	~ActionGameGlobals();
	static ActionGameGlobals* ourInstance;

	EntityManager* myEntityManager = nullptr;
	Slush::PhysicsWorld* myPhysicsWorld = nullptr;
	Slush::AssetStorage<Slush::Texture>* myTextureStorage = nullptr;
};