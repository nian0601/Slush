#include "stdafx.h"

#include "Core/EngineTestSuite.h"
#include "Core/CommandLineArgs.h"

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

		void RunTests()
		{
			FW_UnitTestSuite::BeginSuite("Engine");

			FW_RUN_TEST(TestCommandLineArgsFlagPresent);
			FW_RUN_TEST(TestCommandLineArgsFlagAbsent);
			FW_RUN_TEST(TestCommandLineArgsIgnoresExecutableName);
			FW_RUN_TEST(TestCommandLineArgsEmpty);
		}
	}
}
