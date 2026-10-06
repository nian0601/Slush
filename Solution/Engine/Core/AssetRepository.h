#pragma once

#include <FW_FileSystem.h>
#include <FW_GrowingArray.h>
#include <FW_String.h>

namespace Slush
{
	// Every Slush file path resolves through here.
	// Assets half: authored content lives in Asset Repositories, named folders under the top-level Assets/ folder.
	// A game mounts an ordered list of them (game first, Engine always appended last), and lookups return the first hit.
	// Debug half: logs, screenshots, test reports and other runtime output live in <exeDir>/Debug/.
	namespace AssetRepository
	{
		// FW_FileSystem's FileInfo (names, absolute path, timestamps) plus where it sits among the Asset Repositories
		struct AssetFileInfo
		{
			FW_FileSystem::FileInfo myFileInfo;
			FW_String myRelativeFilePath; // Relative to the Asset Repository, e.g. "EntityPrefabs/Player.prefab"
			int myRepositoryIndex = -1;
		};

		// Appends an Asset Repository to the mount list, highest priority first.
		// Must be called before the first resolution, the list is frozen after that and Engine is appended last.
		void Mount(const char* aName);

		int GetRepositoryCount();
		const FW_String& GetRepositoryName(int aRepositoryIndex);

		// The first mounted Asset Repository that has <name>/<aRelativePath>. Returns false if none has it.
		bool FindAssetFile(const FW_String& aRelativePath, FW_String& anAbsolutePathOut, int* aRepositoryIndexOut = nullptr);

		// The absolute path of aRelativePath inside a specific Asset Repository, whether it exists or not. Index 0 is the first mount.
		void GetAssetWritePath(const FW_String& aRelativePath, int aRepositoryIndex, FW_String& anAbsolutePathOut);

		// Recursive scan of aRelativeDirectory in every mounted Asset Repository. Files with the same name (without extension)
		// in more than one Asset Repository resolve to the higher-priority one, the others go to someOutOverriddenFiles if given.
		void GetAllAssetFiles(const char* aRelativeDirectory, FW_GrowingArray<AssetFileInfo>& someOutFiles, FW_GrowingArray<AssetFileInfo>* someOutOverriddenFiles = nullptr);

		// <exeDir>/Debug/<aRelativePath>, creating Debug/ on first use
		void GetDebugFilePath(const FW_String& aRelativePath, FW_String& anAbsolutePathOut);

		namespace Testing
		{
			// Points the Assets half at a scratch tree (anAssetsRoot holds one folder per Asset Repository) with these mounts,
			// Engine is still appended last. Restore() puts the real state back.
			void SetRootAndMounts(const char* anAssetsRoot, const char* const* someNames, int aCount);
			void Restore();
		}
	}
}
