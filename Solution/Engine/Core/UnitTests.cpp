#include "stdafx.h"

#include "Core/UnitTests.h"
#include "Core/AssetRepository.h"
#include "Core/CommandLineArgs.h"
#include "Core/EngineTestSuite.h"

#include <FW_UnitTestSuite.h>

namespace Slush
{
	namespace UnitTests
	{
		Outcome Run(void (*aGameTestsCallback)())
		{
			FW_String reportFilePath;
			AssetRepository::GetDebugFilePath("unittest_results.txt", reportFilePath);

			FW_String scratchFolder;
			AssetRepository::GetDebugFilePath("", scratchFolder);

			FW_UnitTestSuite::BeginRun(reportFilePath.GetBuffer(), scratchFolder.GetBuffer());

			FW_UnitTestSuite::RunFrameworkTests();
			EngineTestSuite::RunTests();

			if (aGameTestsCallback)
				aGameTestsCallback();

			const bool passed = FW_UnitTestSuite::EndRun();

			if (CommandLineArgs::GetInstance().HasFlag("-runtestsonly"))
				return passed ? Outcome::ExitPassed : Outcome::ExitFailed;

			return Outcome::Continue;
		}
	}
}
