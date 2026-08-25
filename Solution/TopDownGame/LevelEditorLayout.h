#pragma once

#include <Core/Dockables/IAppLayout.h>

class Level;
class LevelEditorDockable;

class LevelEditorLayout : public Slush::IAppLayout
{
public:
	LevelEditorLayout(Level& aLevel);

protected:
	void OnRender() override;

private:
	Level& myLevel;
	LevelEditorDockable* myLevelEditorDockable = nullptr;
};
