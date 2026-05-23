#include "stdafx.h"

#include "GameState.h"
#include "StateStack.h"

void StateStack::PushMainState(GameState* aState)
{
	myStates.Add();
	myMainIndex = myStates.Count() - 1;
	PushSubState(aState);
}

void StateStack::PushSubState(GameState* aState)
{
	FW_GrowingArray<GameState*>& stateList = myStates.GetLast();
	stateList.Add(aState);
	mySubIndex = stateList.Count() - 1;

	aState->myStateStack = this;
	aState->StartState();
}

void StateStack::Update()
{
	FW_ASSERT(myMainIndex > -1 && mySubIndex > -1, "Cant update an empty stack");

	FW_GrowingArray<GameState*>& stateList = myStates.GetLast();
	GameState::GameStateResult result = stateList.GetLast()->Update();
	switch (result)
	{
	case GameState::KEEP:
		break;
	case GameState::POP_MAINSTATE:
		PopMainState();
		break;
	case GameState::POP_SUBSTATE:
		PopSubState();
		break;
	default:
		break;
	}
}

void StateStack::Render()
{
	FW_GrowingArray<GameState*>& stateList = myStates.GetLast();
	FW_ASSERT(!stateList.IsEmpty(), "Cant render an empty stack");
	RenderStateAtIndex(stateList.Count() - 1);
}

void StateStack::Clear()
{
	while (!myStates.IsEmpty())
		PopMainState();
}

void StateStack::PopMainState()
{
	FW_GrowingArray<GameState*>& stateList = myStates.GetLast();
	while (!stateList.IsEmpty())
		PopSubState();

	myStates.RemoveLast();

	if (myStates.IsEmpty())
	{
		SLUSH_DEBUG("Last MainState Popped");
	}
	else
	{
		myStates.GetLast().GetLast()->ResumeState();
	}

}

void StateStack::PopSubState()
{
	FW_GrowingArray<GameState*>& stateList = myStates.GetLast();
	FW_ASSERT(!stateList.IsEmpty(), "Cant pop an empty stack");
	stateList.GetLast()->EndState();
	stateList.DeleteLast();

	if (!stateList.IsEmpty())
		stateList.GetLast()->ResumeState();
}

void StateStack::RenderStateAtIndex(int aSubIndex)
{
	if (aSubIndex < 0)
		return;

	FW_GrowingArray<GameState*>& stateList = myStates.GetLast();
	if (stateList[aSubIndex]->AllowPassThroughRender())
		RenderStateAtIndex(aSubIndex - 1);

	stateList[aSubIndex]->Render();
}
