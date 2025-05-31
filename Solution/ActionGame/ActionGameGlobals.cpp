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
	FW_ASSERT(myTextureStorage != nullptr, "Neet to set a TextureStorage");
	return *myTextureStorage;
}

ActionGameGlobals::ActionGameGlobals()
{
}

ActionGameGlobals::~ActionGameGlobals()
{
	myEntityManager = nullptr;
	myPhysicsWorld = nullptr;
	myTextureStorage = nullptr;
}

