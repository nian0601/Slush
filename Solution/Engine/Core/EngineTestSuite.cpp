#include "stdafx.h"

#include "Core/EngineTestSuite.h"
#include "Core/CommandLineArgs.h"
#include "Core/AssetRepository.h"

#include <FW_FileSystem.h>
#include <FW_UnitTestSuite.h>

namespace Slush
{
	namespace EngineTestSuite
	{
		// Always use a local CommandLineArgs, never Parse() on GetInstance() - that holds the real arguments
		void TestCommandLineArgsFlagPresent()
		{
			char* argv[] = { const_cast<char*>("game.exe"), const_cast<char*>("-first"), const_cast<char*>("-second") };
			CommandLineArgs args;
			args.Parse(3, argv);

			FW_TEST_CHECK(args.HasFlag("-first"), "Expected the first flag to be found");
			FW_TEST_CHECK(args.HasFlag("-second"), "Expected the second flag to be found");
		}

		void TestCommandLineArgsFlagAbsent()
		{
			char* argv[] = { const_cast<char*>("game.exe"), const_cast<char*>("-first") };
			CommandLineArgs args;
			args.Parse(2, argv);

			FW_TEST_CHECK(!args.HasFlag("-missing"), "Expected a flag that wasn't passed to be absent");
			FW_TEST_CHECK(!args.HasFlag("-firs"), "Expected a prefix of a flag not to match");
		}

		void TestCommandLineArgsIgnoresExecutableName()
		{
			char* argv[] = { const_cast<char*>("-fakeflag"), const_cast<char*>("-real") };
			CommandLineArgs args;
			args.Parse(2, argv);

			FW_TEST_CHECK(!args.HasFlag("-fakeflag"), "Expected argv[0] not to be treated as a flag");
			FW_TEST_CHECK(args.HasFlag("-real"), "Expected arguments after argv[0] to be parsed");
		}

		void TestCommandLineArgsEmpty()
		{
			char* argv[] = { const_cast<char*>("game.exe") };
			CommandLineArgs args;
			args.Parse(1, argv);
			FW_TEST_CHECK(!args.HasFlag("-anything"), "Expected no flags when only the executable name is passed");

			CommandLineArgs noArgs;
			noArgs.Parse(0, nullptr);
			FW_TEST_CHECK(!noArgs.HasFlag("-anything"), "Expected no flags for an empty argument list");
		}

		// Scratch tree: one "Game" Asset Repository mounted in front of "Engine"
		//   Game/Shared.txt, Game/Prefabs/A.prefab, Game/Prefabs/Common.prefab
		//   Engine/Shared.txt, Engine/EngineOnly.txt, Engine/Prefabs/Common.prefab, Engine/Prefabs/E.prefab
		static const char* ourAssetRepositoryTestFolder = "data/debug/assetrepository_test/";

		static void WriteScratchFile(const char* aRelativePath)
		{
			FW_String relativePath = ourAssetRepositoryTestFolder;
			relativePath += aRelativePath;
			FW_FileSystem::CreateFolderIfNecessary(relativePath);

			FW_String absolutePath;
			FW_FileSystem::GetAbsoluteFilePath(relativePath, absolutePath);

			FILE* file = nullptr;
			fopen_s(&file, absolutePath.GetBuffer(), "w");
			if (file)
			{
				fprintf(file, "%s\n", aRelativePath);
				fclose(file);
			}
		}

		// Points AssetRepository at the scratch tree for one test, and restores the real state when it leaves scope
		struct ScopedAssetRepositoryTree
		{
			ScopedAssetRepositoryTree()
			{
				WriteScratchFile("Game/Shared.txt");
				WriteScratchFile("Game/Prefabs/A.prefab");
				WriteScratchFile("Game/Prefabs/Common.prefab");
				WriteScratchFile("Engine/Shared.txt");
				WriteScratchFile("Engine/EngineOnly.txt");
				WriteScratchFile("Engine/Prefabs/Common.prefab");
				WriteScratchFile("Engine/Prefabs/E.prefab");

				FW_FileSystem::GetAbsoluteFilePath(ourAssetRepositoryTestFolder, myRoot);

				const char* mounts[] = { "Game" };
				AssetRepository::Testing::SetRootAndMounts(myRoot.GetBuffer(), mounts, 1);
			}

			~ScopedAssetRepositoryTree()
			{
				AssetRepository::Testing::Restore();
			}

			FW_String myRoot;
		};

		static int FindAssetFileInfo(const FW_GrowingArray<AssetRepository::AssetFileInfo>& someFiles, const char* aNameNoExtention)
		{
			for (int i = 0; i < someFiles.Count(); ++i)
			{
				if (someFiles[i].myFileNameNoExtention == aNameNoExtention)
					return i;
			}

			return -1;
		}

		void TestAssetRepositoryEngineMountedLast()
		{
			ScopedAssetRepositoryTree tree;

			FW_TEST_REQUIRE(AssetRepository::GetRepositoryCount() == 2, "Expected the game mount plus Engine");
			FW_TEST_CHECK(AssetRepository::GetRepositoryName(0) == "Game", "Expected the game Asset Repository first");
			FW_TEST_CHECK(AssetRepository::GetRepositoryName(1) == "Engine", "Expected Engine appended last");
		}

