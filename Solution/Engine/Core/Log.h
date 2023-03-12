#pragma once

#include "FW_String.h"
#include "FW_GrowingArray.h"
#include "Core/Engine.h"

namespace Slush
{
	class Logger
	{
	public:
		enum Severity
		{
			INFO,
			DEBUG,
			WARNING,
			ERROR,
		};

		static int GetSeverityColor(Severity aSeverity)
		{
			switch (aSeverity)
			{
			case Slush::Logger::INFO:
				return 0xFF10E07C;
			case Slush::Logger::DEBUG:
				return 0xFFFFFF;
			case Slush::Logger::WARNING:
				return 0xFFE6D30B;
			case Slush::Logger::ERROR:
				return 0xFFDB4242;
			}

			return 0xFF339911;
		}

		static Vector3f GetSeverityColorVec(Severity aSeverity)
		{
			int color = GetSeverityColor(aSeverity);
			return FW_ARGB_To_Vector(color);
		}

		struct LogEntry
		{
			Severity mySeverity;
			FW_String myMessage;
		};

		void AddMessage(Severity aSeverity, const char *aFormattedString, ...);

		const FW_GrowingArray<LogEntry>& GetEntries() const { return myEntries; }
	private:
		FW_GrowingArray<LogEntry> myEntries;
	};
}

#define SLUSH_INFO(...) Slush::Engine::GetInstance().GetLogger().AddMessage(Slush::Logger::INFO, __VA_ARGS__);
#define SLUSH_DEBUG(...) Slush::Engine::GetInstance().GetLogger().AddMessage(Slush::Logger::DEBUG, __VA_ARGS__);
#define SLUSH_WARNING(...) Slush::Engine::GetInstance().GetLogger().AddMessage(Slush::Logger::WARNING, __VA_ARGS__);
#define SLUSH_ERROR(...) Slush::Engine::GetInstance().GetLogger().AddMessage(Slush::Logger::ERROR, __VA_ARGS__);