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
	if (aLine.Empty() == true)
		return;

	int begin = 0;
	while (aLine[begin] == ' ' || aLine[begin] == '\t' || aLine[begin] == '\n')
		++begin;

	int end = aLine.Length();
	if (end > begin)
	{
		while (aLine[end] == ' ' || aLine[end] == '\t' || aLine[end] == '\n')
			--end;
	}

	if (begin != 0 || end != aLine.Length())
		aLine = aLine.SubStr(begin, end);
}

FW_String FW_FileParser::TakeFirstWord(FW_String& aLine) const
{
	int begin = 0;
	int end = aLine.Find(" ", begin);
	if (end == -1)
		return aLine;

	FW_String word = aLine.SubStr(begin, end - 1);
	aLine = aLine.SubStr(end+1, aLine.Length());
	return word;
}

void FW_FileParser::SplitLineOnSpace(const FW_String& aLine, FW_GrowingArray<FW_String>& outWords) const
{
	outWords.RemoveAll();

	int begin = 0;
	int end = aLine.Find(" ", begin);
	if (end == -1)
	{
		outWords.Add(aLine);
		return;
	}

	while (end != -1)
	{
		outWords.Add(aLine.SubStr(begin, end-1));

		begin = end+1;
		end = aLine.Find(" ", begin);
	}

	outWords.Add(aLine.SubStr(begin, aLine.Length()));
}

void FW_FileParser::SplitLine(const FW_String& aLine, const char* aSeperator, FW_GrowingArray<FW_String>& outWords) const
{
	outWords.RemoveAll();

	int begin = 0;
	int end = aLine.Find(aSeperator, begin);
	if (end == -1)
	{
		outWords.Add(aLine);
		return;
	}

	while (end != -1)
	{
		outWords.Add(aLine.SubStr(begin, end - 1));

		begin = end + 1;
		end = aLine.Find(aSeperator, begin);
	}

	outWords.Add(aLine.SubStr(begin, aLine.Length()));
}

float FW_FileParser::GetFloat(const FW_String& aWord) const
{
	return static_cast<float>(atof(aWord.GetBuffer()));
}

int FW_FileParser::GetInt(const FW_String& aWord) const
{
	return static_cast<int>(atoll(aWord.GetBuffer()));
}
