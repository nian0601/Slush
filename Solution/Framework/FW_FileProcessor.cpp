#include "FW_FileProcessor.h"
#include "FW_String.h"
#include "FW_Assert.h"
#include <stdio.h>
#include "FW_FileSystem.h"

FW_FileProcessor::FW_FileProcessor(const char* aFile, int someFlags)
	: myFilePath(aFile)
	, myFlags(someFlags)
{
	FW_String flags;
	if ((myFlags & WRITE) > 0)
	{
		flags += "wb";

#if USING_RAM_STORAGE && USE_BINARY_FILE_PROCESSING
		myDataSize = 1024 * 1024 * 1024;
		myData = new unsigned char[myDataSize];
		myCursorPosition = 0;
#endif
	}
	if ((myFlags& READ) > 0)
	{
		flags += "rb";

#if USING_RAM_STORAGE && USE_BINARY_FILE_PROCESSING
		FW_FileSystem::FileContent entireFile(false);
		FW_FileSystem::ReadEntireFile(aFile, entireFile);

		myData = entireFile.myContents;
		myDataSize = entireFile.myFileSize;
		myCursorPosition = 0;
#endif
	}

	myStatus = fopen_s(&myFile, myFilePath, flags.GetBuffer());
}

FW_FileProcessor::~FW_FileProcessor()
{
	if (IsOpen())
	{
#if USING_RAM_STORAGE && USE_BINARY_FILE_PROCESSING
		if (IsWriting())
		{
			size_t numWritten = fwrite(myData, sizeof(char), myCursorPosition, myFile);
			if (numWritten != unsigned int(myCursorPosition))
				perror(nullptr);
		}
#endif
		fclose(myFile);
	}

#if USING_RAM_STORAGE && USE_BINARY_FILE_PROCESSING
	delete myData;
#endif
}

void FW_FileProcessor::ReadRestOfLine(FW_String& aString)
{
	FW_ASSERT(IsOpen(), "Tried to process an unopened file");
	FW_ASSERT(IsReading(), "Tried to 'ReadRestOfLine' while in Writing-mode");

	if (IsReading())
	{
		char stringBuffer[256];
		fgets(stringBuffer, 256, myFile);
		aString = stringBuffer;
		aString.RemoveOne();
	}
}

void FW_FileProcessor::Process(FW_String& aString)
{
	FW_ASSERT(IsOpen(), "Tried to process an unopened file");

#if USE_BINARY_FILE_PROCESSING
	if (IsWriting())
	{
		if (aString.Empty())
		{
			int stringLenght = 0;
			ProcessRawData(&stringLenght, sizeof(int));
		}
		else
		{
			int stringLenght = aString.Length() + 2;
			ProcessRawData(&stringLenght, sizeof(int));
			ProcessRawData(aString.GetRawBuffer(), sizeof(char) * stringLenght);
		}
	}
	else if (IsReading())
	{
		int stringLenght = 0;
		ProcessRawData(&stringLenght, sizeof(int));
		
		if (stringLenght > 0)
		{
			char stringBuffer[64] = { "INVALID" };
			ProcessRawData(stringBuffer, sizeof(char) * stringLenght);
			aString = stringBuffer;
		}
	}
#else
	if (IsWriting())
	{
		fprintf(myFile, "%s ", aString.GetBuffer());
	}
	else
	{
		char stringBuffer[64] = { "INVALID" };
		fscanf_s(myFile, "%s ", stringBuffer, 64);
		aString = stringBuffer;
	}
#endif
}

void FW_FileProcessor::Process(const FW_String& aString)
{
	FW_ASSERT(IsOpen(), "Tried to process an unopened file");
	FW_ASSERT(IsWriting(), "Tried to read from disc into a const string");

	if (IsWriting())
	{
#if USE_BINARY_FILE_PROCESSING
		int stringLenght = aString.Length() + 2;
		ProcessRawData(&stringLenght, sizeof(int));
		ProcessRawData((void*)aString.GetBuffer(), sizeof(char) * stringLenght);
#else
		fprintf(myFile, "%s ", aString.GetBuffer());
#endif
	}
}

void FW_FileProcessor::Process(float&& aValue)
{
	FW_ASSERT(IsOpen(), "Tried to process an unopened file");
	FW_ASSERT(IsWriting(), "Tried to read from disc into a const string");

	if (IsWriting())
	{
#if USE_BINARY_FILE_PROCESSING
		ProcessRawData(&aValue, sizeof(float));
#else
		fprintf(myFile, "%f ", aValue);
#endif
	}
}

void FW_FileProcessor::Process(float& aValue)
{
	FW_ASSERT(IsOpen(), "Tried to process an unopened file");

#if USE_BINARY_FILE_PROCESSING
	if (IsWriting())
		ProcessRawData(&aValue, sizeof(float));
	else
		ProcessRawData(&aValue, sizeof(float));
#else
	if (IsWriting())
		fprintf(myFile, "%f ", aValue);
	else
		fscanf_s(myFile, "%f ", &aValue);
#endif
}

void FW_FileProcessor::Process(int&& aValue)
{
	FW_ASSERT(IsOpen(), "Tried to process an unopened file");
	FW_ASSERT(IsWriting(), "Tried to read from disc into a const string");

	if (IsWriting())
	{
#if USE_BINARY_FILE_PROCESSING
		ProcessRawData(&aValue, sizeof(int));
#else
		fprintf(myFile, "%d ", aValue);
#endif
	}
}

void FW_FileProcessor::Process(int& aValue)
{
	FW_ASSERT(IsOpen(), "Tried to process an unopened file");

#if USE_BINARY_FILE_PROCESSING
	if (IsWriting())
		ProcessRawData(&aValue, sizeof(int));
	else
		ProcessRawData(&aValue, sizeof(int));
#else
	if (IsWriting())
		fprintf(myFile, "%d ", aValue);
	else
		fscanf_s(myFile, "%d ", &aValue);
#endif
}

void FW_FileProcessor::AddNewline()
{
	FW_ASSERT(IsOpen(), "Tried to process an unopened file");

#if !USE_BINARY_FILE_PROCESSING
	fputs("\n", myFile);
#endif
}

bool FW_FileProcessor::AtEndOfFile() const
{
	return feof(myFile);
}

#if USE_BINARY_FILE_PROCESSING
void FW_FileProcessor::ProcessRawData(void* someData, int aDataSize)
{
	FW_ASSERT(IsOpen(), "Tried to process an unopened file");

	if (IsWriting())
	{
#if USING_RAM_STORAGE
		memcpy(&myData[myCursorPosition], someData, aDataSize);
		myCursorPosition += aDataSize;
#else
		fwrite(someData, aDataSize, 1, myFile);
		if (ferror(myFile))
		{
			perror("Error writing generic data");
			FW_ASSERT_ALWAYS("Something went wrong");
		}
#endif
	}
	else if (IsReading())
	{
#if USING_RAM_STORAGE
		memcpy(someData, &myData[myCursorPosition], aDataSize);
		myCursorPosition += aDataSize;
#else
		fread(someData, aDataSize, 1, myFile);
		if (ferror(myFile))
		{
			perror("Error reading generic data");
			FW_ASSERT_ALWAYS("Something went wrong");
		}
#endif
	}
}
#endif