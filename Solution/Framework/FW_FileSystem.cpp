#include "FW_FileSystem.h"

#include "FW_Assert.h"

#include <windows.h>

namespace FW_FileSystem
{
	static FW_String ourDataFolderPath = "";
	void SetDataFolder(const char* aFolderName)
	{
		ourDataFolderPath = aFolderName;
	}

	void GetAbsoluteFilePath(const FW_String& aFilePath, FW_String& aFilePathOut)
	{
		aFilePathOut = ourDataFolderPath + aFilePath;
	}

	bool GetAllFilesFromRelativeDirectory(const char* aRelativeDirectory, FW_GrowingArray<FileInfo>& someOutFilePaths)
	{
		FW_ASSERT(strlen(aRelativeDirectory) + 3 < MAX_PATH, "Path to directory is too long");

		FW_String absolutDirectory = ourDataFolderPath;
		absolutDirectory += aRelativeDirectory;
		absolutDirectory += "/*";

		
		WIN32_FIND_DATA data;
		HANDLE filehandle = FindFirstFile(absolutDirectory.GetBuffer(), &data);

		if (filehandle == INVALID_HANDLE_VALUE)
			return false;

		do
		{
			FW_String name = data.cFileName;
			if (name == "." || name == "..")
				continue;

			FW_String relativePath = aRelativeDirectory;
			relativePath += "/";
			relativePath += name;

			if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				GetAllFilesFromRelativeDirectory(relativePath.GetBuffer(), someOutFilePaths);
			}
			else
			{
				FileInfo& info = someOutFilePaths.Add();
				info.myFileName = name;
				info.myRelativeFilePath = relativePath;
				GetAbsoluteFilePath(relativePath, info.myAbsoluteFilePath);
				info.myLastTimeModifiedLowbit = data.ftLastWriteTime.dwLowDateTime;
				info.myLastTimeModifiedHighbit = data.ftLastWriteTime.dwHighDateTime;

				RemoveFileExtention(name, info.myFileNameNoExtention);
			}
		} while (FindNextFile(filehandle, &data) != 0);

		if (GetLastError() != ERROR_NO_MORE_FILES)
			FW_ASSERT_ALWAYS("Something went wrong...");

		FindClose(filehandle);
		return true;
	}

	bool GetAllFilesFromAbsoluteDirectory(const char* aDirectory, FW_GrowingArray<FileInfo>& someOutFilePaths)
	{
		FW_ASSERT(strlen(aDirectory) + 3 < MAX_PATH, "Path to directory is too long");

		FW_String directory = aDirectory;
		directory += "/*";

		WIN32_FIND_DATA data;
		HANDLE filehandle = FindFirstFile(directory.GetBuffer(), &data);

		if (filehandle == INVALID_HANDLE_VALUE)
			return false;

		do
		{
			FW_String name = data.cFileName;
			if (name == "." || name == "..")
				continue;

			FW_String fullPath = aDirectory;
			fullPath += "/";
			fullPath += name;

			if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				GetAllFilesFromAbsoluteDirectory(fullPath.GetBuffer(), someOutFilePaths);
			}
			else
			{
				FileInfo& info = someOutFilePaths.Add();
				info.myFileName = name;
				info.myAbsoluteFilePath = fullPath;
				info.myLastTimeModifiedLowbit = data.ftLastWriteTime.dwLowDateTime;
				info.myLastTimeModifiedHighbit = data.ftLastWriteTime.dwHighDateTime;

				RemoveFileExtention(name, info.myFileNameNoExtention);
			}
		} while (FindNextFile(filehandle, &data) != 0);

		if (GetLastError() != ERROR_NO_MORE_FILES)
			FW_ASSERT_ALWAYS("Something went wrong...");

		FindClose(filehandle);
		return true;
	}

	void GetFileName(const FW_String& aFilePath, FW_String& aNameOut)
	{
		int findIndex = aFilePath.RFind("/");
		aNameOut = aFilePath.SubStr(findIndex + 1, aFilePath.Length());
	}

	void GetFileNameNoExtention(const FW_String& aFilePath, FW_String& aNameOut)
	{
		FW_String filename;
		GetFileName(aFilePath, filename);
		RemoveFileExtention(filename, aNameOut);
	}

	void RemoveFileName(const FW_String& aFilePath, FW_String& aFilePathOut)
	{
		int findIndex = aFilePath.RFind("/");
		aFilePathOut = aFilePath.SubStr(0, findIndex);
	}

	void GetFileExtention(const FW_String& aFilePath, FW_String& aExtentionOut)
	{
		int findIndex = aFilePath.RFind(".");
		aExtentionOut = aFilePath.SubStr(findIndex + 1, aFilePath.Length());
	}

	void RemoveFileExtention(const FW_String& aFilePath, FW_String& aNameOut)
	{
		int findIndex = aFilePath.RFind(".");
		aNameOut = aFilePath.SubStr(0, findIndex - 1);
	}

	bool GetFileInfo(const FW_String& aFilePath, FileInfo& aFileInfoOut)
	{
		FW_ASSERT(aFilePath.Length() + 3 < MAX_PATH, "Filepath is too long");

		WIN32_FIND_DATA data;
		HANDLE filehandle = FindFirstFile(aFilePath.GetBuffer(), &data);

		if (filehandle == INVALID_HANDLE_VALUE)
			return false;

		aFileInfoOut.myFileName = data.cFileName;
		aFileInfoOut.myAbsoluteFilePath = aFilePath;
		aFileInfoOut.myLastTimeModifiedLowbit = data.ftLastWriteTime.dwLowDateTime;
		aFileInfoOut.myLastTimeModifiedHighbit = data.ftLastWriteTime.dwHighDateTime;

		FindClose(filehandle);
		return true;
	}

	bool UpdateFileInfo(FileInfo& aFileInfo)
	{
		FileInfo newInfo;
		GetFileInfo(aFileInfo.myAbsoluteFilePath, newInfo);

		FILETIME oldTime;
		oldTime.dwLowDateTime = aFileInfo.myLastTimeModifiedLowbit;
		oldTime.dwHighDateTime = aFileInfo.myLastTimeModifiedHighbit;

		FILETIME newTime;
		newTime.dwLowDateTime = newInfo.myLastTimeModifiedLowbit;
		newTime.dwHighDateTime = newInfo.myLastTimeModifiedHighbit;

		aFileInfo.myLastTimeModifiedLowbit = newInfo.myLastTimeModifiedLowbit;
		aFileInfo.myLastTimeModifiedHighbit = newInfo.myLastTimeModifiedHighbit;

		return CompareFileTime(&oldTime, &newTime) != 0;
	}
		
	bool UpdateFileInfo(FW_GrowingArray<FileInfo>& someFiles)
	{
		bool somethingChanged = false;
		FileInfo newInfo;
		for (FileInfo& oldInfo : someFiles)
		{
			if (UpdateFileInfo(oldInfo))
				somethingChanged = true;
		}

		return somethingChanged;
	}

	void ReadEntireFile(const FW_String& aFilePath, FileContent& aFileContentOut)
	{
		FILE* file;
		fopen_s(&file, aFilePath.GetBuffer(), "rb");
		fseek(file, 0, SEEK_END);
		long fileSize = ftell(file);
		fseek(file, 0, SEEK_SET);

		unsigned char* string = new unsigned char[fileSize + 1];
		fread(string, fileSize, 1, file);
		fclose(file);

		string[fileSize] = 0;

		aFileContentOut.myContents = string;
		aFileContentOut.myFileSize = fileSize;
	}
}