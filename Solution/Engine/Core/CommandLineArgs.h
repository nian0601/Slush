#pragma once
#include "FW_GrowingArray.h"
#include "FW_String.h"

namespace Slush
{
	class CommandLineArgs
	{
	public:
		static CommandLineArgs& GetInstance();
		static void Destroy();

		void Parse(int anArgCount, char** anArgValues);

		bool HasFlag(const char* aFlag) const;

	private:
		CommandLineArgs() {};
		~CommandLineArgs() {};
		static CommandLineArgs* ourInstance;

		FW_GrowingArray<FW_String> myArgs;
	};
}
