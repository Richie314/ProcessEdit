#include "pch.h"
#include "PE_App.h"
#include <Psapi.h>
using namespace pe;
using namespace pe::Reserved;
Bool App::InjectFunctionInit(
	cStringA sName, cStringA sLocation, 
	Memory boolStruct, size_t size)
{
	if (!Good() || !sName || !sLocation || !boolStruct)
	{
		SetLastError(ERROR_INVALID_HANDLE,
			"Invalid call");
		return FALSE;
	}
	BasicStruct data = *((BasicStruct*)boolStruct);
	memset(&data, 0, max(size, sizeof(BasicStruct)));
	//Get address for target function
	HMODULE hLoadedLib = LoadLibraryA(sLocation);
	if (HandleBad(hLoadedLib))
	{
		SetLastError(ERROR_INVALID_HANDLE,
			"Impossible to load library");
		return FALSE;
	}
	data.dwAddr = (Dword)GetProcAddress(hLoadedLib, sName);
	FreeLibrary(hLoadedLib);
	//Allocate space for exit code: BOOL (int)
	data.iExitCode = (Int32*)VirtualAllocEx(hProc, NULL, sizeof(Int32),
		MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (data.iExitCode == PE_NULL(Int32))
	{
		DWORD lastError = GetLastError();
		SetLastError(lastError,
			(lastError == ERROR_ACCESS_DENIED) ?
			"Cannot allocate on the process. "
			"Error: ERROR_ACCESS_DENIED" :
			"Cannot allocate on the process.");
		return false;
	}
	//END OF SETUP
	return true;
}
Bool App::InjectFunctionCreateThread(
	Memory pFunc,
	Memory boolStruct, size_t size)
{
	BasicStruct data = *((BasicStruct*)boolStruct);
	//Allocate space for thread call
	Memory lpThread = VirtualAllocEx(hProc, 0, sizeof(BOOLStruct),
		MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (HandleBad(lpThread))
	{
		SetLastError(GetLastError(),
			"Impossible create space for the thread");
		VirtualFreeEx(hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		return FALSE;
	}
	WriteProcessMemory(hProc, lpThread,
		pFunc, size, NULL);
	//Allocate space for thread params
	Memory pParam = VirtualAllocEx(hProc, NULL, size,
		MEM_COMMIT, PAGE_READWRITE);
	if (pParam == NULL)
	{
		DWORD lastError = GetLastError();
		SetLastError(lastError,
			(lastError == ERROR_ACCESS_DENIED) ?
			"Cannot allocate on the process. "
			"Error: ERROR_ACCESS_DENIED" :
			"Cannot allocate on the process.");
		VirtualFreeEx(hProc, lpThread, size,
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		return false;
	}
	WriteProcessMemory(hProc, pParam, &data, size, NULL);
	//Create the thread
	Handle hNewThread = CreateRemoteThread(hProc, NULL, NULL,
		(LPTHREAD_START_ROUTINE)lpThread, pParam, NULL, NULL);
	if (!HandleGood(hNewThread))
	{
		SetLastError(GetLastError(), "Cannot create the thread.");
		VirtualFreeEx(hProc, lpThread, size,
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(hProc, pParam, size,
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		return false;
	}
	//Wait for the function to end
	WaitForSingleObject(hNewThread, INFINITE);
	CloseHandle(hNewThread);
	//Free the allocated space
	VirtualFreeEx(hProc, pParam, size,
		MEM_RELEASE | MEM_DECOMMIT);
	VirtualFreeEx(hProc, lpThread, size,
		MEM_RELEASE | MEM_DECOMMIT);
}
BOOL App::InjectFunctionGetReturnValue(
	Memory boolStruct)
{
	BasicStruct data = *((BasicStruct*)boolStruct);
	//Allocate space for thread call
	BOOL bExit = FALSE;
	ReadProcessMemory(hProc, data.iExitCode, &bExit, sizeof(Int32), NULL);
	VirtualFreeEx(hProc, data.iExitCode, sizeof(BOOL),
		MEM_DECOMMIT | MEM_RELEASE);
	return bExit;
}

BOOL App::InjectVoidFunction(cStringA sName, cStringA sLocation, Memory pFunc)
{
	BOOLStruct data;
	if (!InjectFunctionInit(sName, sLocation, &data, sizeof(BOOLStruct)))
	{
		return FALSE;
	}
	if (!InjectFunctionCreateThread(pFunc, &data, sizeof(BOOLStruct)))
	{
		return FALSE;
	}
	return InjectFunctionGetReturnValue(&data);
}

BOOL App::InjectStringFunctionA(
	cStringA sName, cStringA sLocation,
	Memory pFunc, cStringA sParam)
{
	using typeData = LPCSTRStruct;
	typeData data;
	if (!InjectFunctionInit(sName, sLocation, &data, sizeof(typeData)))
	{
		return FALSE;
	}
	//Code of this specific function
	size_t sLength = str::LenA(sParam) + 1;
	data.pString = (char*)VirtualAllocEx(hProc, NULL, sLength,
		MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (data.pString == PE_NULL(char))
	{
		VirtualFreeEx(hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		DWORD lastError = GetLastError();
		SetLastError(lastError,
			(lastError == ERROR_ACCESS_DENIED) ?
			"Cannot allocate on the process. "
			"Error: ERROR_ACCESS_DENIED" :
			"Cannot allocate on the process.");
		return FALSE;
	}
	WriteProcessMemory(hProc, data.pString, sParam,
		sLength, NULL);
	//End of specific code
	if (!InjectFunctionCreateThread(pFunc, &data, sizeof(typeData)))
	{
		VirtualFreeEx(hProc, data.pString, sLength,
			MEM_RELEASE | MEM_DECOMMIT);
		return FALSE;
	}
	VirtualFreeEx(hProc, data.pString, sLength,
		MEM_RELEASE | MEM_DECOMMIT);
	return InjectFunctionGetReturnValue(&data);
}
BOOL App::InjectStringFunctionW(
	cStringA sName, cStringA sLocation,
	Memory pFunc, cStringW sParam)
{
	using typeData = LPCWSTRStruct;
	typeData data;
	if (!InjectFunctionInit(sName, sLocation, &data, sizeof(typeData)))
	{
		return FALSE;
	}
	//Code of this specific function
	size_t sLength = (str::LenW(sParam) + 1) * sizeof(wchar_t);
	data.pString = (wchar_t*)VirtualAllocEx(hProc, NULL, sLength,
		MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (data.pString == PE_NULL(wchar_t))
	{
		VirtualFreeEx(hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		DWORD lastError = GetLastError();
		SetLastError(lastError,
			(lastError == ERROR_ACCESS_DENIED) ?
			"Cannot allocate on the process. "
			"Error: ERROR_ACCESS_DENIED" :
			"Cannot allocate on the process.");
		return FALSE;
	}
	WriteProcessMemory(hProc, data.pString, sParam,
		sLength, NULL);
	//End of specific code
	if (!InjectFunctionCreateThread(pFunc, &data, sizeof(typeData)))
	{
		VirtualFreeEx(hProc, data.pString, sLength,
			MEM_RELEASE | MEM_DECOMMIT);
		return FALSE;
	}
	VirtualFreeEx(hProc, data.pString, sLength,
		MEM_RELEASE | MEM_DECOMMIT);
	return InjectFunctionGetReturnValue(&data);
}

BOOL App::InjectDWORDFunction(cStringA sName, cStringA sLocation,
	Memory pFunc, Dword dwParam)
{
	using typeData = DWORDStruct;
	typeData data;
	if (!InjectFunctionInit(sName, sLocation, &data, sizeof(typeData)))
	{
		return FALSE;
	}
	//Code of this specific function
	data.dwParam = (DWORD*)VirtualAllocEx(hProc, NULL, sizeof(DWORD),
		MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (data.dwParam == PE_NULL(DWORD))
	{
		VirtualFreeEx(hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		DWORD lastError = GetLastError();
		SetLastError(lastError,
			(lastError == ERROR_ACCESS_DENIED) ?
			"Cannot allocate on the process. "
			"Error: ERROR_ACCESS_DENIED" :
			"Cannot allocate on the process.");
		return FALSE;
	}
	Dword dwCopy = dwParam;
	WriteProcessMemory(hProc, data.dwParam, &dwCopy,
		sizeof(DWORD), NULL);
	//End of specific code
	if (!InjectFunctionCreateThread(pFunc, &data, sizeof(typeData)))
	{
		VirtualFreeEx(hProc, data.dwParam, sizeof(DWORD),
			MEM_RELEASE | MEM_DECOMMIT);
		return FALSE;
	}
	VirtualFreeEx(hProc, data.dwParam, sizeof(DWORD),
		MEM_RELEASE | MEM_DECOMMIT);
	return InjectFunctionGetReturnValue(&data);
}

BOOL App::InjectPrintFunctionA(
	cStringA sName, cStringA sLocation,
	Memory pFunc, Handle hStream, cStringA sParam,
	Dword* dwWritten)
{
	using typeData = PrintStructA;
	typeData data;
	if (!InjectFunctionInit(sName, sLocation, &data, sizeof(typeData)))
	{
		return FALSE;
	}

	//Text param part
	size_t length = str::LenA(sParam) + 1;
	data.sOut = (char*)VirtualAllocEx(hProc, NULL, length,
		MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (data.sOut == PE_NULL(char))
	{
		VirtualFreeEx(hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		DWORD lastError = GetLastError();
		SetLastError(lastError,
			(lastError == ERROR_ACCESS_DENIED) ?
			"Cannot allocate on the process. "
			"Error: ERROR_ACCESS_DENIED" :
			"Cannot allocate on the process.");
		return 0;
	}
	WriteProcessMemory(hProc, data.sOut, sParam,
		length, NULL);
	//Number of char to write
	data.dwWrite = (Dword*)VirtualAllocEx(hProc, NULL,
		sizeof(Dword), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (data.dwWrite == PE_NULL(Dword))
	{
		DWORD lastError = GetLastError();
		SetLastError(lastError,
			(lastError == ERROR_ACCESS_DENIED) ?
			"Cannot allocate on the process. "
			"Error: ERROR_ACCESS_DENIED" :
			"Cannot allocate on the process.");
		VirtualFreeEx(hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(hProc, data.sOut, length,
			MEM_RELEASE | MEM_DECOMMIT);
		return FALSE;
	}
	WriteProcessMemory(hProc, data.dwWrite,
		&length, sizeof(Dword), NULL);

	//Number of chars written (pointer to variable)
	//Allocate DWORD and get its address
	Dword* lpWritten = (Dword*)VirtualAllocEx(hProc, NULL,
		sizeof(Dword), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (lpWritten == PE_NULL(Dword))
	{
		DWORD lastError = GetLastError();
		SetLastError(lastError,
			(lastError == ERROR_ACCESS_DENIED) ?
			"Cannot allocate on the process. "
			"Error: ERROR_ACCESS_DENIED" :
			"Cannot allocate on the process.");
		VirtualFreeEx(hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(hProc, data.sOut, length,
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(hProc, data.dwWrite,
			sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
		return FALSE;
	}

	data.dwWritten = (Dword**)VirtualAllocEx(hProc, NULL,
		sizeof(Dword*), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (data.dwWritten == PE_NULL(Dword*))
	{
		DWORD lastError = GetLastError();
		SetLastError(lastError,
			(lastError == ERROR_ACCESS_DENIED) ?
			"Cannot allocate on the process. "
			"Error: ERROR_ACCESS_DENIED" :
			"Cannot allocate on the process.");
		VirtualFreeEx(hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(hProc, data.sOut, length,
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(hProc, data.dwWrite,
			sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(hProc, lpWritten,
			sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
		return 0;
	}
	WriteProcessMemory(hProc, data.dwWrite,
		&lpWritten, //Content of lpWritten variable -> address of the allocated DWORD
		sizeof(Dword), NULL);
	//End of specific code
	if (!InjectFunctionCreateThread(pFunc, &data, sizeof(typeData)))
	{
		VirtualFreeEx(hProc, data.sOut, length,
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(hProc, data.dwWrite,
			sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(hProc, lpWritten,
			sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(hProc, data.dwWritten,
			sizeof(Dword*), MEM_DECOMMIT | MEM_RELEASE);
		return FALSE;
	}
	//We set the dwWritten variable to the same value
	//that has in the other process
	ReadProcessMemory(hProc, lpWritten, dwWritten, sizeof(Dword), NULL);
	VirtualFreeEx(hProc, data.sOut, length,
		MEM_DECOMMIT | MEM_RELEASE);
	VirtualFreeEx(hProc, data.dwWrite,
		sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
	VirtualFreeEx(hProc, lpWritten,
		sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
	VirtualFreeEx(hProc, data.dwWritten,
		sizeof(Dword*), MEM_DECOMMIT | MEM_RELEASE);
	return InjectFunctionGetReturnValue(&data);
}
BOOL App::InjectPrintFunctionW(
	cStringA sName, cStringA sLocation,
	Memory pFunc, Handle hStream, cStringW sParam,
	Dword* dwWritten)
{
	using typeData = PrintStructW;
	typeData data;
	if (!InjectFunctionInit(sName, sLocation, &data, sizeof(typeData)))
	{
		return FALSE;
	}

	//Text param part
	size_t length = (str::LenW(sParam) + 1) * sizeof(wchar_t);
	data.sOut = (wchar_t*)VirtualAllocEx(hProc, NULL, length,
		MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (data.sOut == PE_NULL(wchar_t))
	{
		VirtualFreeEx(hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		DWORD lastError = GetLastError();
		SetLastError(lastError,
			(lastError == ERROR_ACCESS_DENIED) ?
			"Cannot allocate on the process. "
			"Error: ERROR_ACCESS_DENIED" :
			"Cannot allocate on the process.");
		return 0;
	}
	WriteProcessMemory(hProc, data.sOut, sParam,
		length, NULL);
	//Number of char to write
	data.dwWrite = (Dword*)VirtualAllocEx(hProc, NULL,
		sizeof(Dword), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (data.dwWrite == PE_NULL(Dword))
	{
		DWORD lastError = GetLastError();
		SetLastError(lastError,
			(lastError == ERROR_ACCESS_DENIED) ?
			"Cannot allocate on the process. "
			"Error: ERROR_ACCESS_DENIED" :
			"Cannot allocate on the process.");
		VirtualFreeEx(hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(hProc, data.sOut, length,
			MEM_RELEASE | MEM_DECOMMIT);
		return FALSE;
	}
	WriteProcessMemory(hProc, data.dwWrite,
		&length, sizeof(Dword), NULL);

	//Number of chars written (pointer to variable)
	//Allocate DWORD and get its address
	Dword* lpWritten = (Dword*)VirtualAllocEx(hProc, NULL,
		sizeof(Dword), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (lpWritten == PE_NULL(Dword))
	{
		DWORD lastError = GetLastError();
		SetLastError(lastError,
			(lastError == ERROR_ACCESS_DENIED) ?
			"Cannot allocate on the process. "
			"Error: ERROR_ACCESS_DENIED" :
			"Cannot allocate on the process.");
		VirtualFreeEx(hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(hProc, data.sOut, length,
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(hProc, data.dwWrite,
			sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
		return FALSE;
	}

	data.dwWritten = (Dword**)VirtualAllocEx(hProc, NULL,
		sizeof(Dword*), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (data.dwWritten == PE_NULL(Dword*))
	{
		DWORD lastError = GetLastError();
		SetLastError(lastError,
			(lastError == ERROR_ACCESS_DENIED) ?
			"Cannot allocate on the process. "
			"Error: ERROR_ACCESS_DENIED" :
			"Cannot allocate on the process.");
		VirtualFreeEx(hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(hProc, data.sOut, length,
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(hProc, data.dwWrite,
			sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(hProc, lpWritten,
			sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
		return 0;
	}
	WriteProcessMemory(hProc, data.dwWrite,
		&lpWritten, //Content of lpWritten variable -> address of the allocated DWORD
		sizeof(Dword), NULL);
	//End of specific code
	if (!InjectFunctionCreateThread(pFunc, &data, sizeof(typeData)))
	{
		VirtualFreeEx(hProc, data.sOut, length,
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(hProc, data.dwWrite,
			sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(hProc, lpWritten,
			sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(hProc, data.dwWritten,
			sizeof(Dword*), MEM_DECOMMIT | MEM_RELEASE);
		return FALSE;
	}
	//We set the dwWritten variable to the same value
	//that has in the other process
	ReadProcessMemory(hProc, lpWritten, dwWritten, sizeof(Dword), NULL);
	VirtualFreeEx(hProc, data.sOut, length,
		MEM_DECOMMIT | MEM_RELEASE);
	VirtualFreeEx(hProc, data.dwWrite,
		sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
	VirtualFreeEx(hProc, lpWritten,
		sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
	VirtualFreeEx(hProc, data.dwWritten,
		sizeof(Dword*), MEM_DECOMMIT | MEM_RELEASE);
	return InjectFunctionGetReturnValue(&data);
}