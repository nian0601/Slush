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
		void Wait(float aDuration, bool anInFrames);
		void WaitFor(const char* aTrigger, float aTimeout);
		void PostTrigger(const char* aName);
		void RequestClick(const char* aLabel);
		bool ConsumeClick(const char* aLabel, bool anIsEnabled);
		bool HasPendingClick() const { return !myClick.myPendingLabel.Empty(); }
		void EndFrame();
		bool HasFailed() const { return myReporting.myHasFailed; }
		void Fail(const char* aMessage);
		bool RequireArgumentCount(const Arguments& anArguments, int aCount);

	private:
		struct Command
		{
			FW_String myName;
			CommandCallback myCallback = nullptr;
		};
		bool SatisfyTrigger();
		bool Tokenize(const char* aText, Arguments& anArguments);

		struct QueueState
		{
			FW_GrowingArray<FW_String> myLines;
			int myNextLine = 0;
		};

		struct WaitState
		{
			float myRemaining = 0.f;
			bool myInFrames = false;
			bool myIsWaiting = false;
		};

		struct TriggerState
		{
			FW_GrowingArray<FW_String> myPostedNames;
			FW_String myWaitingName;
			float myTimeout = 0.f;
		};

		struct ClickState
		{
			FW_String myPendingLabel;
			bool myRequestedThisFrame = false;
		};

		struct ReportingState
		{
			Reporter myReporter = nullptr;
			void* myContext = nullptr;
			bool myHasFailed = false;
		};

		FW_GrowingArray<Command> myCommands;
		QueueState myQueue;
		WaitState myWait;
		TriggerState myTrigger;
		ClickState myClick;
		ReportingState myReporting;
	};
}
