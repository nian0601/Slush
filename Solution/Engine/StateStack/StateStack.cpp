#include "stdafx.h"

#include "IGameState.h"
#include "StateStack.h"
#include "Core/Engine.h"
#include "Core/DebugScriptRunner.h"
#include "Core/Profiler.h"

namespace Slush
{
	namespace StateStackUtils
	{
		void PostTrigger(const IGameState& aState, bool anIsActive)
		{
			if (anIsActive)
				Profiler::GetInstance().SetGameStateName(aState.GetTriggerName());
			FW_String name = FW_String("gamestate.") + aState.GetTriggerName() + (anIsActive ? ".active" : ".inactive");
			Engine::GetInstance().GetDebugScriptRunner().PostTrigger(name.GetBuffer());
		}
	}
	StateStack::~StateStack()
	{
		Clear();
	}

	void StateStack::PushMainState(IGameState* aState)
	{
		if (!myStates.IsEmpty() && !myStates.GetLast().IsEmpty())
			StateStackUtils::PostTrigger(*myStates.GetLast().GetLast(), false);
		myStates.Add();
		myMainIndex = myStates.Count() - 1;
		PushSubState(aState);
	}

	void StateStack::PushSubState(IGameState* aState)
	{
		FW_GrowingArray<IGameState*>& stateList = myStates.GetLast();
		if (!stateList.IsEmpty())
			StateStackUtils::PostTrigger(*stateList.GetLast(), false);
		stateList.Add(aState);
		mySubIndex = stateList.Count() - 1;

		aState->myStateStack = this;
		aState->StartState();
		StateStackUtils::PostTrigger(*aState, true);
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
			StateStackUtils::PostTrigger(*myStates.GetLast().GetLast(), true);
		}

	}

	void StateStack::PopSubState()
	{
		FW_GrowingArray<IGameState*>& stateList = myStates.GetLast();
		FW_ASSERT(!stateList.IsEmpty(), "Cant pop an empty stack");
		StateStackUtils::PostTrigger(*stateList.GetLast(), false);
		stateList.GetLast()->EndState();
		stateList.DeleteLast();

		if (!stateList.IsEmpty())
		{
			stateList.GetLast()->ResumeState();
			StateStackUtils::PostTrigger(*stateList.GetLast(), true);
		}
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
