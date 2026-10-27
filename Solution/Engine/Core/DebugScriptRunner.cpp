#include "stdafx.h"
#include "Core/DebugScriptRunner.h"
#include "Core/Profiler.h"
#include <float.h>

namespace Slush
{
	namespace DebugScriptRunner_Private
	{
		void PerfSpike(DebugScriptRunner& aRunner, const DebugScriptRunner::Arguments& anArguments)
		{
			if (!aRunner.RequireArgumentCount(anArguments, 1))
				return;
			const char* text = anArguments[0].GetBuffer();
			char* end = nullptr;
			const double ms = strtod(text, &end);
			if (end == text || *end != '\0' || !_finite(ms) || ms < 0)
			{
				aRunner.Fail("perfspike expects non-negative finite milliseconds");
				return;
			}
			SLUSH_PROFILE_SCOPE("DebugPerfSpike");
			const Time::TimeUnit start = Time::GetCurrentExactTime();
			while ((Time::GetCurrentExactTime() - start) / 1000.0 < ms)
			{
				// Deliberate wall-clock stall, used to verify spike detection.
			}
		}

		bool ParseDuration(const char* aText, float& aValue, bool& anInFrames)
		{
			char* end = nullptr;
			double value = strtod(aText, &end);
			bool hasDigit = *aText >= '0' && *aText <= '9';
			if (!hasDigit && *aText == '.')
				hasDigit = aText[1] >= '0' && aText[1] <= '9';
			if (!hasDigit || end == aText || (*end != 's' && *end != 'f') || end[1] != '\0' || !_finite(value) || value < 0 || value > FLT_MAX)
				return false;
			aValue = static_cast<float>(value);
			anInFrames = *end == 'f';
			return true;
		}

		void Wait(DebugScriptRunner& aRunner, const DebugScriptRunner::Arguments& anArguments)
		{
			if (!aRunner.RequireArgumentCount(anArguments, 1))
				return;
			float duration = 0.f;
			bool inFrames = false;
			if (!ParseDuration(anArguments[0].GetBuffer(), duration, inFrames))
			{
				aRunner.Fail("wait expects a non-negative duration with s or f suffix");
				return;
			}
			aRunner.Wait(duration, inFrames);
		}

		void WaitFor(DebugScriptRunner& aRunner, const DebugScriptRunner::Arguments& anArguments)
		{
			if (anArguments.Count() < 1 || anArguments.Count() > 2 || anArguments[0].Empty())
			{
				aRunner.Fail("waitfor expects a trigger and optional timeout in seconds");
				return;
			}
			float timeout = 10.f;
			bool frames = false;
			if (anArguments.Count() == 2 && (!ParseDuration(anArguments[1].GetBuffer(), timeout, frames) || frames))
			{
				aRunner.Fail("waitfor timeout must be a non-negative duration in seconds");
				return;
			}
			aRunner.WaitFor(anArguments[0].GetBuffer(), timeout);
		}
	}

	DebugScriptRunner::DebugScriptRunner()
	{
		RegisterCommand("wait", &DebugScriptRunner_Private::Wait);
		RegisterCommand("waitfor", &DebugScriptRunner_Private::WaitFor);
		RegisterCommand("perfspike", &DebugScriptRunner_Private::PerfSpike);
	}

	void DebugScriptRunner::SetReporter(Reporter aReporter, void* aContext)
	{
		myReporting.myReporter = aReporter;
		myReporting.myContext = aContext;
	}

	void DebugScriptRunner::RegisterCommand(const char* aName, CommandCallback aCallback)
	{
		for (Command& command : myCommands)
		{
			if (command.myName == aName)
			{
				command.myCallback = aCallback;
				return;
			}
		}
		Command command;
		command.myName = aName;
		command.myCallback = aCallback;
		myCommands.Add(command);
	}

	void DebugScriptRunner::QueueLine(const char* aText)
	{
		myQueue.myLines.Add(FW_String(aText));
	}

	bool DebugScriptRunner::QueueFile(const char* aPath, bool aTruncate, bool anIsOptional)
	{
		FILE* file = nullptr;
		fopen_s(&file, aPath, "r");
		if (!file)
		{
			if (!anIsOptional)
				Fail((FW_String("cannot open debug script '") + aPath + "'").GetBuffer());
			return false;
		}
		FW_String line;
		bool hadContent = false;
		int character;
		while ((character = fgetc(file)) != EOF)
		{
			hadContent = true;
			if (character == '\n')
			{
				QueueLine(line.GetBuffer());
				line.Clear();
			}
			else
			{
				if (line.Length() + 2 >= line.MaxSize())
					line.Resize(line.MaxSize() * 2);
				line += static_cast<char>(character);
			}
		}
		if (!line.Empty())
			QueueLine(line.GetBuffer());
		bool readFailed = ferror(file) != 0;
		fclose(file);
		if (readFailed)
			Fail("could not read debug script");
		if (aTruncate && hadContent)
		{
			fopen_s(&file, aPath, "w");
			if (file)
				fclose(file);
			else
				Fail("could not truncate debug_input.txt");
		}
		return !readFailed;
	}

