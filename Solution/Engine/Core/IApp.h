#pragma once

namespace Slush
{
	class IApp
	{
	public:
		virtual void Initialize() {};
		virtual void Shutdown() {};

		virtual void Update() {};

		// Not dead despite ActionGame's override being empty - BossMonster (main.cpp) has no
		// game-rendering IAppLayout and uses this as its only render entry point, wrapping
		// StartOffscreenBuffer()/EndOffscreenBuffer() around its draws.
		virtual void Render() {};
	};
}
