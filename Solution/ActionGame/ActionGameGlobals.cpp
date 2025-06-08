#include "stdafx.h"

#include "ActionGameGlobals.h"

ActionGameGlobals* ActionGameGlobals::ourInstance = nullptr;

ActionGameGlobals& ActionGameGlobals::GetInstance()
{
	if (!ourInstance)
		ourInstance = new ActionGameGlobals();

	return *ourInstance;
}

void ActionGameGlobals::Destroy()
{
	FW_SAFE_DELETE(ourInstance);
}

EntityManager& ActionGameGlobals::GetEntityManager()
{
	FW_ASSERT(myEntityManager != nullptr, "Need to set an EntityManager");
	return *myEntityManager;
}

Slush::PhysicsWorld& ActionGameGlobals::GetPhysicsWorld()
{
	FW_ASSERT(myPhysicsWorld != nullptr, "Need to set an PhysicsWorld");
	return *myPhysicsWorld;
}

Slush::AssetStorage<Slush::Texture>& ActionGameGlobals::GetTextureStorage()
{
	FW_ASSERT(myTextureStorage != nullptr, "Need to set a TextureStorage");
	return *myTextureStorage;
}

Slush::AssetStorage<EntityPrefab>& ActionGameGlobals::GetEntityPrefabStorage()
{
	FW_ASSERT(myEntityPrefabStorage != nullptr, "Need to set a EntityPrefabStorage");
	return *myEntityPrefabStorage;
}

ActionGameGlobals::ActionGameGlobals()
{
}

ActionGameGlobals::~ActionGameGlobals()
{
	myEntityManager = nullptr;
	myPhysicsWorld = nullptr;
	myTextureStorage = nullptr;
	myEntityPrefabStorage = nullptr;
}

