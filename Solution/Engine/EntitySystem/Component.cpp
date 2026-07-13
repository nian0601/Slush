#include "stdafx.h"
#include "Component.h"

namespace Slush
{
	void Component::BaseData::Parse(AssetParser::Handle aRootHandle)
	{
		if (aRootHandle.IsReading())
		{
			AssetParser::Handle componentHandle = aRootHandle.ParseChildElement(myComponentDataName);
			if (componentHandle.IsValid())
			{
				myEnabled = true;
				OnParse(componentHandle);
			}
		}
		else
		{
			if (myEnabled)
			{
				AssetParser::Handle componentHandle = aRootHandle.ParseChildElement(myComponentDataName);
				if (componentHandle.IsValid())
					OnParse(componentHandle);
			}
		}
	}

	void Component::BaseData::BuildUI()
	{
		FW_String label = myComponentLabel;
		label += " Component";

		if (myEnabled)
		{
			if (ImGui::CollapsingHeader(label.GetBuffer(), &myEnabled))
			{
				ImGui::Indent();
				OnBuildUI();
				ImGui::Unindent();
			}
		}
	}
}
