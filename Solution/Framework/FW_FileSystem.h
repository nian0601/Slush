#pragma once

#include "FW_GrowingArray.h"
#include "FW_String.h"

namespace FW_FileSystem
{
	struct FileInfo
	{
		FW_String myFileName;
		FW_String myFileNameNoExtention;
		FW_String myRelativeFilePath;
		FW_String myAbsoluteFilePath;
		unsigned long myLastTimeModifiedLowbit;
		unsigned long myLastTimeModifiedHighbit;
	};

	// The Contents of the file will be freed when the FileContent
	// goes out of scope, be careful if you allocated temporary FileContents
	struct FileContent
	{
		FileContent(bool aAutoFreeContents = true) : myContents(nullptr), myAutoFreeContents(aAutoFreeContents) {}
		~FileContent() { if (myAutoFreeContents) delete myContents; }
		unsigned char* myContents;
		long myFileSize;
		bool myAutoFreeContents;
	};


	void SetDataFolder(const char* aFolderName);

	void GetAbsoluteFilePath(const FW_String& aFilePath, FW_String& aFilePathOut);
	bool GetAllFilesFromRelativeDirectory(const char* aDirectory, FW_GrowingArray<FileInfo>& someOutFilePaths);
	bool GetAllFilesFromAbsoluteDirectory(const char* aDirectory, FW_GrowingArray<FileInfo>& someOutFilePaths);

	void GetFileName(const FW_String& aFilePath, FW_String& aNameOut);
	void GetFileNameNoExtention(const FW_String& aFilePath, FW_String& aNameOut);
	void RemoveFileName(const FW_String& aFilePath, FW_String& aFilePathOut);
	void GetFileExtention(const FW_String& aFilePath, FW_String& aExtentionOut);
	void RemoveFileExtention(const FW_String& aFilePath, FW_String& aNameOut);
	bool GetFileInfo(const FW_String& aFilePath, FileInfo& aFileInfoOut);

	bool UpdateFileInfo(FileInfo& aFileInfo);
	bool UpdateFileInfo(FW_GrowingArray<FileInfo>& someFiles);

	void ReadEntireFile(const FW_String& aFilePath, FileContent& aFileContentOut);

	void TrimBeginAndEnd(FW_String& aLine);

	FW_String TakeFirstWord(FW_String& aLine);

	void SplitLineOnSpace(const FW_String& aLine, FW_GrowingArray<FW_String>& outWords);
	void SplitLine(const FW_String& aLine, const char* aSeperator, FW_GrowingArray<FW_String>& outWords);

	float GetFloat(const FW_String& aWord);
	int GetInt(const FW_String& aWord);
}