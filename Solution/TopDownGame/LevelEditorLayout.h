#pragma once

#include <Core/Dockables/IAppLayout.h>

class Level;
class LevelEditorDockable;

class LevelEditorLayout : public Slush::IAppLayout
{
public:
	LevelEditorLayout();
	~LevelEditorLayout();

protected:
	void OnUpdate() override;
	void OnRender() override;

private:
	Level* myLevel = nullptr;
	LevelEditorDockable* myLevelEditorDockable = nullptr;
};
