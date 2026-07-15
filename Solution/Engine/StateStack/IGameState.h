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
		virtual bool AllowPassThroughRender() { return false; }

		StateStack* myStateStack = nullptr;
	};
}
