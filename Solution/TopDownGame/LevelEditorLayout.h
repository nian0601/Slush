#pragma once

#include <Core/Dockables/IAppLayout.h>
#include <FW_String.h>

class Level;
class LevelData;
class LevelEditorDockable;

class LevelEditorLayout : public Slush::IAppLayout
{
public:
	LevelEditorLayout();
	~LevelEditorLayout();

	// Rebuilds the edited Level on aLevelData and remembers it as the level to reopen on the next launch.
	void OpenLevel(LevelData& aLevelData);

	// Both write the new .tdlevel and its .navmesh to disk straight away. The name must already be validated
	// (LevelEditorUtils::ValidateNewLevelName), since the asset storages return existing assets on a clash.
	LevelData& CreateNewLevel(const FW_String& aLevelName);
	LevelData& DuplicateLevel(const FW_String& aLevelName, const LevelData& aSourceLevel);

protected:
	void OnUpdate() override;
	void OnRender() override;

private:
	LevelData& GetStartupLevel() const;
	bool ReadRememberedLevelName(FW_String& anOutLevelName) const;
	void WriteRememberedLevelName(const FW_String& aLevelName) const;

	Level* myLevel = nullptr;
	LevelEditorDockable* myLevelEditorDockable = nullptr;
};
