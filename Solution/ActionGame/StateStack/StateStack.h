#pragma once

class GameState;

class StateStack
{
public:
	void PushMainState(GameState* aState);
	void PushSubState(GameState* aState);

	void Update();
	void Render();

	void Clear();

private:
	void PopMainState();
	void PopSubState();
	void RenderStateAtIndex(int aSubIndex);

	FW_GrowingArray<FW_GrowingArray<GameState*>> myStates;
	int myMainIndex = -1;
	int mySubIndex = -1;
};
