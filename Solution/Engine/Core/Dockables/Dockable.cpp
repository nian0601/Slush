#include "stdafx.h"
#include "Dockable.h"

namespace Slush
{
	void Dockable::Update()
	{
		if (myUniqueIDName.Empty())
		{
			myUniqueIDName = GetName();

			// Switch to "##"?
			// "##" = Use Name + ID to construct the ImGUI-ID ('myDockableID' needs to be unique across instances of the same dockable)
			// "###" = Use only the ID to construct the ImGUI-ID ('myDockableID' needs to be unique across *all* dockables
			myUniqueIDName += "###";
			myUniqueIDName += myDockableID;
		}

		OnUpdate();

		// Only takes effect the very first time this window has no saved ini entry. Safe to call every
		// frame - ImGui clears FirstUseEver eligibility on the window the first time it's applied, so this
		// becomes a no-op (never fights the user's manual resize) for the rest of the window's lifetime.
		if (myDefaultSize.x > 0.0f && myDefaultSize.y > 0.0f)
			ImGui::SetNextWindowSize(ImVec2(myDefaultSize.x, myDefaultSize.y), ImGuiCond_FirstUseEver);

		int windowFlags = 0;
		if (myShowMenuBar)
			windowFlags |= ImGuiWindowFlags_MenuBar;

		if (myEnableHorizontalScrolling)
			windowFlags |= ImGuiWindowFlags_HorizontalScrollbar;

		if (ImGui::Begin(myUniqueIDName.GetBuffer(), myIsClosable ? &myIsOpen : NULL, windowFlags))
			OnBuildUI();

		ImGui::End();
	}
}