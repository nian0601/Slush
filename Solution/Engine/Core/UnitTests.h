#pragma once

namespace Slush
{
	namespace UnitTests
	{
		enum class Outcome
		{
			Continue,
			ExitPassed,
			ExitFailed,
		};

		// Runs every suite (Framework, then aGameTestsCallback if not null) and writes the report.
		// Never exits the process itself, main() acts on the returned Outcome.
		Outcome Run(void (*aGameTestsCallback)());
	}
}
