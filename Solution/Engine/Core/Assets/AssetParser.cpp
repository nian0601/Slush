#include "stdafx.h"
#include "AssetParser.h"
#include <FW_FileParser.h>
#include <FW_FileSystem.h>
#include <FW_FileProcessor.h>

namespace Slush
{
	AssetParser::Handle::Handle(Element* aElement, bool aIsReading)
	{
		myElement = aElement;
		myIsReading = aIsReading;
	}

	Slush::AssetParser::Handle AssetParser::Handle::GetChildElementAtIndex(int aIndex)
	{
		FW_ASSERT(myIsReading, "GetChildElementAtIndex is only valid while reading files");
		if (!IsValid())
			return Handle();

		if (Element* child = myElement->GetChildElementByIndex(aIndex))
			return Handle(child);

		return Handle();
	}

	int AssetParser::Handle::GetNumChildElements()
	{
		FW_ASSERT(myIsReading, "GetNumChildElements is only valid while reading files");
		if (!IsValid())
			return 0;

		return myElement->GetNumChildElements();
	}

	AssetParser::Handle AssetParser::Handle::ParseChildElement(const char* aElementName)
	{
		if (!ValidateField(aElementName))
			return AssetParser::Handle();

		if (myIsReading)
			return GetChildElement(aElementName);

		return AddChildElement(aElementName);
	}

	void AssetParser::Handle::ParseIntField(const char* aFieldName, int& aValue)
	{
		if (!ValidateField(aFieldName))
			return;

		if (myIsReading)
			GetIntField(aFieldName, aValue);
		else
			WriteIntField(aFieldName, aValue);
	}

	void AssetParser::Handle::ParseFloatField(const char* aFieldName, float& aValue)
	{
		if (!ValidateField(aFieldName))
			return;

		if (myIsReading)
			GetFloatField(aFieldName, aValue);
		else
			WriteFloatField(aFieldName, aValue);
	}

	void AssetParser::Handle::ParseBoolField(const char* aFieldName, bool& aValue)
	{
		if (!ValidateField(aFieldName))
			return;

		if (myIsReading)
			GetBoolField(aFieldName, aValue);
		else
			WriteBoolField(aFieldName, aValue);
	}

	void AssetParser::Handle::ParseStringField(const char* aFieldName, FW_String& aValue)
	{
		if (!ValidateField(aFieldName))
			return;

		if (myIsReading)
			GetStringField(aFieldName, aValue);
		else
			WriteStringField(aFieldName, aValue);
	}

	void AssetParser::Handle::ParseVec2iField(const char* aFieldName, Vector2i& aValue)
	{
		if (!ValidateField(aFieldName))
			return;

		Handle fieldHandle = ParseChildElement(aFieldName);

		fieldHandle.ParseIntField("x", aValue.x);
		fieldHandle.ParseIntField("y", aValue.y);
	}

	void AssetParser::Handle::ParseVec2fField(const char* aFieldName, Vector2f& aValue)
	{
		if (!ValidateField(aFieldName))
			return;

		Handle fieldHandle = ParseChildElement(aFieldName);

		fieldHandle.ParseFloatField("x", aValue.x);
		fieldHandle.ParseFloatField("y", aValue.y);
	}

	bool AssetParser::Handle::ValidateField(const char* aFieldName)
	{
		if (IsValid())
			return true;

		SLUSH_ERROR("AssetParser: Tried to parse field '%s' on an invalid Handle", aFieldName);
		return false;
	}

	AssetParser::Handle AssetParser::Handle::GetChildElement(const char* aElementName) const
	{
		if (!IsValid())
			return Handle();

		if (Element* child = myElement->GetChildElement(aElementName))
			return Handle(child);

		return Handle();
	}

	bool AssetParser::Handle::GetIntField(const char* aFieldName, int& aValue) const
	{
		if (Field* field = myElement->GetField(aFieldName))
		{
			aValue = static_cast<int>(atoll(field->myRawData.GetBuffer()));
			return true;
		}

		return false;
	}

	bool AssetParser::Handle::GetFloatField(const char* aFieldName, float& aValue) const
	{
		if (Field* field = myElement->GetField(aFieldName))
		{
			aValue = static_cast<float>(atof(field->myRawData.GetBuffer()));
			return true;
		}

		return false;
	}

	bool AssetParser::Handle::GetBoolField(const char* aFieldName, bool& aValue) const
	{
		int boolAsInt = 0;
		if (GetIntField(aFieldName, boolAsInt))
		{
			aValue = boolAsInt == 1;
			return true;
		}
		
		return false;
	}

	bool AssetParser::Handle::GetStringField(const char* aFieldName, FW_String& aValue) const
	{
		if (Field* field = myElement->GetField(aFieldName))
		{
			aValue = field->myRawData;
			return true;
		}

		return false;
	}

	AssetParser::Handle AssetParser::Handle::AddChildElement(const char* aElementName)
	{
		Element* child = myElement->AddChildElement(aElementName);

		const bool isReading = false;
		return Handle(child, isReading);
	}

