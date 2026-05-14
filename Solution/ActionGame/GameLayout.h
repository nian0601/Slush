#pragma once

#include "AppLayout.h"
#include "UI\UIManager.h"

class EntityManager;
class Level;

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
	void UpdatePhysics();
	void UpdateGameState();

	void UpdateStartScreen();
	void UpdatePlaying();
	void UpdateGameOver();


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

	Slush::UIElementStyle myUIBackgroundStyle;
	Slush::UIElementStyle myUIButtonStyle;
	FW_GrowingArray<Slush::DynamicUIBuilder::RenderCommand> myUIRenderCommands;
	Slush::RectSprite* myUISprite;
	Slush::Text* myText;
	Slush::Font& myFont;
};
