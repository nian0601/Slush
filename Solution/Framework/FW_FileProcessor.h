#pragma once

#include <mbstring.h>
#include <stdio.h>

#include "FW_Assert.h"

#define USING_RAM_STORAGE 1
#define USE_BINARY_FILE_PROCESSING 0

class FW_String;
class FW_FileProcessor
{
public:
	enum Flags
	{
		BINARY = 1 << 0,
		READ = 1 << 1,
		WRITE = 1 << 2,
	};

	FW_FileProcessor(const char* aFile, int someFlags);
	~FW_FileProcessor();

	void ReadRestOfLine(FW_String& aString);
	void Process(FW_String& aString);
	void Process(const FW_String& aString);
	void Process(float&& aValue);
	void Process(float& aValue);
	void Process(int&& aValue);
	void Process(int& aValue);

	void AddNewline();
	bool IsWriting() const { return (myFlags & WRITE) > 0; }
	bool IsReading() const { return (myFlags & READ) > 0; }

	bool AtEndOfFile() const;

private:
	bool IsOpen() const { return myStatus == 0; }

#if USE_BINARY_FILE_PROCESSING
	void ProcessRawData(void* someData, int aDataSize);
#endif

	FILE* myFile;
	int myFlags;
	int myStatus;
	const char* myFilePath; // Just for debugging

#if USING_RAM_STORAGE && USE_BINARY_FILE_PROCESSING
	unsigned char* myData;
	int myDataSize;
	int myCursorPosition;
#endif
};