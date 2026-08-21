#pragma once

namespace Slush
{
	class IGameState;

	class StateStack
	{
	public:
		void PushMainState(IGameState* aState);
		void PushSubState(IGameState* aState);

		// This split cannot be merged: Update() advances only the top state, while Render() recurses
		// down through AllowPassThroughRender() and renders bottom-to-top - a 1:N relationship (one
		// state updates, several may render) that is how a paused game stays visible behind a menu.
		// Unrelated to immediate-mode draw ordering, so the RenderQueue landing (#11) didn't dissolve
		// it. Because the pass-through recursion submits bottom-to-top in a single pass, submission
		// order already equals correct back-to-front draw order - no z-index/layer concept is needed
		// in the render queue to support it.
		void Update();
		void Render();

		void Clear();

	private:
		void PopMainState();
		void PopSubState();
		void RenderStateAtIndex(int aSubIndex);

		FW_GrowingArray<FW_GrowingArray<IGameState*>> myStates;
		int myMainIndex = -1;
		int mySubIndex = -1;
	};
}