	void AssetParser::Handle::WriteIntField(const char* aFieldName, int aValue)
	{
		Field* field = myElement->AddField(aFieldName);
		field->myRawData += aValue;
	}

	void AssetParser::Handle::WriteFloatField(const char* aFieldName, float aValue)
	{
		Field* field = myElement->AddField(aFieldName);
		field->myRawData += aValue;
	}

	void AssetParser::Handle::WriteBoolField(const char* aFieldName, bool aValue)
	{
		WriteIntField(aFieldName, aValue ? 1 : 0);
	}

	void AssetParser::Handle::WriteStringField(const char* aFieldName, FW_String& aValue)
	{
		Field* field = myElement->AddField(aFieldName);
		field->myRawData += aValue;
	}

	//////////////////////////////////////////////////////////////////////////

	AssetParser::Handle AssetParser::Load(const char* aFile)
	{
		FW_FileParser fileParser(aFile);
		FW_String line;

		if (!fileParser.ReadLine(line))
		{
			SLUSH_ERROR("AssetParser: Failed to read first line in file %s", fileParser.GetFilePath().GetBuffer());
			return Handle();
		}

		fileParser.TrimBeginAndEnd(line);
		myRootElement.myElementName = line;
		myRootElement.Load(fileParser);

		return Handle(&myRootElement);
	}

	Slush::AssetParser::Handle AssetParser::StartWriting(const char* aRootElementName)
	{
		myRootElement.myElementName = aRootElementName;
		const bool isReading = false;
		return Handle(&myRootElement, isReading);
	}

	void AssetParser::FinishWriting(const char* aFile)
	{
		FW_String filepath = aFile;
		FW_FileSystem::GetAbsoluteFilePath(filepath, filepath);

		FW_FileProcessor processor(filepath.GetBuffer(), FW_FileProcessor::WRITE);
		myRootElement.Save(processor);
	}

	AssetParser::Element::~Element()
	{
		myFields.DeleteAll();
		myChildElements.DeleteAll();
	}

	void AssetParser::Element::Load(FW_FileParser& aFileParser)
	{
		FW_String line;
		aFileParser.ReadLine(line);
		aFileParser.TrimBeginAndEnd(line);

		if (line != "{")
		{
			SLUSH_ERROR("AssetParser: Malformed Element, expected '{' but found '%s', in %s", line.GetBuffer(), aFileParser.GetFilePath().GetBuffer());
			return;
		}

		while (aFileParser.ReadLine(line))
		{
			aFileParser.TrimBeginAndEnd(line);

			FW_GrowingArray<FW_String> words;
			aFileParser.SplitLineOnSpace(line, words);

			// Only one word means that its the start of a child-element
			if (words.Count() == 1)
			{
				if (words[0] == "}")
					return;

				Element* child = new Element();
				child->myElementName = words[0];
				child->Load(aFileParser);
				myChildElements.Add(child);
			}
			// Two means that its a field (key + value-pair)
			else if (words.Count() == 2)
			{
				Field* field = new Field();
				field->myFieldName = words[0];
				field->myRawData = words[1];
				myFields.Add(field);
			}
			// Anything else is invalid formatting
			else
			{
				SLUSH_ERROR("AssetParser: Malformed Element, found '%i' words on one line but only 1 or 2 is expected, in %s", words.Count(), aFileParser.GetFilePath().GetBuffer());
			}
		}

		SLUSH_ERROR("AssetParser: Malformed Element, reached eof without encountering '}', in %s", aFileParser.GetFilePath().GetBuffer());
	}

	void AssetParser::Element::Save(FW_FileProcessor& aFileProcessor)
	{
		aFileProcessor.Process(myElementName);
		aFileProcessor.AddNewline();

		aFileProcessor.Process("{");
		aFileProcessor.IncreaseIndentDepth();
		
		for (Field* field : myFields)
		{
			aFileProcessor.AddNewline();
			aFileProcessor.Process(field->myFieldName);
			aFileProcessor.Process(field->myRawData);
		}

		for (Element* child : myChildElements)
		{
			aFileProcessor.AddNewline();
			child->Save(aFileProcessor);
		}

		aFileProcessor.DecreaseIndentDepth();
		aFileProcessor.AddNewline();
		aFileProcessor.Process("}");
	}

	AssetParser::Field* AssetParser::Element::AddField(const FW_String& aFieldName)
	{
		Field* field = new Field();
		field->myFieldName = aFieldName;
		myFields.Add(field);

		return field;
	}

	AssetParser::Element* AssetParser::Element::AddChildElement(const FW_String& aElementName)
	{
		Element* child = new Element();
		child->myElementName = aElementName;
		myChildElements.Add(child);

		return child;
	}

	AssetParser::Field* AssetParser::Element::GetField(const FW_String& aFieldName)
	{
		for (Field* field : myFields)
		{
			if (field->myFieldName == aFieldName)
				return field;
		}

		return nullptr;
	}

	AssetParser::Element* AssetParser::Element::GetChildElement(const FW_String& aElementName)
	{
		for (Element* child : myChildElements)
		{
			if (child->myElementName == aElementName)
				return child;
		}

		return nullptr;
	}
}