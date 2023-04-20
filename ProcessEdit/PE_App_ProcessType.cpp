#include "PE.h"
using namespace pe;

App::ArchitectureType defaultType()
{
	switch (sys::info.wProcessorArchitecture)
	{
	case PROCESSOR_ARCHITECTURE_INTEL:
		return App::x86;
	case PROCESSOR_ARCHITECTURE_IA64:
		return App::Intel64;

	case PROCESSOR_ARCHITECTURE_ARM:
		return App::ARM32;
	case PROCESSOR_ARCHITECTURE_ARM64:
		return App::ARM64;

	case PROCESSOR_ARCHITECTURE_AMD64:
		return App::Generic64;

	default:
		return App::Unknown;
	}
}

Bool App::Is32Bit()const
{
	switch (this->Architecture)
	{
	case App::ARM32:
	case App::x86:
	case App::WOW64:
		return true;
	default:
		return false;
	}
}
Bool App::Is64Bit()const
{
	switch (this->Architecture)
	{
	case App::ARM64:
	case App::x64:
	case App::Intel64:
	case App::Generic64:
		return true;
	default:
		return false;
	}
}
Bool App::GetProcessType()
{
	if (HandleBad(this->hProc))
	{
		return false;
	}

	if (!sys::infoLoaded || !sys::canDetectProcessType || !sys::IsArchitectureKnown())
	{
		return false;
	}

	this->Architecture = defaultType();
	BOOL bResult = FALSE;
	if (!IsWow64Process(this->hProc, &bResult))
	{
		return false;
	}
	if (bResult)
	{
		this->Architecture = App::WOW64;
	}
	if (Is64Bit())
	{
		this->PointerSize = 8U;
	} else {
		this->PointerSize = 4U;
	}
	return true;
}