		void TestAssetRepositoryFindFirstHit()
		{
			ScopedAssetRepositoryTree tree;

			FW_String path;
			int repositoryIndex = -1;
			FW_TEST_CHECK(AssetRepository::FindAssetFile("Shared.txt", path, &repositoryIndex), "Expected a file in both Asset Repositories to be found");
			FW_TEST_CHECK(repositoryIndex == 0, "Expected the game file to override the Engine file at the same path");

			FW_String expected = tree.myRoot;
			expected += "Game/Shared.txt";
			FW_TEST_CHECK(path == expected, "Expected the absolute path into the game Asset Repository");

			repositoryIndex = -1;
			FW_TEST_CHECK(AssetRepository::FindAssetFile("EngineOnly.txt", path, &repositoryIndex), "Expected an Engine-only file to be found");
			FW_TEST_CHECK(repositoryIndex == 1, "Expected an Engine-only file to resolve to Engine");

			expected = tree.myRoot;
			expected += "Engine/EngineOnly.txt";
			FW_TEST_CHECK(path == expected, "Expected the absolute path into the Engine Asset Repository");
		}

		void TestAssetRepositoryFindMissing()
		{
			ScopedAssetRepositoryTree tree;

			FW_String path;
			FW_TEST_CHECK(!AssetRepository::FindAssetFile("Missing.txt", path), "Expected a missing file not to be found");
			FW_TEST_CHECK(!AssetRepository::FindAssetFile("Prefabs", path), "Expected a folder not to count as a file");
		}

		void TestAssetRepositoryGetAllAssetFiles()
		{
			ScopedAssetRepositoryTree tree;

			FW_GrowingArray<AssetRepository::AssetFileInfo> files;
			FW_GrowingArray<AssetRepository::AssetFileInfo> overridden;
			AssetRepository::GetAllAssetFiles("Prefabs", files, &overridden);

			FW_TEST_REQUIRE(files.Count() == 3, "Expected A and Common from the game plus E from Engine");
			FW_TEST_REQUIRE(overridden.Count() == 1, "Expected Engine's Common to be overridden");

			const int gameOnly = FindAssetFileInfo(files, "A");
			const int common = FindAssetFileInfo(files, "Common");
			const int engineOnly = FindAssetFileInfo(files, "E");
			FW_TEST_REQUIRE(gameOnly != -1 && common != -1 && engineOnly != -1, "Expected every asset name once");

			FW_TEST_CHECK(files[gameOnly].myRepositoryIndex == 0, "Expected A tagged with the game Asset Repository");
			FW_TEST_CHECK(files[common].myRepositoryIndex == 0, "Expected the game's Common to win");
			FW_TEST_CHECK(files[engineOnly].myRepositoryIndex == 1, "Expected E tagged with Engine");
			FW_TEST_CHECK(files[common].myRelativeFilePath == "Prefabs/Common.prefab", "Expected the path relative to the Asset Repository");

			FW_String expected = tree.myRoot;
			expected += "Engine/Prefabs/E.prefab";
			FW_TEST_CHECK(files[engineOnly].myAbsoluteFilePath == expected, "Expected the absolute path into Engine");

			FW_TEST_CHECK(overridden[0].myFileNameNoExtention == "Common", "Expected Common to be the overridden asset");
			FW_TEST_CHECK(overridden[0].myRepositoryIndex == 1, "Expected the overridden Common to come from Engine");
		}

		void TestAssetRepositoryWritePath()
		{
			ScopedAssetRepositoryTree tree;

			// Existing asset: written back to the Asset Repository it was found in
			FW_String foundPath;
			int repositoryIndex = -1;
			FW_TEST_REQUIRE(AssetRepository::FindAssetFile("EngineOnly.txt", foundPath, &repositoryIndex), "Expected the Engine-only file to be found");

			FW_String writePath;
			AssetRepository::GetAssetWritePath("EngineOnly.txt", repositoryIndex, writePath);
			FW_TEST_CHECK(writePath == foundPath, "Expected an existing asset to be written back to its source Asset Repository");

			// New asset: written to the first mount
			AssetRepository::GetAssetWritePath("New.txt", 0, writePath);
			FW_String expected = tree.myRoot;
			expected += "Game/New.txt";
			FW_TEST_CHECK(writePath == expected, "Expected a new asset to go to the first mounted Asset Repository");
		}

		void RunTests()
		{
			FW_UnitTestSuite::BeginSuite("Engine");

			FW_RUN_TEST(TestCommandLineArgsFlagPresent);
			FW_RUN_TEST(TestCommandLineArgsFlagAbsent);
			FW_RUN_TEST(TestCommandLineArgsIgnoresExecutableName);
			FW_RUN_TEST(TestCommandLineArgsEmpty);

			FW_RUN_TEST(TestAssetRepositoryEngineMountedLast);
			FW_RUN_TEST(TestAssetRepositoryFindFirstHit);
			FW_RUN_TEST(TestAssetRepositoryFindMissing);
			FW_RUN_TEST(TestAssetRepositoryGetAllAssetFiles);
			FW_RUN_TEST(TestAssetRepositoryWritePath);
		}
	}
}
