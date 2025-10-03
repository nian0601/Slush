#include "stdafx.h"

#include "ActionGameGlobals.h"

void ActionGameGlobals::DebugSettings::LoadFromDisk()
{
	Slush::AssetParser parser;
	Slush::AssetParser::Handle rootHandle = parser.Load("Data/DebugSettings.sdebug");

	if (rootHandle.IsValid())
		OnParse(rootHandle);
}

void ActionGameGlobals::DebugSettings::SaveToDisk()
{
	Slush::AssetParser parser;
	Slush::AssetParser::Handle rootHandle = parser.StartWriting("DebugSettings");

	OnParse(rootHandle);

	parser.FinishWriting("Data/DebugSettings.sdebug");
}

void ActionGameGlobals::DebugSettings::OnParse(Slush::AssetParser::Handle aHandle)
{
	aHandle.ParseBoolField("PauseEnemySpawning", myPauseEnemySpawning);
	aHandle.ParseBoolField("SkipStartScreen", mySkipStartScreen);
	aHandle.ParseBoolField("UseNewUI", myUseNewUI);
}


void ActionGameGlobals::DebugSettingsDockable::OnBuildUI()
{
	ActionGameGlobals::DebugSettings& settings = ActionGameGlobals::GetInstance().myDebugSettings;
	ImGui::Checkbox("Pause Enemy Spawning", &settings.myPauseEnemySpawning);
	ImGui::Checkbox("Skip Start Screen", &settings.mySkipStartScreen);
	ImGui::Checkbox("New UI", &settings.myUseNewUI);
}


//////////////////////////////////////////////////////////////////////////

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

Slush::AssetStorage<Slush::UILayout>& ActionGameGlobals::GetUILayoutStorage()
{
	FW_ASSERT(myUILayoutStorage!= nullptr, "Need to set a UILayoutStorage");
	return *myUILayoutStorage;
}

Slush::Font& ActionGameGlobals::GetFont()
{
	FW_ASSERT(myFont != nullptr, "Need to set a Font");
	return *myFont;
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

ActionGameGlobals::ActionGameGlobals()
{
	myDebugSettings.LoadFromDisk();
}

ActionGameGlobals::~ActionGameGlobals()
{
	myDebugSettings.SaveToDisk();

	myTextureStorage = nullptr;
	myEntityPrefabStorage = nullptr;
	myUILayoutStorage = nullptr;
	myFont = nullptr;

	myEntityManager = nullptr;
	myPhysicsWorld = nullptr;
}