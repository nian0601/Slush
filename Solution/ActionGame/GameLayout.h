#pragma once

#include "Core/Dockables/IAppLayout.h"
#include "UI/UIManager.h"

class Level;

namespace Slush
{
	class Font;
	class RectSprite;
	class Text;
	class PhysicsWorld;
	class StateStack;
}

class GameLayout : public Slush::IAppLayout
{
public:
	GameLayout();
	~GameLayout();

	void OnUpdate() override;
	void OnRender() override;

private:
	void UpdateGameOver();

	Slush::StateStack* myStateStack = nullptr;
};
