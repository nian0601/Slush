#include "stdafx.h"
#include "Core/Log.h"
#include "Core/CommandLineArgs.h"
#include "Core/AssetRepository.h"

#include <time.h>
#include <sys/timeb.h>

#include <FW_FileSystem.h>

namespace Slush
{
	Logger::Logger()
	{
		CommandLineArgs& commandLineArgs = CommandLineArgs::GetInstance();
		const char* logName = commandLineArgs.GetString("-logname");
		const bool hasValidLogName = IsValidLogName(logName);
		FW_String logFileName = "debuglog.txt";
		if (hasValidLogName)
		{
			logFileName = logName;
			logFileName += ".txt";
		}

		FW_String absoluteLogPath;
		AssetRepository::GetDebugFilePath(logFileName, absoluteLogPath);

		fopen_s(&myLogFile, absoluteLogPath.GetBuffer(), "w");
		FW_ASSERT(myLogFile != nullptr, "Failed to open %s for writing", logFileName.GetBuffer());

		if (commandLineArgs.HasFlag("-logname") && !hasValidLogName)
		{
			if (logName)
			{
				AddMessage(WARNING, "Rejected -logname value '%s'; using debuglog.txt", logName);
			}
			else
			{
				AddMessage(WARNING, "Missing -logname value; using debuglog.txt");
			}
		}
	}

	Logger::~Logger()
	{
		if (myLogFile)
			fclose(myLogFile);
	}

	bool Logger::IsValidLogName(const char* aLogName)
	{
		if (!aLogName || aLogName[0] == '\0')
			return false;

		for (const char* character = aLogName; *character != '\0'; ++character)
		{
			if (!((*character >= 'A' && *character <= 'Z') ||
				(*character >= 'a' && *character <= 'z') ||
				(*character >= '0' && *character <= '9') ||
				*character == '_' || *character == '-'))
				return false;
		}

		return true;
	}

	void Logger::Flush()
	{
		for (int i = myFlushedEntryCount; i < myEntries.Count(); ++i)
		{
			const LogEntry& entry = myEntries[i];
			fprintf(myLogFile, "[%s] %s\n", Logger::GetSeverityText(entry.mySeverity), entry.myMessage.GetBuffer());
		}

		myFlushedEntryCount = myEntries.Count();
		fflush(myLogFile);
	}

	void Logger::Update()
	{
		if (myFlushTimer.IsStarted() && myFlushTimer.HasExpired() && myEntries.Count() > myFlushedEntryCount)
			Flush();
	}

	void Logger::ForceFlush()
	{
		Flush();
	}

	void Logger::AddMessage(Severity aSeverity, const char *aFormattedString, ...)
	{
		//Get time and store as string in buf
		time_t now = time(0);
		struct tm tstruct;
		char buf[30];
		localtime_s(&tstruct, &now);

		strftime(buf, sizeof(buf), "%H:%M:%S:", &tstruct);

		//Get Miliseconds and store in tStructMilli
		struct _timeb tstructMilli;
		char bufMilli[128];

		_strtime_s(bufMilli);
		_ftime_s(&tstructMilli);

		//Get VA_ARGS and store as string in buffer
		char buffer[4096];
		va_list args;
		va_start(args, aFormattedString);
		vsprintf_s(buffer, aFormattedString, args);
		va_end(args);


		LogEntry& entry = myEntries.Add();
		entry.mySeverity = aSeverity;
		entry.myMessage = "[";
		entry.myMessage += buf;
		entry.myMessage += tstructMilli.millitm;
		entry.myMessage += "]: ";
		entry.myMessage += buffer;

		myFlushTimer.Start(3.f);

		if (myEntries.Count() - myFlushedEntryCount > 25)
			Flush();
	}
}
