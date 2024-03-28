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
			Handle();
			Handle(Element* aElement);

			bool IsValid() const { return myElement != nullptr; }
			Handle GetChildElement(const char* aElementName) const;
			int GetIntField(const char* aFieldName) const;
			float GetFloatField(const char* aFieldName) const;
			bool GetBoolField(const char* aFieldName) const;

			Handle AddChildElement(const char* aElementName);
			void WriteIntField(const char* aFieldName, int aValue);
			void WriteFloatField(const char* aFieldName, float aValue);
			void WriteBoolField(const char* aFieldName, bool aValue);

		private:
			Element* myElement = nullptr;
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