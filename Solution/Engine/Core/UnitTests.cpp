#include "stdafx.h"

#include "Core/UnitTests.h"
#include "Core/CommandLineArgs.h"

#include <FW_UnitTestSuite.h>

namespace Slush
{
	namespace UnitTests
	{
		Outcome Run(void (*aGameTestsCallback)())
		{
			FW_UnitTestSuite::BeginRun();

			FW_UnitTestSuite::RunFrameworkTests();

			if (aGameTestsCallback)
				aGameTestsCallback();

			const bool passed = FW_UnitTestSuite::EndRun();

			if (CommandLineArgs::GetInstance().HasFlag("-runtestsonly"))
				return passed ? Outcome::ExitPassed : Outcome::ExitFailed;

			return Outcome::Continue;
		}
	}
}
