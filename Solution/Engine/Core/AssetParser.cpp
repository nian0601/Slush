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

	AssetParser::Handle AssetParser::Handle::ParseChildElement(const char* aElementName)
	{
		if (!IsValid())
		{
			SLUSH_ERROR("AssetParser: Tried to ParseChildElement '%s' on an invalid Handle", aElementName);
			return Handle();
		}

		if (myIsReading)
			return GetChildElement(aElementName);

		return AddChildElement(aElementName);
	}

	void AssetParser::Handle::ParseIntField(const char* aFieldName, int& aValue)
	{
		if (!IsValid())
		{
			SLUSH_ERROR("AssetParser: Tried to ParseIntField '%s' on an invalid Handle", aFieldName);
			return;
		}

		if (myIsReading)
			aValue = GetIntField(aFieldName);
		else
			WriteIntField(aFieldName, aValue);
	}

	void AssetParser::Handle::ParseFloatField(const char* aFieldName, float& aValue)
	{
		if (!IsValid())
		{
			SLUSH_ERROR("AssetParser: Tried to ParseFloatField '%s' on an invalid Handle", aFieldName);
			return;
		}

		if (myIsReading)
			aValue = GetFloatField(aFieldName);
		else
			WriteFloatField(aFieldName, aValue);
	}

	void AssetParser::Handle::ParseBoolField(const char* aFieldName, bool& aValue)
	{
		if (!IsValid())
		{
			SLUSH_ERROR("AssetParser: Tried to ParseBoolField '%s' on an invalid Handle", aFieldName);
			return;
		}

		if (myIsReading)
			aValue = GetBoolField(aFieldName);
		else
			WriteBoolField(aFieldName, aValue);
	}

	AssetParser::Handle AssetParser::Handle::GetChildElement(const char* aElementName) const
	{
		if (!IsValid())
			return Handle();

		Element* const* child = myElement->myChildElements.GetIfExists(aElementName);
		if (child)
			return Handle(*child);

		return Handle();
	}

	int AssetParser::Handle::GetIntField(const char* aFieldName) const
	{
		if (!IsValid())
		{
			SLUSH_ERROR("AssetParser: Tried to read Field '%s' from an invalid Handle", aFieldName);
			return 0;
		}

		Field* const* field = myElement->myFields.GetIfExists(aFieldName);
		if (!field)
		{
			SLUSH_ERROR("AssetParser: Didnt find Field '%s'", aFieldName);
			return 0;
		}

		return static_cast<int>(atoll((*field)->myRawData.GetBuffer()));
	}

	float AssetParser::Handle::GetFloatField(const char* aFieldName) const
	{
		if (!IsValid())
		{
			SLUSH_ERROR("AssetParser: Tried to read Field '%s' from an invalid Handle", aFieldName);
			return 0;
		}

		Field* const* field = myElement->myFields.GetIfExists(aFieldName);
		if (!field)
		{
			SLUSH_ERROR("AssetParser: Didnt find Field '%s'", aFieldName);
			return 0;
		}

		return static_cast<float>(atof((*field)->myRawData.GetBuffer()));
	}

	bool AssetParser::Handle::GetBoolField(const char* aFieldName) const
	{
		return GetIntField(aFieldName) == 1;
	}

	AssetParser::Handle AssetParser::Handle::AddChildElement(const char* aElementName)
	{
		if (!IsValid())
		{
			SLUSH_ERROR("AssetParser: Tried to '%s'-Element to an invalid Handle", aElementName);
			return Handle();
		}

		Element* child = new Element();
		child->myElementTypeName = aElementName;
		myElement->myChildElements[child->myElementTypeName] = child;

		const bool isReading = false;
		return Handle(child, isReading);
	}

	void AssetParser::Handle::WriteIntField(const char* aFieldName, int aValue)
	{
		if (!IsValid())
		{
			SLUSH_ERROR("AssetParser: Tried to write Int '%s' to an invalid Handle", aFieldName);
			return;
		}

		Field* field = new Field();
		field->myFieldName = aFieldName;
		field->myRawData += aValue;
		myElement->myFields[aFieldName] = field;
	}

	void AssetParser::Handle::WriteFloatField(const char* aFieldName, float aValue)
	{
		if (!IsValid())
		{
			SLUSH_ERROR("AssetParser: Tried to write Float '%s' to an invalid Handle", aFieldName);
			return;
		}

		Field* field = new Field();
		field->myFieldName = aFieldName;
		field->myRawData += aValue;
		myElement->myFields[aFieldName] = field;
	}

	void AssetParser::Handle::WriteBoolField(const char* aFieldName, bool aValue)
	{
		WriteIntField(aFieldName, aValue ? 1 : 0);
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
		myRootElement.myElementTypeName = line;
		myRootElement.Load(fileParser);

		return Handle(&myRootElement);
	}

	Slush::AssetParser::Handle AssetParser::StartWriting(const char* aRootElementType)
	{
		myRootElement.myElementTypeName = aRootElementType;
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
				child->myElementTypeName = words[0];
				child->Load(aFileParser);
				myChildElements[child->myElementTypeName] = child;
			}
			// Two means that its a field (key + value-pair)
			else if (words.Count() == 2)
			{
				Field* field = new Field();
				field->myFieldName = words[0];
				field->myRawData = words[1];
				myFields[field->myFieldName] = field;
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
		aFileProcessor.Process(myElementTypeName);
		aFileProcessor.AddNewline();

		aFileProcessor.Process("{");
		aFileProcessor.IncreaseIndentDepth();
		

		for (auto it = myFields.Begin(); it != myFields.End(); it = myFields.Next(it))
		{
			aFileProcessor.AddNewline();
			Field* field = it.Second();
			aFileProcessor.Process(field->myFieldName);
			aFileProcessor.Process(field->myRawData);
		}

		for (auto it = myChildElements.Begin(); it != myChildElements.End(); it = myChildElements.Next(it))
		{
			aFileProcessor.AddNewline();
			Element* child = it.Second();
			child->Save(aFileProcessor);
		}

		aFileProcessor.DecreaseIndentDepth();
		aFileProcessor.AddNewline();
		aFileProcessor.Process("}");
	}
}