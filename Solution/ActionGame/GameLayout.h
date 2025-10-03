#pragma once

#include "AppLayout.h"

class EntityManager;
class Level;

namespace Slush
{
	class PhysicsWorld;
	class UIButton;
	class UIManager;
}

class GameLayout : public AppLayout
{
public:
	GameLayout();
	~GameLayout();

	void Update() override;
	void Render() override;

private:
	void UpdatePhysics();
	void UpdateGameState();

	enum GameState
	{
		START_SCREEN,
		LOADING_LEVEL,
		PLAYING,
		GAME_OVER,
	};

	EntityManager* myEntityManager = nullptr;
	Slush::PhysicsWorld* myPhysicsWorld = nullptr;

	Level* myLevel = nullptr;
	GameState myGameState = START_SCREEN;

	Slush::UIButton* myStartGameButton = nullptr;
	Slush::UIManager* myStartGameUIManager = nullptr;

	Slush::UIButton* myRestartGameButton = nullptr;
	Slush::UIManager* myRestartGameUIManager = nullptr;
};
