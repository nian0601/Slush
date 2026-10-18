#pragma once
#include "FW_GrowingArray.h"
#include "FW_String.h"

namespace Slush
{
	class DebugScriptRunner
	{
	public:
		using Arguments = FW_GrowingArray<FW_String>;
		using CommandCallback = void (*)(DebugScriptRunner& aRunner, const Arguments& anArguments);
		using Reporter = void (*)(bool anIsError, const char* aMessage, void* aContext);

		DebugScriptRunner();
		void SetReporter(Reporter aReporter, void* aContext = nullptr);
		void RegisterCommand(const char* aName, CommandCallback aCallback);
		void QueueLine(const char* aText);
		bool QueueFile(const char* aPath, bool aTruncate = false, bool anIsOptional = false);
		void Step(float aDelta);
		bool HasFailed() const { return myHasFailed; }
		void Fail(const char* aMessage);
		bool RequireArgumentCount(const Arguments& anArguments, int aCount);

	private:
		struct Command
		{
			FW_String myName;
			CommandCallback myCallback = nullptr;
		};
		static void Wait(DebugScriptRunner& aRunner, const Arguments& anArguments);
		bool Tokenize(const char* aText, Arguments& anArguments);

		FW_GrowingArray<Command> myCommands;
		FW_GrowingArray<FW_String> myLines;
		int myNextLine = 0;
		float myWaitRemaining = 0.f;
		bool myWaitInFrames = false;
		bool myIsWaiting = false;
		bool myHasFailed = false;
		Reporter myReporter = nullptr;
		void* myReporterContext = nullptr;
	};
}
