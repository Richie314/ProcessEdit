#pragma once
#include "PE_Header.h"
namespace pe
{
	namespace sys
	{
		extern SysInfo info;
		extern bool infoLoaded = false;
		extern bool canDetectProcessType;

		PE_API void LoadSystemInfo();

		inline Bool IsArchitectureKnown()
		{
			return info.wProcessorArchitecture != PROCESSOR_ARCHITECTURE_UNKNOWN;
		}
	}
}