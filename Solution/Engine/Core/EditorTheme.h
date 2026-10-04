#pragma once

// Editor (ImGui) look, modelled on Glöd's dashboard visual language.
// Only affects ImGui editor UI, not in-game UI built through UIBuilder.
namespace Slush
{
	namespace EditorTheme
	{
		enum Palette
		{
			IMGUI_DEFAULT,	// Stock ImGui dark style, for comparison
			COLD,			// Glöd neutrals + Slush blue-grey accent
			WARM,			// Glöd neutrals + Föhn orange accent
		};

		enum Font
		{
			NOTO_SANS,
			JETBRAINS_MONO,
		};

		// Requires an existing ImGui context
		void ApplyPalette(Palette aPalette);
		void LoadFonts(Font aFont);
	}
}
