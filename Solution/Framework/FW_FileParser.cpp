#include "FW_FileParser.h"
#include "FW_FileSystem.h"
#include "FW_Assert.h"

FW_FileParser::FW_FileParser(const char* aFile)
	: myFilePath(aFile)
{
	FW_String realFilePath;
	FW_FileSystem::GetAbsoluteFilePath(aFile, realFilePath);

	int result = fopen_s(&myFile, realFilePath.GetBuffer(), "r");
	FW_ASSERT(result == 0, "Failed to open file");
}

FW_FileParser::~FW_FileParser()
{
	fclose(myFile);
}

bool FW_FileParser::ReadLine(FW_String& outLine)
{
	char text[256];

	char* result = fgets(text, 256, myFile);
	if (result == NULL)
		return false;

	outLine = text;
	return true;
}

void FW_FileParser::TrimBeginAndEnd(FW_String& aLine) const
{
	FW_FileSystem::TrimBeginAndEnd(aLine);
}

FW_String FW_FileParser::TakeFirstWord(FW_String& aLine) const
{
	return FW_FileSystem::TakeFirstWord(aLine);
}

void FW_FileParser::SplitLineOnSpace(const FW_String& aLine, FW_GrowingArray<FW_String>& outWords) const
{
	FW_FileSystem::SplitLineOnSpace(aLine, outWords);
}

void FW_FileParser::SplitLine(const FW_String& aLine, const char* aSeperator, FW_GrowingArray<FW_String>& outWords) const
{
	FW_FileSystem::SplitLine(aLine, aSeperator, outWords);
}

float FW_FileParser::GetFloat(const FW_String& aWord) const
{
	return FW_FileSystem::GetFloat(aWord);
}

int FW_FileParser::GetInt(const FW_String& aWord) const
{
	return FW_FileSystem::GetInt(aWord);
}
