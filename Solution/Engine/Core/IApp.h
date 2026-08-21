#pragma once

namespace Slush
{
	class IApp
	{
	public:
		virtual void Initialize() {};
		virtual void Shutdown() {};

		virtual void Update() {};

		// Not dead despite ActionGame's override being empty - BossMonster (main.cpp) and TopDownGame
		// (main.cpp) have no game-rendering IAppLayout and use this as their only render entry point,
		// wrapping StartOffscreenBuffer()/EndOffscreenBuffer() around their draws.
		virtual void Render() {};
	};
}
