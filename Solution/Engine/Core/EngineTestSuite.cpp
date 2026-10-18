#include "stdafx.h"

#include "Core/EngineTestSuite.h"
#include "Core/DebugScriptRunner.h"
#include "Core/Input.h"
#include "Core/CommandLineArgs.h"
#include "Core/AssetRepository.h"
#include "UI/UITheme.h"
#include "UI/UIBuilder.h"

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
		static void WriteScratchFile(const FW_String& aRoot, const char* aRelativePath)
		{
			FW_String absolutePath = aRoot;
			absolutePath += aRelativePath;

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
				AssetRepository::GetDebugFilePath("assetrepository_test/", myRoot);

				const char* folders[] = { "", "Game", "Game/Prefabs", "Engine", "Engine/Prefabs" };
				for (const char* folder : folders)
				{
					FW_String folderPath = myRoot;
					folderPath += folder;
					FW_FileSystem::CreateFolder(folderPath);
				}

				WriteScratchFile(myRoot, "Game/Shared.txt");
				WriteScratchFile(myRoot, "Game/Prefabs/A.prefab");
				WriteScratchFile(myRoot, "Game/Prefabs/Common.prefab");
				WriteScratchFile(myRoot, "Engine/Shared.txt");
				WriteScratchFile(myRoot, "Engine/EngineOnly.txt");
				WriteScratchFile(myRoot, "Engine/Prefabs/Common.prefab");
				WriteScratchFile(myRoot, "Engine/Prefabs/E.prefab");

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
				if (someFiles[i].myFileInfo.myFileNameNoExtention == aNameNoExtention)
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
			FW_TEST_CHECK(files[engineOnly].myFileInfo.myAbsoluteFilePath == expected, "Expected the absolute path into Engine");
			FW_TEST_CHECK(files[engineOnly].myFileInfo.myLastTimeModifiedHighbit != 0, "Expected the timestamp from FW_FileSystem's FileInfo");

			FW_TEST_CHECK(overridden[0].myFileInfo.myFileNameNoExtention == "Common", "Expected Common to be the overridden asset");
			FW_TEST_CHECK(overridden[0].myRepositoryIndex == 1, "Expected the overridden Common to come from Engine");
		}

		void TestAssetRepositoryWritePath()
		{
			ScopedAssetRepositoryTree tree;

			// Existing asset: written back to the Asset Repository it was found in
			FW_String foundPath;
			int repositoryIndex = -1;
			FW_TEST_REQUIRE(AssetRepository::FindAssetFile("EngineOnly.txt", foundPath, &repositoryIndex), "Expected the Engine-only file to be found");
			FW_TEST_CHECK(repositoryIndex == 1, "Expected the Engine-only file to come from Engine");

			FW_String writePath;
			AssetRepository::GetAssetWritePath("EngineOnly.txt", repositoryIndex, writePath);
			FW_String expected = tree.myRoot;
			expected += "Engine/EngineOnly.txt";
			FW_TEST_CHECK(writePath == expected, "Expected an existing asset to be written back to its source Asset Repository");

			// New asset: written to the first mount
			AssetRepository::GetAssetWritePath("New.txt", 0, writePath);
			expected = tree.myRoot;
			expected += "Game/New.txt";
			FW_TEST_CHECK(writePath == expected, "Expected a new asset to go to the first mounted Asset Repository");
		}

		void TestUIThemeButtonStates()
		{
			FW_TEST_CHECK(UIThemeUtils::ResolveState(UIState::Normal, false) == UIState::Normal, "Normal stays normal away from the mouse");
			FW_TEST_CHECK(UIThemeUtils::ResolveState(UIState::Normal, true) == UIState::Hovered, "Normal highlights under the mouse");
			FW_TEST_CHECK(UIThemeUtils::ResolveState(UIState::Hovered, false) == UIState::Hovered, "Explicit hovered state stays hovered");
			FW_TEST_CHECK(UIThemeUtils::ResolveState(UIState::Hovered, true) == UIState::Hovered, "Explicit hovered state stays hovered under the mouse");
			FW_TEST_CHECK(UIThemeUtils::ResolveState(UIState::Selected, false) == UIState::Selected, "Selected stays selected away from the mouse");
			FW_TEST_CHECK(UIThemeUtils::ResolveState(UIState::Selected, true) == UIState::Selected, "Selected keeps its look under the mouse");
			FW_TEST_CHECK(UIThemeUtils::ResolveState(UIState::Disabled, false) == UIState::Disabled, "Disabled stays disabled away from the mouse");
			FW_TEST_CHECK(UIThemeUtils::ResolveState(UIState::Disabled, true) == UIState::Disabled, "Disabled never highlights under the mouse");
			FW_TEST_CHECK(UIThemeUtils::IsClickable(UIState::Normal), "Normal can be clicked");
			FW_TEST_CHECK(UIThemeUtils::IsClickable(UIState::Hovered), "Hovered can be clicked");
			FW_TEST_CHECK(UIThemeUtils::IsClickable(UIState::Selected), "Selected can be clicked");
			FW_TEST_CHECK(!UIThemeUtils::IsClickable(UIState::Disabled), "Disabled cannot be clicked");
		}

		void TestUIBuilderLayout()
		{
			UITheme theme;
			UIBuilder builder;
			builder.Start(theme, { 800, 600 });
			builder.OpenElement().SetColor(0xFFFFFFFF).SetXSizing(UIElementStyle::FIXED, 100).SetYSizing(UIElementStyle::FIXED, 50);
			builder.CloseElement();
			builder.Finish(UIInputState());
			FW_GrowingArray<UIBuilder::RenderCommand> commands;
			builder.GenerateRenderCommands(commands);
			FW_TEST_REQUIRE(commands.Count() == 1, "One element produces one command");
			FW_TEST_CHECK(commands[0].mySize == Vector2f(100, 50), "Fixed size is preserved");
			FW_TEST_CHECK(commands[0].myPosition == Vector2f(350, 275), "Root centres the element");
		}

		void TestUIBuilderClicks()
		{
			UITheme theme;
			UIBuilder builder;
			for (int test = 0; test < 4; ++test)
			{
				builder.Start(theme, { 800, 600 });
				builder.BeginButton("A", test == 3 ? UIState::Disabled : UIState::Normal)
					.SetXSizing(UIElementStyle::FIXED, 100).SetYSizing(UIElementStyle::FIXED, 50);
				builder.EndButton();
				UIInputState input;
				input.myMousePosition = test == 1 ? Vector2i(0, 0) : Vector2i(400, 300);
				input.myWasLeftMouseReleased = test != 2;
				builder.Finish(input);
				FW_TEST_CHECK(builder.WasClicked("A") == (test == 0), "Only an enabled button released inside is clicked");
			}
		}

		void TestUIBuilderMouseOver()
		{
			UITheme theme;
			UIBuilder builder;
			UIInputState input;
			input.myMousePosition = { 400, 300 };
			for (int test = 0; test < 5; ++test)
			{
				builder.Start(theme, { 800, 600 });
				FW_TEST_CHECK(!builder.IsMouseOverUI(), "Start clears the mouse-over result");
				UIElementStyle& style = test == 2 ? builder.BeginPanel() : builder.OpenElement();
				style.SetXSizing(UIElementStyle::FIXED, 100).SetYSizing(UIElementStyle::FIXED, 50);
				if (test < 2)
					style.SetColor(0xFFFFFFFF);
				if (test == 3)
				{
					style.SetXSizing(UIElementStyle::GROW).SetYSizing(UIElementStyle::GROW);
					builder.VerticalSpacing(20);
				}
				if (test == 4)
					style.SetOutlineColor(0xFFFFFFFF).SetOutlineThickness(-1.f);
				builder.CloseElement();
				input.myMousePosition = test == 1 ? Vector2i(0, 0) : Vector2i(400, 300);
				builder.Finish(input);
				FW_TEST_CHECK(builder.IsMouseOverUI() == (test != 1 && test != 3), "Visible content blocks; outside and invisible layout do not");
			}
			builder.Start(theme, { 800, 600 });
			builder.ScreenFade();
			input.myMousePosition = { 1, 1 };
			builder.Finish(input);
			FW_TEST_CHECK(builder.IsMouseOverUI(), "Screen fade blocks the entire screen");
		}

		void TestUIBuilderAnchors()
		{
			UITheme theme;
			UIBuilder builder;
			const UIElementStyle::AnchorPoint anchors[] = {
				UIElementStyle::AnchorPoint::TOP_LEFT, UIElementStyle::AnchorPoint::TOP_CENTER, UIElementStyle::AnchorPoint::TOP_RIGHT,
				UIElementStyle::AnchorPoint::CENTER_LEFT, UIElementStyle::AnchorPoint::CENTER, UIElementStyle::AnchorPoint::CENTER_RIGHT,
				UIElementStyle::AnchorPoint::BOTTOM_LEFT, UIElementStyle::AnchorPoint::BOTTOM_CENTER, UIElementStyle::AnchorPoint::BOTTOM_RIGHT
			};
			for (int i = 0; i < 11; ++i)
			{
				builder.Start(theme, { 800, 600 });
				UIElementStyle& style = builder.OpenElement();
				style.SetColor(0xFFFFFFFF).SetXSizing(i == 10 ? UIElementStyle::GROW : UIElementStyle::FIXED, 100)
					.SetYSizing(UIElementStyle::FIXED, i == 10 ? 100 : 50);
				style.SetAnchor(anchors[i < 9 ? i : 6], i == 9 ? Vector2i(10, -100) : Vector2i(0, 0));
				builder.CloseElement();
				builder.Finish(UIInputState());
				FW_GrowingArray<UIBuilder::RenderCommand> commands;
				builder.GenerateRenderCommands(commands);
				FW_TEST_REQUIRE(commands.Count() == 1, "One anchored element");
				Vector2f expectedPosition = i < 9 ? Vector2f(350.f * (i % 3), 275.f * (i / 3)) : (i == 9 ? Vector2f(10, 450) : Vector2f(0, 500));
				FW_TEST_CHECK(commands[0].myPosition == expectedPosition, "Anchor aligns matching corners with offset");
				FW_TEST_CHECK(commands[0].mySize == (i == 10 ? Vector2f(800, 100) : Vector2f(100, 50)), "Anchored GROW fills parent");
			}
		}

		void TestUIBuilderAnchoredFlowAndPadding()
		{
			UITheme theme;
			UIBuilder builder;
			const int paddings[] = { 0, 10 };
			for (int padding : paddings)
			{
				builder.Start(theme, { 800, 600 });
				builder.OpenElement().SetPadding(padding, padding).SetChildGap(20);
				builder.OpenElement().SetColor(0xFFFFFFFF).SetXSizing(UIElementStyle::FIXED, 100).SetYSizing(UIElementStyle::FIXED, 50);
				builder.CloseElement();
				builder.OpenElement().SetColor(0xFFFFFFFF).SetXSizing(UIElementStyle::FIXED, 300).SetYSizing(UIElementStyle::FIXED, 300)
					.SetAnchor(UIElementStyle::AnchorPoint::BOTTOM_LEFT);
				builder.CloseElement();
				builder.CloseElement();
				builder.Finish(UIInputState());
				FW_GrowingArray<UIBuilder::RenderCommand> commands;
				builder.GenerateRenderCommands(commands);
				FW_TEST_REQUIRE(commands.Count() == 3, "Parent and two children");
				FW_TEST_CHECK(commands[0].mySize == Vector2f(100.f + padding * 2, 50.f + padding * 2), "Anchored child adds no FIT size or gap");
				FW_TEST_CHECK(commands[2].myPosition == commands[0].myPosition + Vector2f(0, commands[0].mySize.y - 300), "Anchor uses parent outer rect, ignoring padding");
			}
		}

		void TestUIBuilderAnchoredDrawAndClickOrder()
		{
			UITheme theme;
			UIBuilder builder;
			builder.Start(theme, { 800, 600 });
			builder.BeginButton("Anchored", UIState::Selected).SetXSizing(UIElementStyle::FIXED, 100).SetYSizing(UIElementStyle::FIXED, 50)
				.SetAnchor(UIElementStyle::AnchorPoint::CENTER);
			builder.EndButton();
			builder.BeginButton("Flow").SetXSizing(UIElementStyle::FIXED, 100).SetYSizing(UIElementStyle::FIXED, 50);
			builder.EndButton();
			UIInputState input;
			input.myMousePosition = { 400, 300 };
			input.myWasLeftMouseReleased = true;
			builder.Finish(input);
			FW_GrowingArray<UIBuilder::RenderCommand> commands;
			builder.GenerateRenderCommands(commands);
			FW_TEST_REQUIRE(commands.Count() == 2, "Both overlapping buttons draw");
			FW_TEST_CHECK(builder.WasClicked("Anchored") && !builder.WasClicked("Flow"), "Top-most anchored button alone receives the click");
			FW_TEST_CHECK(builder.IsMouseOverUI(), "Anchored content participates in mouse-over");
			FW_TEST_CHECK(commands[0].myColor == theme.myButton.myVisuals[static_cast<int>(UIState::Hovered)].myColor, "Flow button hover is still updated after another button consumes click");
			FW_TEST_CHECK(commands[1].myColor == theme.myButton.myVisuals[static_cast<int>(UIState::Selected)].myColor, "Anchored button draws last despite being declared first");
		}

		void TestUIBuilderTooltipPlacement()
		{
			const Vector2i screenSize = { 800, 600 };
			FW_TEST_CHECK(UITooltipUtils::CalculatePosition(MakeRect(350, 275, 450, 325), { 200, 40 }, screenSize) == Vector2i(300, 231), "Tooltip is centred above with four pixel gap");
			FW_TEST_CHECK(UITooltipUtils::CalculatePosition(MakeRect(350, 0, 450, 50), { 200, 40 }, screenSize) == Vector2i(300, 54), "Tooltip at top falls below element");
			FW_TEST_CHECK(UITooltipUtils::CalculatePosition(MakeRect(0, 275, 100, 325), { 200, 40 }, screenSize) == Vector2i(0, 231), "Tooltip clamps to left edge");
			FW_TEST_CHECK(UITooltipUtils::CalculatePosition(MakeRect(700, 275, 800, 325), { 200, 40 }, screenSize) == Vector2i(600, 231), "Tooltip clamps to right edge");
			FW_TEST_CHECK(UITooltipUtils::CalculatePosition(MakeRect(16, 518, 246, 582), { 300, 40 }, screenSize) == Vector2i(0, 474), "Build bar tooltip fits above button");
			FW_TEST_CHECK(UITooltipUtils::CalculatePosition(MakeRect(350, 0, 450, 590), { 200, 40 }, screenSize) == Vector2i(300, 560), "Fallback clamps to bottom edge");
			FW_TEST_CHECK(UITooltipUtils::CalculatePosition(MakeRect(0, 0, 100, 50), { 900, 700 }, screenSize) == Vector2i(0, 0), "Oversized tooltip pins to screen origin");
		}

		int locScriptCalls = 0;
		FW_String locScriptArgument;
		int locScriptErrors = 0;

		void ScriptProbe(DebugScriptRunner& aRunner, const DebugScriptRunner::Arguments& anArguments)
		{
			if (!aRunner.RequireArgumentCount(anArguments, 1))
				return;
			++locScriptCalls;
			locScriptArgument = anArguments[0];
		}

		void TestDebugScriptParsingAndFailure()
		{
			locScriptCalls = 0;
			locScriptErrors = 0;
			DebugScriptRunner runner;
			runner.SetReporter([](bool anIsError, const char*, void*) { if (anIsError) ++locScriptErrors; });
			runner.RegisterCommand("probe", &ScriptProbe);
			runner.QueueLine("   # ignored");
			runner.QueueLine(" ");
			runner.QueueLine("probe \"Restart Level\"");
			runner.Step(0.f);
			FW_TEST_CHECK(locScriptCalls == 1 && locScriptArgument == "Restart Level", "Quotes preserve whitespace");
			FW_TEST_CHECK(!runner.HasFailed(), "Comments and blanks are ignored");
			runner.QueueLine("missing");
			runner.QueueLine("probe");
			runner.QueueLine("probe \"unterminated");
			runner.QueueLine("probe after");
			runner.Step(0.f);
			FW_TEST_CHECK(runner.HasFailed() && locScriptErrors == 3, "Unknown command, arity and quotes each fail");
			FW_TEST_CHECK(locScriptCalls == 2 && locScriptArgument == "after", "Failures do not abort later lines");
		}

		void TestDebugScriptWaits()
		{
			locScriptCalls = 0;
			DebugScriptRunner runner;
			runner.RegisterCommand("probe", &ScriptProbe);
			runner.QueueLine("probe first");
			runner.QueueLine("wait 0.5s");
			runner.QueueLine("probe second");
			runner.Step(100.f);
			FW_TEST_CHECK(locScriptCalls == 1, "Step runs until wait without charging preceding delta");
			runner.QueueLine("wait 2f");
			runner.QueueLine("probe third");
			runner.Step(0.25f);
			FW_TEST_CHECK(locScriptCalls == 1, "Seconds wait blocks");
			runner.Step(0.25f);
			FW_TEST_CHECK(locScriptCalls == 2, "Queued lines follow running wait");
			runner.Step(100.f);
			FW_TEST_CHECK(locScriptCalls == 2, "Frame waits ignore delta");
			runner.Step(0.f);
			FW_TEST_CHECK(locScriptCalls == 3 && !runner.HasFailed(), "Frame wait resumes after two frames");
			const char* valid[] = { "wait 2s", "wait 0.5s", "wait 120f" };
			for (const char* line : valid)
			{
				DebugScriptRunner validRunner;
				validRunner.QueueLine(line);
				validRunner.Step(0.f);
				FW_TEST_CHECK(!validRunner.HasFailed(), "Valid wait accepted");
			}
			const char* invalid[] = { "wait 2", "wait -1s", "wait abcs", "wait 2ss", "wait nans", "wait 1e999s", "wait", "wait 1s extra" };
			for (const char* line : invalid)
			{
				DebugScriptRunner invalidRunner;
				invalidRunner.QueueLine(line);
				invalidRunner.Step(0.f);
				FW_TEST_CHECK(invalidRunner.HasFailed(), "Invalid duration fails");
			}
		}

		void TestInputKeyNames()
		{
			for (int i = 0; i < Input::__COUNT; ++i)
			{
				Input::KeyCode key;
				const char* name = InputUtils::GetKeyName(static_cast<Input::KeyCode>(i));
				FW_TEST_CHECK(InputUtils::FindKeyCode(name, key) && key == i, "Unique round-trip for every key");
				FW_String lower = name;
				_strlwr_s(lower.GetRawBuffer(), lower.MaxSize());
				FW_TEST_CHECK(InputUtils::FindKeyCode(lower.GetBuffer(), key) && key == i, "Case-insensitive key lookup");
			}
			Input::KeyCode key;
			FW_TEST_CHECK(!InputUtils::FindKeyCode("nonsense", key), "Unknown key rejected");
		}

		void RunTests()
		{
			FW_UnitTestSuite::BeginSuite("Engine");
			FW_RUN_TEST(TestDebugScriptParsingAndFailure);
			FW_RUN_TEST(TestDebugScriptWaits);
			FW_RUN_TEST(TestInputKeyNames);

			FW_RUN_TEST(TestUIThemeButtonStates);
			FW_RUN_TEST(TestUIBuilderLayout);
			FW_RUN_TEST(TestUIBuilderClicks);
			FW_RUN_TEST(TestUIBuilderMouseOver);
			FW_RUN_TEST(TestUIBuilderAnchors);
			FW_RUN_TEST(TestUIBuilderAnchoredFlowAndPadding);
			FW_RUN_TEST(TestUIBuilderAnchoredDrawAndClickOrder);
			FW_RUN_TEST(TestUIBuilderTooltipPlacement);

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
