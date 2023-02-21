#include "pch.h"
#include "PE_App.h"
#include <Psapi.h>
using namespace pe;
using namespace pe::Reserved;
#pragma warning(disable: 6388)
#pragma warning(disable: 6387)
Bool App::FindProcAddressInLibrary(cStringA sFunc, cStringA sLib, Memory pVal)
{
	if (!Good())
		return false;
	for (
		const List<LoadedLib>* LoadedLibrary = &LoadedLibraries;
		LoadedLibrary != nullptr && !LoadedLibrary->isEmpty();
		LoadedLibrary = LoadedLibrary->Next)
	{
		if (!str::EqualsA(LoadedLibrary->Value->sPath, sLib))
		{
			continue;
		}
		HMODULE hModule = LoadedLibrary->Value->hModule;
		if (!HandleGood(hModule)) {
			SetLastError(ERROR_INVALID_HANDLE,
				(std::string("Impossible to get the HMODULE of ") + sLib).c_str());
			return false;
		}
		*((FARPROC*)pVal) = GetProcAddress(hModule, sFunc);
		if (*((FARPROC*)pVal) == NULL)
		{
			SetLastError(ERROR_INVALID_HANDLE,
				(std::string("Impossible to get the address of ") + sFunc).c_str());
			return false;
		}
		//Process found in already loaded library
		return true;
	}
	HMODULE hLib = LoadLibraryExA(sLib, hProc,
		LOAD_LIBRARY_SEARCH_DEFAULT_DIRS |
		LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32);
	if (HandleGood(hLib))
	{
		LoadedLib lib;
		lib.hModule = hLib;
		str::CopyA(&lib.sPath, sLib);
		LoadedLibraries.push(lib);
		*((FARPROC*)pVal) = GetProcAddress(hLib, sFunc);
		if (*((FARPROC*)pVal))
		{
			return true;
		}
		SetLastError(ERROR_INVALID_HANDLE,
			(std::string("Impossible to find the address of ") + sFunc +
				std::string(" in ") + sLib).c_str());
		return false;
	}
	SetLastError(ERROR_INVALID_HANDLE, "Impossible to load the requested library");
	return false;
}

void App::ListLibraries()
{
	LoadedLibraries.clear();
	if (!Good())
	{
		return;
	}
	Array<HMODULE> aModule;
	aModule.Allocate(512U);
	Dword dwCbNeeded;
	if (K32EnumProcessModules(
		hProc, aModule.addr, 
		sizeof(HMODULE) * aModule.count, &dwCbNeeded))
	{
		dwCbNeeded /= sizeof(HMODULE);
		for (size_t i = 0; i < dwCbNeeded; ++i)
		{
			StringA sModName;
			str::AllocA(&sModName, MAX_PATH);
			if (K32GetModuleFileNameExA(hProc, aModule[i], sModName, MAX_PATH))
			{
				LoadedLib lib;
				lib.hModule = aModule[i];
				str::CopyA(&lib.sPath, sModName);
				LoadedLibraries.push(lib);
			}
		}
	}
	aModule.DeAllocate();
}

Bool App::IsLibraryLoaded(cStringA sLib)const
{
	if (str::LenA(sLib) == 0 || LoadedLibraries.isEmpty())
		return false;
	for (
		const List<LoadedLib>* LoadedLibrary = &LoadedLibraries; 
		LoadedLibrary != nullptr && !LoadedLibrary->isEmpty();
		LoadedLibrary = LoadedLibrary->Next)
	{
		if (str::EqualsA(LoadedLibrary->Value->sPath, sLib))
			return true;
	}
	return false;
}
Bool App::IsLibraryLoaded(cStringW sLib)const
{
	if (str::LenW(sLib) == 0)
		return false;
	StringA AnsiEquivalent = str::WtoA(sLib);
	Bool bReturn = false;
	for (
		const List<LoadedLib>* LoadedLibrary = &LoadedLibraries;
		LoadedLibrary != nullptr && !LoadedLibrary->isEmpty();
		LoadedLibrary = LoadedLibrary->Next)
	{
		if (str::EqualsA(LoadedLibrary->Value->sPath, AnsiEquivalent))
			return true;
	}
	str::FreeA(&AnsiEquivalent, 0);
	return bReturn;
}