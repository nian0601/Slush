#pragma once
#include "FW_GrowingArray.h"
#include "FW_String.h"

namespace Slush
{
	class CommandLineArgs
	{
	public:
		// Public so tests can build throwaway instances, normal access is GetInstance()
		CommandLineArgs() {};
		~CommandLineArgs() {};

		static CommandLineArgs& GetInstance();
		static void Destroy();

		void Parse(int anArgCount, char** anArgValues);

		bool HasFlag(const char* aFlag) const;
		const char* GetString(const char* aFlag) const;

	private:
		static CommandLineArgs* ourInstance;

		FW_GrowingArray<FW_String> myArgs;
	};
}
