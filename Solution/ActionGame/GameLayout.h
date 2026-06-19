#pragma once

#include "Core/Dockables/IAppLayout.h"
#include "UI/UIManager.h"

class EntityManager;
class Level;
class StateStack;

namespace Slush
{
	class Font;
	class RectSprite;
	class Text;
	class PhysicsWorld;
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

	StateStack* myStateStack = nullptr;
};
