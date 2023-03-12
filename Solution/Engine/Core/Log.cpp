#include "stdafx.h"
#include "Core/Log.h"

#include <time.h>
#include <sys/timeb.h>

namespace Slush
{
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
		perror(buffer);
		va_end(args);


		LogEntry& entry = myEntries.Add();
		entry.mySeverity = aSeverity;
		entry.myMessage = "[";
		entry.myMessage += buf;
		entry.myMessage += tstructMilli.millitm;
		entry.myMessage += "]: ";
		entry.myMessage += buffer;
	}
}