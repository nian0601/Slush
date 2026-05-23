#pragma once

#include "AppLayout.h"
#include "UI\UIManager.h"

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

class GameLayout : public AppLayout
{
public:
	GameLayout();
	~GameLayout();

	void Update() override;
	void Render() override;

private:
	void UpdateGameOver();

	StateStack* myStateStack = nullptr;
};
