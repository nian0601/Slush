#pragma once
#include <Core\AssetParser.h>

namespace Slush
{
	struct UILayout
	{
		UILayout(const char* aName);

		void SaveToDisk();
		void Load(const char* aFilePath, bool aIsAbsolutePath);

		struct Button
		{
			void Parse(Slush::AssetParser::Handle aParserHandle);

			Vector2i myPosition;
			Vector2i mySize;
			int myColor;
			int myHoverColor;
			int myPressedColor;
			FW_String myIdentifier;
		};
		FW_GrowingArray<Button> myButtons;

		bool myIsDirty = true;
		FW_String myName;
	};
}
