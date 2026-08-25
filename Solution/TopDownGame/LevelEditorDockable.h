#pragma once

#include <Core/Dockables/Dockable.h>

class Level;

class LevelEditorDockable : public Slush::DockableBase<LevelEditorDockable>
{
public:
	LevelEditorDockable(Level& aLevel);

	const char* GetName() const override { return "Level Editor"; }

protected:
	void OnUpdate() override;
	void OnBuildUI() override;

private:
	Level& myLevel;
};
