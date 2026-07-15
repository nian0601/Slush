#pragma once

namespace Slush
{
	class IGameState;

	class StateStack
	{
	public:
		void PushMainState(IGameState* aState);
		void PushSubState(IGameState* aState);

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
