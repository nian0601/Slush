#include "FW_UnitTestSuite.h"
#include "FW_FileProcessor.h"
#include "FW_FileSystem.h"
#include "FW_GrowingArray.h"
#include "FW_Assert.h"
#include "FW_String.h"

#include <windows.h>
#include <stdio.h>

namespace FW_UnitTestSuite
{
	static const int ourLineBufferSize = 1024;

	static FW_String ourCurrentSuite;
	static FW_String ourCurrentTest;
	static int ourTestCount = 0;
	static int ourCheckCount = 0;
	static FW_GrowingArray<FW_String> ourFailureLines;
	static FW_String ourReportFilePath;
	static FW_String ourScratchFolder;

	void BeginRun(const char* aReportFilePath, const char* aScratchFolder)
	{
		ourReportFilePath = aReportFilePath;
		ourScratchFolder = aScratchFolder;

		ourCurrentSuite = "";
		ourCurrentTest = "";
		ourTestCount = 0;
		ourCheckCount = 0;
		ourFailureLines.RemoveAll();
	}

	void BeginSuite(const char* aSuiteName)
	{
		ourCurrentSuite = aSuiteName;
	}

	void BeginTest(const char* aTestName)
	{
		ourCurrentTest = aTestName;
		++ourTestCount;
	}

	bool RecordCheck(bool aPassed, const char* aFile, int aLine, const char* anExpression, const char* aMessage)
	{
		++ourCheckCount;
		if (aPassed)
			return true;

		// Filename only, __FILE__ is a full path
		const char* fileName = aFile;
		for (const char* character = aFile; *character != '\0'; ++character)
		{
			if (*character == '\\' || *character == '/')
				fileName = character + 1;
		}

		char line[ourLineBufferSize];
		_snprintf_s(line, ourLineBufferSize, _TRUNCATE, "FAIL %s::%s  %s(%d): %s -- %s",
			ourCurrentSuite.GetBuffer(), ourCurrentTest.GetBuffer(), fileName, aLine, anExpression, aMessage);

		ourFailureLines.Add(FW_String(line));
		return false;
	}

	bool EndRun()
	{
		const bool passed = ourFailureLines.Count() == 0;

		char summary[ourLineBufferSize];
		_snprintf_s(summary, ourLineBufferSize, _TRUNCATE, "UNITTEST_RESULT: %s (%d failed / %d checks, %d tests)",
			passed ? "PASS" : "FAIL", ourFailureLines.Count(), ourCheckCount, ourTestCount);

		FILE* file = nullptr;
		fopen_s(&file, ourReportFilePath.GetBuffer(), "w");
		if (file)
			fprintf(file, "%s\n", summary);

		OutputDebugStringA(summary);
		OutputDebugStringA("\n");

		for (int i = 0; i < ourFailureLines.Count(); ++i)
		{
			if (file)
				fprintf(file, "%s\n", ourFailureLines[i].GetBuffer());

			OutputDebugStringA(ourFailureLines[i].GetBuffer());
			OutputDebugStringA("\n");
		}

		if (file)
			fclose(file);

		if (!passed && IsDebuggerPresent())
			FW_DEBUG_BREAK;

		return passed;
	}

	void TestFileProcessor()
	{
		FW_String scratchPath = ourScratchFolder;
		scratchPath += "fileprocessor_test.output";

		float floatWrite = 123.f;
		float floatWrite2 = 567.f;
		int intWrite = 321;
		FW_String stringWrite = "Apa_123";
		FW_String stringWrite2 = "This is a long string";
		bool boolWrite = false;
		bool boolWrite2 = true;

		{
			FW_FileProcessor processor(scratchPath.GetBuffer(), FW_FileProcessor::WRITE);

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
			FW_FileProcessor processor(scratchPath.GetBuffer(), FW_FileProcessor::READ);
			
			processor.Process(floatRead);
			processor.Process(floatRead2);
			processor.Process(intRead);
			processor.Process(stringRead);
			processor.ReadRestOfLine(stringRead2);
			processor.Process(boolRead);
			processor.Process(boolRead2);
		}

		FW_TEST_CHECK(floatWrite == floatRead, "float round-trip");
		FW_TEST_CHECK(floatWrite2 == floatRead2, "second float round-trip");
		FW_TEST_CHECK(intWrite == intRead, "int round-trip");
		FW_TEST_CHECK(stringWrite == stringRead, "string round-trip");
		FW_TEST_CHECK(stringWrite2 == stringRead2, "rest-of-line string round-trip");
		FW_TEST_CHECK(boolWrite == boolRead, "bool round-trip");
		FW_TEST_CHECK(boolWrite2 == boolRead2, "second bool round-trip");
	}

	void TestCreateFolderIfNecessary()
	{
		// Remove the tree a previous run left behind (deepest first), so the creation below is actually tested
		const char* foldersToRemove[] = { "createfolder_test/nested/deeper", "createfolder_test/nested", "createfolder_test" };
		for (const char* folder : foldersToRemove)
		{
			FW_String folderToRemove = ourScratchFolder;
			folderToRemove += folder;
			RemoveDirectoryA(folderToRemove.GetBuffer());
		}

		FW_String rootFolder = ourScratchFolder;
		rootFolder += "createfolder_test";
		FW_TEST_REQUIRE(!FW_FileSystem::DirectoryExists(rootFolder), "Expected the previous run's folders to be removed");

		FW_String filePath = ourScratchFolder;
		filePath += "createfolder_test/nested/deeper/file.txt";
		FW_FileSystem::CreateFolderIfNecessary(filePath);

		FW_String folderPath = ourScratchFolder;
		folderPath += "createfolder_test/nested/deeper";
		FW_TEST_CHECK(FW_FileSystem::DirectoryExists(folderPath), "Expected every folder along the path to be created");
		FW_TEST_CHECK(!FW_FileSystem::DirectoryExists(filePath), "Expected the trailing filename not to become a folder");

		// Already existing folders are fine
		FW_FileSystem::CreateFolderIfNecessary(folderPath);
		FW_TEST_CHECK(FW_FileSystem::DirectoryExists(folderPath), "Expected an existing folder to stay");
	}

	void TestStringLength()
	{
		// Length() is the index of the last character, not the count. Callers rely on this together with SubStr's inclusive end
		FW_String text = "abc";
		FW_TEST_CHECK(text.Length() == 2, "Expected Length() to be the last index, one less than the character count");
		FW_TEST_CHECK(text[text.Length()] == 'c', "Expected [Length()] to be the last character");
		FW_TEST_CHECK(text.SubStr(1, text.Length()) == "bc", "Expected SubStr(i, Length()) to run to the end");

		FW_String empty;
		FW_TEST_CHECK(empty.Length() == -1, "Expected Length() of an empty string to be -1");
		FW_TEST_CHECK(empty.Empty(), "Expected Empty() to be true for an empty string");
	}

	void RunFrameworkTests()
	{
		BeginSuite("Framework");
		FW_RUN_TEST(TestFileProcessor);
		FW_RUN_TEST(TestCreateFolderIfNecessary);
		FW_RUN_TEST(TestStringLength);
	}
}
