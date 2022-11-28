#include "pch.h"
#include "PE_App.h"
#include <Psapi.h>
using namespace pe;
using namespace pe::Reserved;
#pragma warning(disable: 6388)
Bool App::FindProcAddressInLibrary(LPCSTR sFunc, LPCSTR sLib, Memory pVal)
{
	if (!Good())
		return false;
	size_t contains = ListLoadedLibraries.size;
	for (size_t i = 0; i < contains; i++)
	{
		if (ListLoadedLibraries[i].sPath.compare(sLib))
			contains = i;
	}
	if (contains < ListLoadedLibraries.size)
	{
		HMODULE hModule = ListLoadedLibraries[contains].hModule;
		if (!HandleGood(hModule)) {
			SetLastError(PIE_ERROR_HANDLE_ERROR,
				SuperAnsiString("Impossible to get the HMODULE of ") + sLib);
			return false;
		}
		*((FARPROC*)pVal) = GetProcAddress(hModule, sFunc);
		if (*((FARPROC*)pVal) == NULL)
		{
			SetLastError(PIE_ERROR_MEMORY_ERROR | PIE_ERROR_HANDLE_ERROR,
				SuperAnsiString("Impossible to find the address of ") + sFunc +
				" in " + sLib);
			return false;
		}
		return true;
	}
	HMODULE hLib = LoadLibraryExA(sLib, hProc,
		LOAD_LIBRARY_SEARCH_DEFAULT_DIRS |
		LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32);
	if (HandleGood(hLib))
	{
		ListLoadedLibraries.push_back({ sLib, hLib });
		*((FARPROC*)pVal) = GetProcAddress(hLib, sFunc);
		if (*((FARPROC*)pVal))
		{
			return true;
		}
		SetLastError(PIE_ERROR_MEMORY_ERROR | PIE_ERROR_HANDLE_ERROR,
			SuperAnsiString("Impossible to find the address of ") + sFunc +
			" in " + sLib);
		return false;
	}
	SetLastError(PIE_ERROR_HANDLE_ERROR, "Impossible to load the requested library");
	return false;
}

void App::ListLibraries()
{
	ListLoadedLibraries.clear();
	if (Good())
	{
		HMODULE aModule[512];
		Dword dwCbNeeded;
		if (K32EnumProcessModules(hProc, aModule, sizeof(HMODULE) * 512, &dwCbNeeded))
		{
			for (size_t i = 0; i < (dwCbNeeded / sizeof(HMODULE)); i++)
			{
				char sModName[MAX_PATH];
				memset(sModName, 0, MAX_PATH);
				if (K32GetModuleFileNameExA(hProc, aModule[i], sModName, MAX_PATH))
				{
					ListLoadedLibraries.push_back({ sModName, aModule[i] });
				}
			}
		}
	}
}