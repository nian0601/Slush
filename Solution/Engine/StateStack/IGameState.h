#pragma once

namespace Slush
{
	class StateStack;

	class IGameState
	{
	public:
		enum GameStateResult
		{
			KEEP,
			POP_MAINSTATE,
			POP_SUBSTATE,
		};

	public:
		virtual ~IGameState() {};

		virtual void StartState() {};
		virtual void EndState() {};
		virtual void ResumeState() {};

		virtual GameStateResult Update() = 0;
		virtual void Render() {};

		// Lets a lower state keep rendering while it's not the one being updated - e.g. a paused game
		// staying visible behind the pause menu. See StateStack::Update()/Render() for why this 1:N
		// relationship means the Update/Render split can't be merged.
		virtual bool AllowPassThroughRender() { return false; }

		StateStack* myStateStack = nullptr;
	};
}
