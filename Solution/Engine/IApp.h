#pragma once

namespace Slush
{
	class IApp
	{
	public:
		virtual void Initialize() {};
		virtual void Shutdown() {};

		virtual void Update() {};
		virtual void Render() {};
		virtual void RenderImGUI() {};
	};
}
