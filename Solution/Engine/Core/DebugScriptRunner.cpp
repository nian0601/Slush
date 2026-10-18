#include "stdafx.h"
#include "Core/DebugScriptRunner.h"
#include <float.h>

namespace Slush
{
	DebugScriptRunner::DebugScriptRunner()
	{
		RegisterCommand("wait", &Wait);
	}

	void DebugScriptRunner::SetReporter(Reporter aReporter, void* aContext)
	{
		myReporter = aReporter;
		myReporterContext = aContext;
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
		myLines.Add(FW_String(aText));
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

	void DebugScriptRunner::Fail(const char* aMessage)
	{
		myHasFailed = true;
		if (myReporter)
			myReporter(true, aMessage, myReporterContext);
	}

	bool DebugScriptRunner::RequireArgumentCount(const Arguments& anArguments, int aCount)
	{
		if (anArguments.Count() == aCount)
			return true;
		Fail("wrong debug command argument count");
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

	void DebugScriptRunner::Wait(DebugScriptRunner& aRunner, const Arguments& anArguments)
	{
		if (!aRunner.RequireArgumentCount(anArguments, 1))
			return;
		const char* text = anArguments[0].GetBuffer();
		char* end = nullptr;
		double value = strtod(text, &end);
		bool hasDigit = *text >= '0' && *text <= '9';
		if (!hasDigit && *text == '.')
			hasDigit = text[1] >= '0' && text[1] <= '9';
		if (!hasDigit || end == text || (*end != 's' && *end != 'f') || end[1] != '\0' || !_finite(value) || value < 0 || value > FLT_MAX)
		{
			aRunner.Fail("wait expects a non-negative duration with s or f suffix");
			return;
		}
		aRunner.myWaitRemaining = static_cast<float>(value);
		aRunner.myWaitInFrames = *end == 'f';
		aRunner.myIsWaiting = true;
	}

	void DebugScriptRunner::Step(float aDelta)
	{
		if (myIsWaiting)
		{
			myWaitRemaining -= myWaitInFrames ? 1.f : aDelta;
			if (myWaitRemaining > 0.f)
				return;
			myIsWaiting = false;
		}
		while (myNextLine < myLines.Count())
		{
			Arguments tokens;
			FW_String line = myLines[myNextLine++];
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
			if (myIsWaiting)
				break;
		}
		if (myNextLine == myLines.Count())
		{
			myLines.RemoveAll();
			myNextLine = 0;
		}
	}
}
