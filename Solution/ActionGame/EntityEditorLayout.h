#pragma once

#include "AppLayout.h"

class EntityEditorLayout : public AppLayout
{
public:
	EntityEditorLayout();
	~EntityEditorLayout();

	void Update() override;
	void Render() override;

private:
};
