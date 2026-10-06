#pragma once

// Test runner: records every failed check and keeps going, then writes
// the report file given to BeginRun (see EndRun). Line 1 of that file is always
// "UNITTEST_RESULT: PASS (...)" or "UNITTEST_RESULT: FAIL (...)".
namespace FW_UnitTestSuite
{
	// Both absolute. aScratchFolder must exist and end with '/', Framework tests write their scratch files there.
	void BeginRun(const char* aReportFilePath, const char* aScratchFolder);
	void BeginSuite(const char* aSuiteName);
	void BeginTest(const char* aTestName);

	// Returns aPassed, so FW_TEST_REQUIRE can leave the test on failure
	bool RecordCheck(bool aPassed, const char* aFile, int aLine, const char* anExpression, const char* aMessage);

	// Writes the report, returns true if every check passed
	bool EndRun();

	// Runs just the Framework suite, without BeginRun/EndRun
	void RunFrameworkTests();
}

#define FW_RUN_TEST(aTestFunc) \
{\
	FW_UnitTestSuite::BeginTest(#aTestFunc);\
	aTestFunc();\
}

// Records the failure and continues
#define FW_TEST_CHECK(anExpression, aMessage) \
	FW_UnitTestSuite::RecordCheck((anExpression) ? true : false, __FILE__, __LINE__, #anExpression, aMessage)

// Records the failure and leaves the current test. Only for failures that make the rest of the test unsafe or meaningless.
#define FW_TEST_REQUIRE(anExpression, aMessage) \
{\
	if (!FW_UnitTestSuite::RecordCheck((anExpression) ? true : false, __FILE__, __LINE__, #anExpression, aMessage)) \
		return;\
}