	void DebugScriptRunner::Step(float aDelta)
	{
		if (!myTrigger.myWaitingName.Empty() && !SatisfyTrigger())
		{
			myTrigger.myTimeout -= aDelta;
			if (myTrigger.myTimeout > 0.f)
				return;
			Fail((FW_String("timed out waiting for trigger '") + myTrigger.myWaitingName + "'").GetBuffer());
			myTrigger.myWaitingName.Clear();
		}
		if (myWait.myIsWaiting)
		{
			myWait.myRemaining -= myWait.myInFrames ? 1.f : aDelta;
			if (myWait.myRemaining > 0.f)
				return;
			myWait.myIsWaiting = false;
		}
		while (myQueue.myNextLine < myQueue.myLines.Count())
		{
			Arguments tokens;
			FW_String line = myQueue.myLines[myQueue.myNextLine++];
			if (!Tokenize(line.GetBuffer(), tokens) || tokens.IsEmpty())
				continue;
			FW_String name = tokens[0];
			tokens.RemoveNonCyclicAtIndex(0);
			bool found = false;
			for (const Command& command : myCommands)
			{
				if (command.myName == name.GetBuffer())
				{
					command.myCallback(*this, tokens);
					found = true;
					break;
				}
			}
			if (!found)
				Fail((FW_String("unknown debug command '") + name + "'").GetBuffer());
			if (myWait.myIsWaiting || !myTrigger.myWaitingName.Empty())
				break;
		}
		if (myQueue.myNextLine == myQueue.myLines.Count())
		{
			myQueue.myLines.RemoveAll();
			myQueue.myNextLine = 0;
		}
	}

	void DebugScriptRunner::Wait(float aDuration, bool anInFrames)
	{
		myWait.myRemaining = aDuration;
		myWait.myInFrames = anInFrames;
		myWait.myIsWaiting = true;
	}

	void DebugScriptRunner::WaitFor(const char* aTrigger, float aTimeout)
	{
		myTrigger.myWaitingName = aTrigger;
		myTrigger.myTimeout = aTimeout;
		SatisfyTrigger();
	}

	void DebugScriptRunner::PostTrigger(const char* aName)
	{
		myTrigger.myPostedNames.AddUnique(FW_String(aName));
		if (myReporting.myReporter)
			myReporting.myReporter(false, (FW_String("[Trigger] ") + aName).GetBuffer(), myReporting.myContext);
	}

	void DebugScriptRunner::RequestClick(const char* aLabel)
	{
		if (myClick.myRequestedThisFrame)
		{
			Fail("only one click is allowed per frame");
			return;
		}
		if (!aLabel || !*aLabel)
		{
			Fail("click expects a non-empty button label");
			return;
		}
		myClick.myPendingLabel = aLabel;
		myClick.myRequestedThisFrame = true;
	}

	bool DebugScriptRunner::ConsumeClick(const char* aLabel, bool anIsEnabled)
	{
		if (myClick.myPendingLabel.Empty() || myClick.myPendingLabel != aLabel)
			return false;
		if (!anIsEnabled)
			Fail((FW_String("button '") + aLabel + "' is disabled").GetBuffer());
		myClick.myPendingLabel.Clear();
		return anIsEnabled;
	}

	void DebugScriptRunner::EndFrame()
	{
		if (!myClick.myPendingLabel.Empty())
			Fail((FW_String("no clickable button '") + myClick.myPendingLabel + "' this frame").GetBuffer());
		myClick.myPendingLabel.Clear();
		myClick.myRequestedThisFrame = false;
	}

	void DebugScriptRunner::Fail(const char* aMessage)
	{
		myReporting.myHasFailed = true;
		if (myReporting.myReporter)
			myReporting.myReporter(true, aMessage, myReporting.myContext);
	}

	bool DebugScriptRunner::RequireArgumentCount(const Arguments& anArguments, int aCount)
	{
		if (anArguments.Count() == aCount)
			return true;
		Fail("wrong debug command argument count");
		return false;
	}

	bool DebugScriptRunner::SatisfyTrigger()
	{
		for (const FW_String& trigger : myTrigger.myPostedNames)
		{
			if (trigger == myTrigger.myWaitingName.GetBuffer())
			{
				myTrigger.myPostedNames.RemoveAll();
				myTrigger.myWaitingName.Clear();
				return true;
			}
		}
		return false;
	}

	bool DebugScriptRunner::Tokenize(const char* aText, Arguments& anArguments)
	{
		const char* cursor = aText;
		while (*cursor == ' ' || *cursor == '\t' || *cursor == '\r')
			++cursor;
		if (*cursor == '#')
			return true;
		while (*cursor)
		{
			while (*cursor == ' ' || *cursor == '\t' || *cursor == '\r' || *cursor == '\n')
				++cursor;
			if (!*cursor)
				break;
			bool quoted = *cursor == '"';
			if (quoted)
				++cursor;
			const char* begin = cursor;
			while (*cursor && (quoted ? *cursor != '"' : *cursor != ' ' && *cursor != '\t' && *cursor != '\r' && *cursor != '\n'))
				++cursor;
			if (quoted && *cursor != '"')
			{
				Fail("unterminated quote in debug command");
				return false;
			}
			FW_String token;
			token.Resize(static_cast<int>(cursor - begin) + 1);
			while (begin < cursor)
				token += *begin++;
			anArguments.Add(token);
			if (quoted)
			{
				++cursor;
				if (*cursor && *cursor != ' ' && *cursor != '\t' && *cursor != '\r' && *cursor != '\n')
				{
					Fail("expected whitespace after quoted debug argument");
					return false;
				}
			}
		}
		return true;
	}
}
