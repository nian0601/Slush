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

		int windowFlags = 0;
		if (myShowMenuBar)
			windowFlags |= ImGuiWindowFlags_MenuBar;

		if (myEnableHorizontalScrolling)
			windowFlags |= ImGuiWindowFlags_HorizontalScrollbar;

		if (ImGui::Begin(myUniqueIDName.GetBuffer(), NULL, windowFlags))
			OnBuildUI();

		ImGui::End();
	}
}