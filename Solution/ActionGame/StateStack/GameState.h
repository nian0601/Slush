#pragma once

class StateStack;

class GameState
{
public:
	enum GameStateResult
	{
		KEEP,
		POP_MAINSTATE,
		POP_SUBSTATE,
	};

public:
	virtual ~GameState() {};

	virtual void StartState() {};
	virtual void EndState() {};
	virtual void ResumeState() {};

	virtual GameStateResult Update() = 0;
	virtual void Render() {};
	virtual bool AllowPassThroughRender() { return false; }

	StateStack* myStateStack = nullptr;
};
