#include "FW_UnitTestSuite.h"
#include "FW_FileProcessor.h"
#include "FW_Assert.h"

#include <windows.h>
#include <string>
#include "FW_String.h"

namespace FW_UnitTestSuite
{
	void ReplaceAllOccurancesInString(std::string& aString, const std::string& aSomethingToReplace, const std::string& aNewString)
	{
		// Get the first occurrence
		size_t pos = aString.find(aSomethingToReplace);

		// Repeat till end is reached
		while (pos != std::string::npos)
		{
			// Replace this occurrence of Sub String
			aString.replace(pos, aSomethingToReplace.size(), aNewString);
			// Get the next occurrence from the current position
			pos = aString.find(aSomethingToReplace, pos + aNewString.size());
		}
	}

	void TestFileProcessor()
	{
		char fileBuffer[128];
		GetModuleFileNameA(NULL, fileBuffer, 128);

		std::string assetPath;
		assetPath.append(fileBuffer);
		assetPath.erase(assetPath.rfind("\\"), std::string::npos);
		ReplaceAllOccurancesInString(assetPath, "\\", "/");
		assetPath.append("/temp/fileProcessorTests.output");


		float floatWrite = 123.f;
		float floatWrite2 = 567.f;
		int intWrite = 321;
		FW_String stringWrite = "Apa_123";
		FW_String stringWrite2 = "This is a long string";
		bool boolWrite = false;
		bool boolWrite2 = true;

		{
			FW_FileProcessor processor(assetPath.c_str(), FW_FileProcessor::WRITE);

			processor.Process(floatWrite);
			processor.Process(floatWrite2);
			processor.AddNewline();

			processor.Process(intWrite);

			processor.Process(stringWrite);
			processor.AddNewline();

			processor.Process(stringWrite2);
			processor.AddNewline();

			processor.Process(boolWrite);
			processor.Process(boolWrite2);
		}


		float floatRead = 0.f;
		float floatRead2 = 0.f;
		int intRead = 0;
		FW_String stringRead;
		FW_String stringRead2;
		bool boolRead = true;
		bool boolRead2 = false;

		{
			FW_FileProcessor processor(assetPath.c_str(), FW_FileProcessor::READ);
			
			processor.Process(floatRead);
			processor.Process(floatRead2);
			processor.Process(intRead);
			processor.Process(stringRead);
			processor.ReadRestOfLine(stringRead2);
			processor.Process(boolRead);
			processor.Process(boolRead2);
		}

		FW_ASSERT(floatWrite == floatRead);
		FW_ASSERT(floatWrite2 == floatRead2);
		FW_ASSERT(intWrite == intRead);
		FW_ASSERT(stringWrite == stringRead);
		FW_ASSERT(stringWrite2 == stringRead2);
		FW_ASSERT(boolWrite == boolRead);
		FW_ASSERT(boolWrite2 == boolRead2);
	}

	void RunTests()
	{
		TestFileProcessor();
	}
}