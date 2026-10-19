#include "stdafx.h"

#include "LevelEditorLayout.h"

#include "LevelEditorDockable.h"
#include "LevelEditorUtils.h"
#include "Level/Level.h"
#include "Level/LevelData.h"
#include "Level/NavmeshData.h"

#include "Core/AssetRepository.h"
#include "Core/Assets/AssetStorage.h"
#include "Core/Assets/DependencyTracker.h"
#include "Core/CommandLineArgs.h"
#include "Core/Engine.h"
#include "Core/Dockables/ContentBrowserDockable.h"
#include "Core/Dockables/GameViewDockable.h"
#include "Core/Dockables/LogDockable.h"
#include "Graphics/Window.h"
#include "Graphics/Renderer.h"

#include <FW_FileSystem.h>

namespace
{
	const char* const ourDefaultLevelDataAssetName = "level_main";
	const char* const ourRememberedLevelFileName = "leveleditor_lastlevel.txt";

	// Resolves aLevelData's references the same way AssetRegistry::LoadAllAssets() does, so a level created
	// or copied at runtime has live prefab/navmesh pointers and shows up in the dependency tracker.
	void ResolveLevelDependencies(LevelData& aLevelData)
	{
		Slush::DependencyTracker& dependencyTracker = Slush::DependencyTracker::GetInstance();
		dependencyTracker.BeginTrackingAsset(&aLevelData);
		aLevelData.ResolveDependencies();
		dependencyTracker.EndTrackingAsset();
	}
}

LevelEditorLayout::LevelEditorLayout()
	: Slush::IAppLayout("LevelEditor", "Level Editor")
{
	LevelData& levelData = GetStartupLevel();
	myLevel = new Level(levelData, false, true);

	AddDockable(new Slush::GameViewDockable());

	myLevelEditorDockable = new LevelEditorDockable(*this, *myLevel);
	AddDockable(myLevelEditorDockable);
	AddDockable(new Slush::ContentBrowserDockable(Slush::ContentBrowserDockable::ViewMode::Grid));
	AddDockable(new Slush::LogDockable());

	WriteRememberedLevelName(levelData.GetAssetName());
	SLUSH_INFO("[Level Editor] Opened '%s'", levelData.GetAssetName().GetBuffer());
}

LevelEditorLayout::~LevelEditorLayout()
{
	FW_SAFE_DELETE(myLevel);
}

void LevelEditorLayout::OpenLevel(LevelData& aLevelData)
{
	FW_SAFE_DELETE(myLevel);
	myLevel = new Level(aLevelData, false, true);
	myLevelEditorDockable->SetLevel(*myLevel);

	WriteRememberedLevelName(aLevelData.GetAssetName());
	SLUSH_INFO("[Level Editor] Opened '%s'", aLevelData.GetAssetName().GetBuffer());
}

LevelData& LevelEditorLayout::CreateNewLevel(const FW_String& aLevelName)
{
	Slush::AssetRegistry& assetRegistry = Slush::AssetRegistry::GetInstance();

	FW_String navmeshName;
	LevelEditorUtils::GetNavmeshNameForLevel(aLevelName, navmeshName);
	Slush::IAssetStorage& navmeshStorage = assetRegistry.GetAssetStorage(Slush::GetAssetID<NavmeshData>());
	NavmeshData& navmeshData = static_cast<NavmeshData&>(navmeshStorage.CreateNewAsset(navmeshName.GetBuffer()));
	navmeshData.myNavmesh.GenerateDefaultGrid();
	navmeshData.Save();

	// Waves keep LevelData's defaults and the unlock list starts empty. Start/goal sit at left-middle and
	// right-middle of the 1920x1080 play area so the level is playable straight away.
	Slush::IAssetStorage& levelStorage = assetRegistry.GetAssetStorage(Slush::GetAssetID<LevelData>());
	LevelData& levelData = static_cast<LevelData&>(levelStorage.CreateNewAsset(aLevelName.GetBuffer()));
	levelData.myStartPosition = Vector2f(100.f, 540.f);
	levelData.myGoalPosition = Vector2f(1820.f, 540.f);
	levelData.myNavmeshData.Set(&navmeshData);
	ResolveLevelDependencies(levelData);
	levelData.Save();

	return levelData;
}

