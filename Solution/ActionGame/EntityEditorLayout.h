#pragma once

#include "Core/Dockables/IAppLayout.h"

class EntityEditorLayout : public Slush::IAppLayout
{
public:
	EntityEditorLayout();
	~EntityEditorLayout();

	void OnUpdate() override;
	void OnRender() override;

private:
};
