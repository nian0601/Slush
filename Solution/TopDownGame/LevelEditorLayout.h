#pragma once

#include <Core/Dockables/IAppLayout.h>
#include <FW_String.h>

class Level;
class LevelData;
class LevelEditorDockable;

namespace Slush
{
	class StateStack;
}

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

	// Play runs a real LevelState on the open level's in-memory data (unsaved edits included) in its own
	// StateStack. LevelState plays on a copy of the navmesh, so stopping returns to the unchanged editor.
	bool IsPlaying() const;
	void StartPlay();
	void StopPlay();

	// Info text for the game view: the playing note while playing, otherwise the active tool's hint (null when none).
	FW_String GetPlayingNote() const;
	const char* GetToolHint() const;

protected:
	void OnUpdate() override;
	void OnRender() override;

private:
	LevelData& GetStartupLevel() const;
	bool ReadRememberedLevelName(FW_String& anOutLevelName) const;
	void WriteRememberedLevelName(const FW_String& aLevelName) const;

	void OnPlayEnded();
	void RenderPlayingNote() const;

	Level* myLevel = nullptr;
	Slush::StateStack* myPlayStateStack = nullptr;
	LevelEditorDockable* myLevelEditorDockable = nullptr;
};
