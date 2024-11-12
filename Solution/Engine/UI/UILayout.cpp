#include "stdafx.h"
#include "UILayout.h"
#include <Core\AssetParser.h>

namespace Slush
{
	UILayout::UILayout(const char* aName)
		: myName(aName)
	{

	}

	void UILayout::SaveToDisk()
	{
		Slush::AssetParser parser;
		Slush::AssetParser::Handle rootHandle = parser.StartWriting("uilayout");

		{
			Slush::AssetParser::Handle buttonsHandle = rootHandle.ParseChildElement("buttons");
			for (Button& button : myButtons)
			{
				button.Parse(buttonsHandle);
			}
		}

		FW_String filepath = "Data/UILayouts/";
		filepath += myName;
		filepath += ".uilayout";
		parser.FinishWriting(filepath.GetBuffer());
	}

	void UILayout::Load(const char* aFilePath, bool /*aIsAbsolutePath*/)
	{
		Slush::AssetParser parser;
		Slush::AssetParser::Handle rootHandle = parser.Load(aFilePath);

		Slush::AssetParser::Handle buttonsHandle = rootHandle.ParseChildElement("buttons");

		myButtons.RemoveAll();
		Button& button = myButtons.Add();
		button.Parse(buttonsHandle);

		myIsDirty = true;
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
	}

}