LevelData& LevelEditorLayout::DuplicateLevel(const FW_String& aLevelName, const LevelData& aSourceLevel)
{
	Slush::AssetRegistry& assetRegistry = Slush::AssetRegistry::GetInstance();
	const NavmeshData* sourceNavmesh = aSourceLevel.myNavmeshData.Get();
	FW_ASSERT(sourceNavmesh, "Duplicating a level with no resolved navmesh");

	// CopyAsset copies from the files on disk, so unsaved edits aren't part of the copy.
	FW_String navmeshName;
	LevelEditorUtils::GetNavmeshNameForLevel(aLevelName, navmeshName);
	Slush::IAssetStorage& navmeshStorage = assetRegistry.GetAssetStorage(Slush::GetAssetID<NavmeshData>());
	NavmeshData& navmeshData = static_cast<NavmeshData&>(navmeshStorage.CopyAsset(navmeshName.GetBuffer(), *sourceNavmesh));

	// Re-point the copy at its own navmesh before resolving, so two levels never share one navmesh.
	Slush::IAssetStorage& levelStorage = assetRegistry.GetAssetStorage(Slush::GetAssetID<LevelData>());
	LevelData& levelData = static_cast<LevelData&>(levelStorage.CopyAsset(aLevelName.GetBuffer(), aSourceLevel));
	levelData.myNavmeshData.Set(&navmeshData);
	ResolveLevelDependencies(levelData);
	levelData.Save();

	return levelData;
}

void LevelEditorLayout::OnUpdate()
{
	myLevel->Update();
}

void LevelEditorLayout::OnRender()
{
	Slush::Engine& engine = Slush::Engine::GetInstance();
	engine.GetWindow().GetRenderer().StartOffscreenBuffer();

	myLevelEditorDockable->RenderOverlay();
	myLevel->Render();

	engine.GetWindow().GetRenderer().EndOffscreenBuffer();
}

LevelData& LevelEditorLayout::GetStartupLevel() const
{
	// -editlevel > remembered level > level_main
	Slush::AssetRegistry& assetRegistry = Slush::AssetRegistry::GetInstance();
	Slush::CommandLineArgs& commandLineArgs = Slush::CommandLineArgs::GetInstance();
	if (commandLineArgs.HasFlag("-editlevel"))
	{
		if (const char* levelAssetName = commandLineArgs.GetString("-editlevel"))
		{
			LevelData* levelData = assetRegistry.GetAsset<LevelData>(levelAssetName);
			FW_ASSERT(levelData, "-editlevel has no valid LevelData - requested '%s'", levelAssetName);
			return *levelData;
		}

		SLUSH_WARNING("Missing -editlevel value; opening the remembered level");
	}

	FW_String rememberedLevelName;
	if (ReadRememberedLevelName(rememberedLevelName))
	{
		if (LevelData* levelData = assetRegistry.GetAsset<LevelData>(rememberedLevelName))
			return *levelData;

		SLUSH_WARNING("[Level Editor] Remembered level '%s' no longer exists; opening '%s'", rememberedLevelName.GetBuffer(), ourDefaultLevelDataAssetName);
	}

	LevelData* levelData = assetRegistry.GetAsset<LevelData>(ourDefaultLevelDataAssetName);
	FW_ASSERT(levelData, "Level Editor has no valid LevelData - expected a 'level_main' LevelData asset");
	return *levelData;
}

bool LevelEditorLayout::ReadRememberedLevelName(FW_String& anOutLevelName) const
{
	FW_String filePath;
	Slush::AssetRepository::GetDebugFilePath(ourRememberedLevelFileName, filePath);

	FILE* file = nullptr;
	fopen_s(&file, filePath.GetBuffer(), "r");
	if (!file)
		return false;

	char buffer[256] = {};
	const bool hasLine = fgets(buffer, sizeof(buffer), file) != nullptr;
	fclose(file);
	if (!hasLine)
		return false;

	anOutLevelName = buffer;
	FW_FileSystem::TrimBeginAndEnd(anOutLevelName);
	return !anOutLevelName.Empty();
}

void LevelEditorLayout::WriteRememberedLevelName(const FW_String& aLevelName) const
{
	// Per-machine state, so it lives in the untracked Debug folder rather than DebugSettings.sdebug
	FW_String filePath;
	Slush::AssetRepository::GetDebugFilePath(ourRememberedLevelFileName, filePath);

	FILE* file = nullptr;
	fopen_s(&file, filePath.GetBuffer(), "w");
	if (!file)
	{
		SLUSH_WARNING("[Level Editor] Could not write '%s'", filePath.GetBuffer());
		return;
	}

	fprintf(file, "%s\n", aLevelName.GetBuffer());
	fclose(file);
}
