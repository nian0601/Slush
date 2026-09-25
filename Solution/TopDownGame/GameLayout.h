#pragma once

#include "Core/Dockables/IAppLayout.h"

namespace Slush
{
	class StateStack;
}

class GameLayout : public Slush::IAppLayout
{
public:
	GameLayout();
	~GameLayout();

protected:
	void OnUpdate() override;
	void OnRender() override;

private:
	Slush::StateStack* myStateStack = nullptr;
};
