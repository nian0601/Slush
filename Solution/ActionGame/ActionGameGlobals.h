#pragma once


// I dont like needing to include Texture here, but its needed due to AssetStorage
// Perhaps I should expand on the IAssetStorage-concept to allow storing AssetStorage
// in places where I dont necessarily care about the concrete asset-type.
// It will make it a bit more cumbersome to use in the places that grabs the storage
// from here, since IAssetStorage will return base Asset*, so it needs to be
// casted into the actual asset, but maybe thats fine?
#include "Graphics/Texture.h"
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