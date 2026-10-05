#include "stdafx.h"

#include "ActionGameGlobals.h"

#include <Core/AssetRepository.h>

void ActionGameGlobals::DebugSettings::LoadFromDisk()
{
	FW_String filePath;
	Slush::AssetRepository::GetDebugFilePath("DebugSettings.sdebug", filePath);

	Slush::AssetParser parser;
	Slush::AssetParser::Handle rootHandle = parser.Load(filePath.GetBuffer());

	if (rootHandle.IsValid())
		OnParse(rootHandle);
}

void ActionGameGlobals::DebugSettings::SaveToDisk()
{
	Slush::AssetParser parser;
	Slush::AssetParser::Handle rootHandle = parser.StartWriting("DebugSettings");

	OnParse(rootHandle);

	FW_String filePath;
	Slush::AssetRepository::GetDebugFilePath("DebugSettings.sdebug", filePath);
	parser.FinishWriting(filePath.GetBuffer());
}

void ActionGameGlobals::DebugSettings::OnParse(Slush::AssetParser::Handle aHandle)
{
	aHandle.ParseBoolField("PauseEnemySpawning", myPauseEnemySpawning);
	aHandle.ParseBoolField("SkipStartScreen", mySkipStartScreen);
	aHandle.ParseBoolField("UseNewUI", myUseNewUI);
	aHandle.ParseBoolField("ShowPhysicsObjects", myShowPhysicsObjects);
	aHandle.ParseBoolField("ShowPhysicsContacts", myShowPhysicsContacts);
}


void ActionGameGlobals::DebugSettingsDockable::OnBuildUI()
{
	ActionGameGlobals::DebugSettings& settings = ActionGameGlobals::GetInstance().myDebugSettings;
	ImGui::Checkbox("Pause Enemy Spawning", &settings.myPauseEnemySpawning);
	ImGui::Checkbox("Skip Start Screen", &settings.mySkipStartScreen);
	ImGui::Checkbox("New UI", &settings.myUseNewUI);
	ImGui::Checkbox("Physics Objects", &settings.myShowPhysicsObjects);
	ImGui::Checkbox("Physics Contacts", &settings.myShowPhysicsContacts);
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

Slush::Font& ActionGameGlobals::GetFont()
{
	FW_ASSERT(myFont != nullptr, "Need to set a Font");
	return *myFont;
}

Slush::EntityManager& ActionGameGlobals::GetEntityManager()
{
	FW_ASSERT(myEntityManager != nullptr, "Need to set an EntityManager");
	return *myEntityManager;
}

ActionGameGlobals::ActionGameGlobals()
{
	myDebugSettings.LoadFromDisk();
}

ActionGameGlobals::~ActionGameGlobals()
{
	myDebugSettings.SaveToDisk();

	myFont = nullptr;

	myEntityManager = nullptr;
}