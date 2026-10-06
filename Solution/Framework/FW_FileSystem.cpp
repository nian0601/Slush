#include "FW_FileSystem.h"

#include "FW_Assert.h"

#include <windows.h>

namespace FW_FileSystem
{
	// The one place a FileInfo gets built from the OS's file data, so every FileInfo carries the same fields
	static void FillFileInfo(const FW_String& aFileName, const FW_String& anAbsoluteFilePath, const WIN32_FIND_DATA& someData, FileInfo& aFileInfoOut)
	{
		aFileInfoOut.myFileName = aFileName;
		aFileInfoOut.myAbsoluteFilePath = anAbsoluteFilePath;
		aFileInfoOut.myLastTimeModifiedLowbit = someData.ftLastWriteTime.dwLowDateTime;
		aFileInfoOut.myLastTimeModifiedHighbit = someData.ftLastWriteTime.dwHighDateTime;

		RemoveFileExtention(aFileName, aFileInfoOut.myFileNameNoExtention);
	}

	void GetExecutableDirectory(FW_String& anOut)
	{
		char buffer[MAX_PATH];
		const DWORD length = GetModuleFileNameA(NULL, buffer, MAX_PATH);
		FW_ASSERT(length > 0 && length < MAX_PATH, "Failed to get the executable path");

		// Strip the filename and use forward slashes
		int lastSeparator = -1;
		for (DWORD i = 0; i < length; ++i)
		{
			if (buffer[i] == '\\')
				buffer[i] = '/';

			if (buffer[i] == '/')
				lastSeparator = static_cast<int>(i);
		}

		FW_ASSERT(lastSeparator != -1, "Executable path has no directory");
		buffer[lastSeparator + 1] = '\0';

		anOut = buffer;
	}

	bool FileExists(const FW_String& anAbsoluteFilePath)
	{
		const DWORD attributes = GetFileAttributesA(anAbsoluteFilePath.GetBuffer());
		return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
	}

	bool DirectoryExists(const FW_String& anAbsoluteFolderPath)
	{
		const DWORD attributes = GetFileAttributesA(anAbsoluteFolderPath.GetBuffer());
		return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
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
				FillFileInfo(name, fullPath, data, someOutFilePaths.Add());
			}
		} while (FindNextFile(filehandle, &data) != 0);

		if (GetLastError() != ERROR_NO_MORE_FILES)
			FW_ASSERT_ALWAYS("Something went wrong...");

		FindClose(filehandle);
		return true;
	}

	void CreateFolderIfNecessary(const FW_String& anAbsolutePath)
	{
		FW_GrowingArray<FW_String> words;
		SplitLine(anAbsolutePath, "/", words);

		// If the last word is a filename, then remove it so we dont make a folder out of it
		FW_String extention;
		GetFileExtention(words.GetLast(), extention);
		if (!extention.Empty())
			words.RemoveLast();

		// words[0] is the drive ("C:"), it always exists
		FW_String pathToCreate = words[0];
		for (int i = 1; i < words.Count(); ++i)
		{
			if (words[i].Empty())
				continue;

			pathToCreate += "/";
			pathToCreate += words[i];

			bool success = CreateDirectoryA(pathToCreate.GetBuffer(), NULL);
			if (!success && GetLastError() != ERROR_ALREADY_EXISTS)
				FW_ASSERT_ALWAYS;
		}
	}

	// Creates exactly the single directory passed in (its parent must already exist).
	void CreateFolder(const FW_String& anAbsoluteFolderPath)
	{
		bool success = CreateDirectoryA(anAbsoluteFolderPath.GetBuffer(), NULL);
		if (!success && GetLastError() != ERROR_ALREADY_EXISTS)
			FW_ASSERT_ALWAYS;
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
		if (findIndex == -1)
			return;

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

		FillFileInfo(data.cFileName, aFilePath, data, aFileInfoOut);

		FindClose(filehandle);
		return true;
	}

	bool RenameFile(const FW_String& aSourcePath, const FW_String& aDestinationPath)
	{
		return MoveFileExA(aSourcePath.GetBuffer(), aDestinationPath.GetBuffer(), MOVEFILE_REPLACE_EXISTING) != 0;
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

	void TrimBeginAndEnd(FW_String& aLine)
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

	FW_String TakeFirstWord(FW_String& aLine)
	{
		int begin = 0;
		int end = aLine.Find(" ", begin);
		if (end == -1)
			return aLine;

		FW_String word = aLine.SubStr(begin, end - 1);
		aLine = aLine.SubStr(end + 1, aLine.Length());
		return word;
	}

	void SplitLineOnSpace(const FW_String& aLine, FW_GrowingArray<FW_String>& outWords)
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
			outWords.Add(aLine.SubStr(begin, end - 1));

			begin = end + 1;
			end = aLine.Find(" ", begin);
		}

		outWords.Add(aLine.SubStr(begin, aLine.Length()));
	}

	void SplitLine(const FW_String& aLine, const char* aSeperator, FW_GrowingArray<FW_String>& outWords)
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

	float GetFloat(const FW_String& aWord)
	{
		return static_cast<float>(atof(aWord.GetBuffer()));
	}

	int GetInt(const FW_String& aWord)
	{
		return static_cast<int>(atoll(aWord.GetBuffer()));
	}
}