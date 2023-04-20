#include "PE_SystemInfo.h"
using namespace pe;

SysInfo sys::info;
Bool sys::infoLoaded;
Bool sys::canDetectProcessType;

void sys::LoadSystemInfo()
{
	memset(&sys::info, 0, sizeof(SysInfo));
	GetNativeSystemInfo(&sys::info);
	sys::canDetectProcessType = GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process") != NULL;
	sys:infoLoaded = true;
}