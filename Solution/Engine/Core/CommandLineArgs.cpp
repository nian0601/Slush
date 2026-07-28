#include "stdafx.h"
#include "Core/CommandLineArgs.h"

namespace Slush
{
	CommandLineArgs* CommandLineArgs::ourInstance = nullptr;
	CommandLineArgs& CommandLineArgs::GetInstance()
	{
		if (!ourInstance)
			ourInstance = new CommandLineArgs();

		return *ourInstance;
	}

	void CommandLineArgs::Destroy()
	{
		FW_SAFE_DELETE(ourInstance);
	}

	void CommandLineArgs::Parse(int anArgCount, char** anArgValues)
	{
		for (int i = 1; i < anArgCount; ++i)
			myArgs.Add(FW_String(anArgValues[i]));
	}

	bool CommandLineArgs::HasFlag(const char* aFlag) const
	{
		for (int i = 0; i < myArgs.Count(); ++i)
		{
			if (myArgs[i] == aFlag)
				return true;
		}

		return false;
	}
}
