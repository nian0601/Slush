#include "stdafx.h"

#include "Core/AssetRepository.h"

#include <FW_Assert.h>
#include <FW_FileSystem.h>

namespace Slush
{
	namespace AssetRepository
	{
		static const char* ourEngineRepositoryName = "Engine";

		static FW_String ourAssetsRoot; // Absolute, trailing '/'
		static FW_GrowingArray<FW_String> ourRepositoryNames;
		static bool ourIsFrozen = false;

		static FW_String ourDebugFolder; // Absolute, trailing '/'

		static bool ourHasTestingState = false;
		static FW_String ourSavedAssetsRoot;
		static FW_GrowingArray<FW_String> ourSavedRepositoryNames;
		static bool ourSavedIsFrozen = false;

		static void FindAssetsRoot()
		{
			FW_String directory;
			FW_FileSystem::GetExecutableDirectory(directory);

			// Walk up from the exe's folder to the first ancestor that has an Assets/ folder
			while (!directory.Empty())
			{
				FW_String candidate = directory;
				candidate += "Assets";
				if (FW_FileSystem::DirectoryExists(candidate))
				{
					ourAssetsRoot = candidate;
					ourAssetsRoot += "/";
					return;
				}

				// Drop the trailing '/' and the last folder
				const FW_String withoutSlash = directory.SubStr(0, directory.Length() - 1);
				const int lastSeparator = withoutSlash.RFind("/");
				if (lastSeparator == FW_String::NotFound)
					break;

				directory = withoutSlash.SubStr(0, lastSeparator);
			}

			FW_ASSERT_ALWAYS("Failed to find an Assets/ folder in any folder above the executable");
		}

		static void Freeze()
		{
			if (ourIsFrozen)
				return;

			if (ourAssetsRoot.Empty())
				FindAssetsRoot();

			ourRepositoryNames.Add(ourEngineRepositoryName);
			ourIsFrozen = true;
		}

		static void BuildRepositoryPath(int aRepositoryIndex, const FW_String& aRelativePath, FW_String& anAbsolutePathOut)
		{
			anAbsolutePathOut = ourAssetsRoot;
			anAbsolutePathOut += ourRepositoryNames[aRepositoryIndex];
			anAbsolutePathOut += "/";
			anAbsolutePathOut += aRelativePath;
		}

		void Mount(const char* aName)
		{
			FW_ASSERT(!ourIsFrozen, "Mount('%s') called after the Asset Repository list was frozen by the first lookup", aName);
			FW_ASSERT(strcmp(aName, ourEngineRepositoryName) != 0, "The Engine Asset Repository is always mounted last automatically");

			ourRepositoryNames.Add(aName);
		}

		int GetRepositoryCount()
		{
			Freeze();
			return ourRepositoryNames.Count();
		}

		const FW_String& GetRepositoryName(int aRepositoryIndex)
		{
			Freeze();
			FW_ASSERT(aRepositoryIndex >= 0 && aRepositoryIndex < ourRepositoryNames.Count(), "Invalid Asset Repository index %i", aRepositoryIndex);
			return ourRepositoryNames[aRepositoryIndex];
		}

		bool FindAssetFile(const FW_String& aRelativePath, FW_String& anAbsolutePathOut, int* aRepositoryIndexOut)
		{
			Freeze();

			FW_String candidate;
			for (int i = 0; i < ourRepositoryNames.Count(); ++i)
			{
				BuildRepositoryPath(i, aRelativePath, candidate);
				if (FW_FileSystem::FileExists(candidate))
				{
					anAbsolutePathOut = candidate;
					if (aRepositoryIndexOut)
						*aRepositoryIndexOut = i;

					return true;
				}
			}

			return false;
		}

		void GetAssetWritePath(const FW_String& aRelativePath, int aRepositoryIndex, FW_String& anAbsolutePathOut)
		{
			Freeze();
			FW_ASSERT(aRepositoryIndex >= 0 && aRepositoryIndex < ourRepositoryNames.Count(), "Invalid Asset Repository index %i", aRepositoryIndex);
			BuildRepositoryPath(aRepositoryIndex, aRelativePath, anAbsolutePathOut);
		}

		void GetAllAssetFiles(const char* aRelativeDirectory, FW_GrowingArray<AssetFileInfo>& someOutFiles, FW_GrowingArray<AssetFileInfo>* someOutOverriddenFiles)
		{
			Freeze();

			FW_GrowingArray<FW_FileSystem::FileInfo> repositoryFiles;
			FW_String repositoryRoot;
			FW_String directory;
			for (int i = 0; i < ourRepositoryNames.Count(); ++i)
			{
				BuildRepositoryPath(i, "", repositoryRoot);
				directory = repositoryRoot;
				directory += aRelativeDirectory;

				repositoryFiles.RemoveAll();
				FW_FileSystem::GetAllFilesFromAbsoluteDirectory(directory.GetBuffer(), repositoryFiles);

				const int filesFromHigherPriority = someOutFiles.Count();
				for (const FW_FileSystem::FileInfo& file : repositoryFiles)
				{
					AssetFileInfo info;
					info.myFileName = file.myFileName;
					info.myFileNameNoExtention = file.myFileNameNoExtention;
					info.myRelativeFilePath = file.myAbsoluteFilePath.SubStr(repositoryRoot.Length() + 1, file.myAbsoluteFilePath.Length());
					info.myAbsoluteFilePath = file.myAbsoluteFilePath;
					info.myRepositoryIndex = i;

					bool isOverridden = false;
					for (int j = 0; j < filesFromHigherPriority; ++j)
					{
						if (someOutFiles[j].myFileNameNoExtention == info.myFileNameNoExtention)
						{
							isOverridden = true;
							break;
						}
					}

					if (!isOverridden)
						someOutFiles.Add(info);
					else if (someOutOverriddenFiles)
						someOutOverriddenFiles->Add(info);
				}
			}
		}

		void GetDebugFilePath(const FW_String& aRelativePath, FW_String& anAbsolutePathOut)
		{
			if (ourDebugFolder.Empty())
			{
				FW_FileSystem::GetExecutableDirectory(ourDebugFolder);
				ourDebugFolder += "Debug";
				FW_FileSystem::CreateFolder(ourDebugFolder);
				ourDebugFolder += "/";
			}

			anAbsolutePathOut = ourDebugFolder;
			anAbsolutePathOut += aRelativePath;
		}

		namespace Testing
		{
			void SetRootAndMounts(const char* anAssetsRoot, const char* const* someNames, int aCount)
			{
				FW_ASSERT(!ourHasTestingState, "AssetRepository::Testing::SetRootAndMounts called twice without Restore()");

				ourSavedAssetsRoot = ourAssetsRoot;
				ourSavedRepositoryNames = ourRepositoryNames;
				ourSavedIsFrozen = ourIsFrozen;
				ourHasTestingState = true;

				ourAssetsRoot = anAssetsRoot;
				if (ourAssetsRoot.Empty() || ourAssetsRoot[ourAssetsRoot.Length()] != '/')
					ourAssetsRoot += "/";

				ourRepositoryNames.RemoveAll();
				ourIsFrozen = false;
				for (int i = 0; i < aCount; ++i)
					Mount(someNames[i]);
			}

			void Restore()
			{
				FW_ASSERT(ourHasTestingState, "AssetRepository::Testing::Restore called without SetRootAndMounts()");

				ourAssetsRoot = ourSavedAssetsRoot;
				ourRepositoryNames = ourSavedRepositoryNames;
				ourIsFrozen = ourSavedIsFrozen;
				ourHasTestingState = false;
			}
		}
	}
}
