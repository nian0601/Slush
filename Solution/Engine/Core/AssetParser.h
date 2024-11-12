#pragma once

#include "FW_Hashmap.h"
#include "FW_GrowingArray.h"

class FW_FileParser;
class FW_FileProcessor;

namespace Slush
{
	class AssetParser
	{
		struct Element;
	public:
		struct Handle
		{
		public:
			Handle(){}
			Handle(Element* aElement, bool aIsReading = true);

			bool IsValid() const { return myElement != nullptr; }
			bool IsReading() const { return myIsReading; }

			Handle ParseChildElement(const char* aElementName);
			void ParseIntField(const char* aFieldName, int& aValue);
			void ParseFloatField(const char* aFieldName, float& aValue);
			void ParseBoolField(const char* aFieldName, bool& aValue);
			void ParseStringField(const char* aFieldName, FW_String& aValue);

			void ParseVec2iField(const char* aFieldName, Vector2i& aValue);
			void ParseVec2fField(const char* aFieldName, Vector2f& aValue);

		private:
			bool ValidateField(const char* aFieldName);

			Handle GetChildElement(const char* aElementName) const;
			bool GetIntField(const char* aFieldName, int& aValue) const;
			bool GetFloatField(const char* aFieldName, float& aValue) const;
			bool GetBoolField(const char* aFieldName, bool& aValue) const;
			bool GetStringField(const char* aFieldName, FW_String& aValue) const;

			Handle AddChildElement(const char* aElementName);
			void WriteIntField(const char* aFieldName, int aValue);
			void WriteFloatField(const char* aFieldName, float aValue);
			void WriteBoolField(const char* aFieldName, bool aValue);
			void WriteStringField(const char* aFieldName, FW_String& aValue);

			Element* myElement = nullptr;
			bool myIsReading = true;
		};

		Handle Load(const char* aFile);
		Handle StartWriting(const char* aRootElementType);
		void FinishWriting(const char* aFile);

	private:
		struct Field
		{
			FW_String myFieldName;
			FW_String myRawData;
		};
		struct Element 
		{
			~Element();
			void Load(FW_FileParser& aFileParser);
			void Save(FW_FileProcessor& aFileProcessor);

			FW_String myFilePath;
			FW_String myElementTypeName;
			FW_Hashmap<FW_String, Field*> myFields;
			FW_Hashmap<FW_String, Element*> myChildElements;
		};
		Element myRootElement;
	};
};