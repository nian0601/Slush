#include "stdafx.h"
#include "Component.h"

namespace Slush
{
	bool Component::BaseData::Parse(AssetParser::Handle aRootHandle)
	{
		if (aRootHandle.IsReading())
		{
			AssetParser::Handle componentHandle = aRootHandle.ParseChildElement(myComponentDataName);
			if (componentHandle.IsValid())
			{
				myEnabled = true;

				int loadedVersion = 0;
				componentHandle.ParseOptionalIntField("version", loadedVersion, true);

				OnParse(componentHandle, static_cast<unsigned int>(loadedVersion));
				return static_cast<unsigned int>(loadedVersion) < myCurrentComponentVersion;
			}
		}
		else
		{
			if (myEnabled)
			{
				AssetParser::Handle componentHandle = aRootHandle.ParseChildElement(myComponentDataName);
				if (componentHandle.IsValid())
				{
					int versionToWrite = static_cast<int>(myCurrentComponentVersion);
					componentHandle.ParseOptionalIntField("version", versionToWrite, true);
					OnParse(componentHandle, myCurrentComponentVersion);
				}
			}
		}

		return false;
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
