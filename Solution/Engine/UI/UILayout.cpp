#include "stdafx.h"
#include "UILayout.h"
#include <Core/Assets/AssetParser.h>

namespace Slush
{
	void UILayout::OnParse(AssetParser::Handle aRootHandle)
	{
		Slush::AssetParser::Handle buttonsHandle = aRootHandle.ParseChildElement("buttons");
		
		if (aRootHandle.IsReading())
		{
			int buttonCount = 0;
			buttonsHandle.ParseIntField("count", buttonCount);

			myButtons.RemoveAll();
			for (int i = 0; i < buttonCount; ++i)
			{
				Button& button = myButtons.Add();
				button.Parse(buttonsHandle);
			}
		}
		else
		{
			int buttonCount = myButtons.Count();
			buttonsHandle.ParseIntField("count", buttonCount);
			for (Button& button : myButtons)
			{
				button.Parse(buttonsHandle);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////

	UILayout::Button::Button()
	{
		myIdentifier = "<Button>";


		// https://coolors.co/db2b39-29335c-f3a712-f0cea0-534d41
		myColor = 0xff5B5548;
		FW_ARGB_To_RGBAFloat(myColor, myFloatColor);

		myHoverColor = 0xff454036;
		FW_ARGB_To_RGBAFloat(myHoverColor, myHoverFloatColor);

		myPressedColor = 0xff726A5A;
		FW_ARGB_To_RGBAFloat(myPressedColor, myPressedFloatColor);
	}

	void UILayout::Button::Parse(Slush::AssetParser::Handle aParserHandle)
	{
		Slush::AssetParser::Handle handle = aParserHandle.ParseChildElement("button");

		handle.ParseVec2iField("position", myPosition);
		handle.ParseVec2iField("size", mySize);
		handle.ParseIntField("color", myColor);
		handle.ParseIntField("hoverColor", myHoverColor);
		handle.ParseIntField("pressedColor", myPressedColor);
		handle.ParseStringField("identifier", myIdentifier);

		if (aParserHandle.IsReading())
		{
			FW_ARGB_To_RGBAFloat(myColor, myFloatColor);
			FW_ARGB_To_RGBAFloat(myHoverColor, myHoverFloatColor);
			FW_ARGB_To_RGBAFloat(myPressedColor, myPressedFloatColor);
		}
	}

}
