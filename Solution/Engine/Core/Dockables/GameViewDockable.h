#pragma once

#include <FW_String.h>
#include <FW_FileSystem.h>
#include <FW_Rect.h>

#include "Dockable.h"

namespace Slush
{
	class GameViewDockable : public Slush::Dockable
	{
	public:
		const char* GetName() const override { return "Game View"; }

	protected:
		void OnBuildUI() override;

		// Draws ImGui on top of the game image, in screen space. The game image is a scaled-down render target, so
		// small text rendered into the game itself gets blurry - editor info text drawn here stays at native resolution.
		virtual void OnBuildOverlay(const Rectf& /*aGameViewRect*/) {}

		// Text on a dark backdrop, so it stays readable over any game content. aColor is an ImU32.
		static void DrawOverlayText(const Vector2f& aScreenPosition, const char* aText, unsigned int aColor);
	};
}