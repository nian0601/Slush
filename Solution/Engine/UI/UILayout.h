#pragma once
#include "Core/Assets/AssetParser.h"

namespace Slush
{
	struct UILayout
	{
		// https://coolors.co/db2b39-29335c-f3a712-f0cea0-534d41
		UILayout(const char* aName);

		static const char* GetAssetTypeName() { return "UI Layout"; }
		static const char* GetAssetTypeExtention() { return "uilayout"; }
		static const char* GetAssetTypeFolder() { return "data/uilayouts"; }

		void SaveToDisk();
		void Load(const char* aFilePath, bool aIsAbsolutePath);

		struct Button
		{
			Button();

			void Parse(Slush::AssetParser::Handle aParserHandle);

			Vector2i myPosition;
			Vector2i mySize;
			int myColor;
			float myFloatColor[4];
			int myHoverColor;
			float myHoverFloatColor[4];
			int myPressedColor;
			float myPressedFloatColor[4];
			FW_String myIdentifier;
		};
		FW_GrowingArray<Button> myButtons;

		bool myIsDirty = true;
		FW_String myName;
	};
}
