#include "stdafx.h"

#include "IGameState.h"
#include "StateStack.h"

namespace Slush
{
	void StateStack::PushMainState(IGameState* aState)
	{
		myStates.Add();
		myMainIndex = myStates.Count() - 1;
		PushSubState(aState);
	}

	void StateStack::PushSubState(IGameState* aState)
	{
		FW_GrowingArray<IGameState*>& stateList = myStates.GetLast();
		stateList.Add(aState);
		mySubIndex = stateList.Count() - 1;

		aState->myStateStack = this;
		aState->StartState();
	}

	void StateStack::Update()
	{
		FW_ASSERT(myMainIndex > -1 && mySubIndex > -1, "Cant update an empty stack");

		FW_GrowingArray<IGameState*>& stateList = myStates.GetLast();
		IGameState::GameStateResult result = stateList.GetLast()->Update();
		switch (result)
		{
		case IGameState::KEEP:
			break;
		case IGameState::POP_MAINSTATE:
			PopMainState();
			break;
		case IGameState::POP_SUBSTATE:
			PopSubState();
			break;
		default:
			break;
		}
	}

	void StateStack::Render()
	{
		FW_GrowingArray<IGameState*>& stateList = myStates.GetLast();
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
		FW_GrowingArray<IGameState*>& stateList = myStates.GetLast();
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
		FW_GrowingArray<IGameState*>& stateList = myStates.GetLast();
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

		FW_GrowingArray<IGameState*>& stateList = myStates.GetLast();
		if (stateList[aSubIndex]->AllowPassThroughRender())
			RenderStateAtIndex(aSubIndex - 1);

		stateList[aSubIndex]->Render();
	}
}
