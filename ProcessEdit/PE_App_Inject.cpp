#include "pch.h"
#include "PE_App.h"
#include <Psapi.h>
using namespace pe;
using namespace pe::Reserved;


Bool App::InjectDll(cStringA dllPath)
{
	size_t length = str::LenA(dllPath);
	if (!Good() || length < 5U)
		return false;
	Memory mem = VirtualAllocEx(hProc, 0, length, MEM_COMMIT, PAGE_READWRITE);
	if (!mem)
	{
		return false;
	}
	if (!WriteProcessMemory(hProc, mem,
		(LPCVOID)dllPath, length, PE_NULL(size_t)))
	{
		//VirtualFreeEx(hProc, mem, path.length(), MEM_RELEASE);
		VirtualFreeEx(hProc, mem, 0, MEM_RELEASE);
		SetLastError(GetLastError(),
			"Cannot write in the process");
		return false;
	}
	HANDLE hLoadThread = CreateRemoteThread(hProc, 0, 0,
		(LPTHREAD_START_ROUTINE)GetProcAddress(
			GetModuleHandleA("Kernel32.dll"),
			"LoadLibraryA"), mem, 0, 0);
	if (HandleBad(hLoadThread))
	{
		//VirtualFreeEx(hProc, mem, path.length(), MEM_RELEASE);
		VirtualFreeEx(hProc, mem, 0, MEM_RELEASE);
		SetLastError(GetLastError(),
			"Cannot create Thread in the process");
		return false;
	}
	WaitForSingleObject(hLoadThread, INFINITE);
	//VirtualFreeEx(hProc, mem, path.length(), MEM_RELEASE);
	VirtualFreeEx(hProc, mem, 0, MEM_RELEASE);
	return true;
}
Bool App::InjectDll(cStringW dllPath)
{
	size_t length = str::LenW(dllPath);
	if (!Good() || length < 5U)
		return false;
	length *= sizeof(Wchar);
	Memory mem = VirtualAllocEx(hProc, 0, length,
		MEM_COMMIT, PAGE_READWRITE);
	if (mem)
	{
		if (!WriteProcessMemory(hProc, mem,
			(LPCVOID)dllPath, length, PE_NULL(size_t)))
		{
			//VirtualFreeEx(hProc, mem, path.length() * 2, MEM_RELEASE);
			VirtualFreeEx(hProc, mem, 0, MEM_RELEASE);
			SetLastError(GetLastError(),
				"Cannot write in the process");
			return false;
		}
		HANDLE hLoadThread = CreateRemoteThread(hProc, 0, 0,
			(LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleW(L"Kernel32.dll"),
				"LoadLibraryW"), mem, 0, 0);
		if (HandleBad(hLoadThread))
		{
			//VirtualFreeEx(hProc, mem, path.length(), MEM_RELEASE);
			VirtualFreeEx(hProc, mem, 0, MEM_RELEASE);
			SetLastError(GetLastError(),
				"Cannot create Thread in the process");
			return false;
		}
		WaitForSingleObject(hLoadThread, INFINITE);
		VirtualFreeEx(hProc, mem, length, MEM_RELEASE);
		return true;
	}
	return false;
}
//////////////////////////////////////////////////////////
UInt PE_CALL RemoteMessageBoxThreadA(__MessageBoxAData* data)
{
	*(data->iExitCode) = ((__MessageBoxAProc)(data->dwAddr))(data->hWnd,
		data->lpTitle, data->lpMessage, data->uType);
	return *(data->iExitCode);
}
UInt PE_CALL RemoteMessageBoxThreadW(__MessageBoxWData* data)
{
	*(data->iExitCode) = ((__MessageBoxWProc)(data->dwAddr))(data->hWnd,
		data->lpTitle, data->lpMessage, data->uType);
	return *(data->iExitCode);
}
//////////////////////////////////////////////////////////
Int32 App::InjectMessageBox(Hwnd hWnd, cStringA sCaption,
	cStringA sMessage, UInt dwType)
{
	if (!Good()) return FALSE;
	using typeData = __MessageBoxAData;
	typeData data;
	//Setup the parameters
	memset(&data, 0, sizeof(typeData));
	//Get address for MessageBoxA
	HMODULE hUser32 = LoadLibraryA("user32.dll");
	if (!HandleGood(hUser32))
	{
		SetLastError(ERROR_INVALID_HANDLE,
			"Impossible to load user32.dll");
		return FALSE;
	}
	data.dwAddr = (Dword)GetProcAddress(hUser32, "MessageBoxA");
	FreeLibrary(hUser32);
	//Allocate space for exit code of MessageBoxA (int)
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
		return FALSE;
	}
	//Allocate space for parameters
	if (sCaption)
	{
		data.lpTitle = (char*)VirtualAllocEx(
			hProc, NULL, str::LenA(sCaption),
			MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (data.lpTitle == PE_NULL(char))
		{
			DWORD lastError = GetLastError();
			SetLastError(lastError,
				(lastError == ERROR_ACCESS_DENIED) ?
				"Cannot allocate on the process. "
				"Error: ERROR_ACCESS_DENIED" :
				"Cannot allocate on the process.");
		} else {
			WriteProcessMemory(
				hProc, data.lpTitle, sCaption,
				str::LenA(sCaption), NULL);
		}
	}
	if (sMessage)
	{
		data.lpMessage = (char*)VirtualAllocEx(
			hProc, NULL, str::LenA(sMessage),
			MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (data.lpMessage == PE_NULL(char))
		{
			DWORD lastError = GetLastError();
			SetLastError(lastError,
				(lastError == ERROR_ACCESS_DENIED) ? 
				"Cannot allocate on the process. "
				"Error: ERROR_ACCESS_DENIED" :
				"Cannot allocate on the process.");
		}
		else {
			WriteProcessMemory(
				hProc, data.lpMessage, sMessage,
				str::LenA(sMessage), NULL);
		}
	}
	//Allocate space for thread call
	Memory lpThread = VirtualAllocEx(hProc, 0, sizeof(typeData),
		MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (HandleBad(lpThread))
	{
		//Fatal error
		SetLastError(GetLastError(),
			"Impossible create space for the thread");
		//VirtualFreeEx(hProc, data.iExitCode, sizeof(Int32),
		//	MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(hProc, data.iExitCode, 0, MEM_RELEASE);
		//Free allocation if it was performed
		if (sCaption)
			VirtualFreeEx(hProc, data.lpTitle, 0, MEM_RELEASE);
		if (sMessage)
			VirtualFreeEx(hProc, data.lpMessage, 0, MEM_RELEASE);
		return FALSE;
	}
	WriteProcessMemory(hProc, lpThread,
		(Memory)RemoteMessageBoxThreadA, sizeof(typeData), NULL);
	//Allocate space for thread params
	Memory pParam = VirtualAllocEx(hProc, NULL, sizeof(typeData),
		MEM_COMMIT, PAGE_READWRITE);

	if (pParam == NULL)
	{
		DWORD lastError = GetLastError();
		SetLastError(lastError,
			(lastError == ERROR_ACCESS_DENIED) ?
			"Cannot allocate on the process. "
			"Error: ERROR_ACCESS_DENIED" :
			"Cannot allocate on the process.");
		/*VirtualFreeEx(hProc, lpThread, sizeof(typeData),
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);*/
		VirtualFreeEx(hProc, lpThread, 0, MEM_RELEASE);
		VirtualFreeEx(hProc, data.iExitCode, 0, MEM_RELEASE);
		if (sCaption)
			VirtualFreeEx(hProc, data.lpTitle, 0, MEM_RELEASE);
		if (sMessage)
			VirtualFreeEx(hProc, data.lpMessage, 0, MEM_RELEASE);
		return FALSE;
	}
	WriteProcessMemory(hProc, pParam, &data, sizeof(typeData), NULL);
	//Create the thread
	Handle hNewThread = CreateRemoteThread(hProc, NULL, NULL,
		(LPTHREAD_START_ROUTINE)lpThread, pParam, NULL, NULL);
	if (!HandleGood(hNewThread))
	{
		SetLastError(GetLastError(), "Cannot create the thread.");
		VirtualFreeEx(hProc, lpThread, 0, MEM_RELEASE);
		VirtualFreeEx(hProc, pParam, 0, MEM_RELEASE);
		VirtualFreeEx(hProc, data.iExitCode, 0, MEM_RELEASE);
		if (sCaption)
			VirtualFreeEx(hProc, data.lpTitle, 0, MEM_RELEASE);
		if (sMessage)
			VirtualFreeEx(hProc, data.lpMessage, 0, MEM_RELEASE);
		return FALSE;
	}
	//Wait for MessageBoxA to end
	WaitForSingleObject(hNewThread, INFINITE);
	CloseHandle(hNewThread);
	//Free the allocated space
	VirtualFreeEx(hProc, pParam, 0, MEM_RELEASE);
	VirtualFreeEx(hProc, lpThread, 0, MEM_RELEASE);

	if (sCaption)
		VirtualFreeEx(hProc, data.lpTitle, 0, MEM_RELEASE);
	if (sMessage)
		VirtualFreeEx(hProc, data.lpMessage, 0, MEM_RELEASE);

	//Get the return value
	Int32 exit_ = FALSE;
	ReadProcessMemory(hProc, data.iExitCode, &exit_, sizeof(Int32), NULL);
	VirtualFreeEx(hProc, data.iExitCode, 0, MEM_RELEASE);
	return exit_;//Everything is done!
}
Int32 App::InjectMessageBox(Hwnd hWnd, cStringW sCaption,
	cStringW sMessage, UInt dwType)
{
	if (!Good()) return FALSE;
	using typeData = __MessageBoxWData;
	typeData data;
	//Setup the parameters
	memset(&data, 0, sizeof(typeData));
	//Get address for MessageBoxW
	HMODULE hUser32 = LoadLibraryW(L"user32.dll");
	if (!HandleGood(hUser32))
	{
		SetLastError(ERROR_INVALID_HANDLE,
			"Impossible to load user32.dll");
		return FALSE;
	}
	data.dwAddr = (Dword)GetProcAddress(hUser32, "MessageBoxW");
	FreeLibrary(hUser32);
	//Allocate space for exit code of MessageBoxW (int)
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
		return FALSE;
	}
	//Allocate space for parameters
	if (sCaption)
	{
		data.lpTitle = (wchar_t*)VirtualAllocEx(
			hProc, NULL, str::LenW(sCaption) * 2,
			MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (data.lpTitle == PE_NULL(wchar_t))
		{
			DWORD lastError = GetLastError();
			SetLastError(lastError,
				(lastError == ERROR_ACCESS_DENIED) ?
				"Cannot allocate on the process. "
				"Error: ERROR_ACCESS_DENIED" :
				"Cannot allocate on the process.");
		}
		else {
			WriteProcessMemory(
				hProc, data.lpTitle, sCaption,
				str::LenW(sCaption) * 2, NULL);
		}
	}
	if (sMessage)
	{
		data.lpMessage = (wchar_t*)VirtualAllocEx(
			hProc, NULL, str::LenW(sMessage) * 2,
			MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (data.lpMessage == PE_NULL(wchar_t))
		{
			DWORD lastError = GetLastError();
			SetLastError(lastError,
				(lastError == ERROR_ACCESS_DENIED) ?
				"Cannot allocate on the process. "
				"Error: ERROR_ACCESS_DENIED" :
				"Cannot allocate on the process.");
		}
		else {
			WriteProcessMemory(
				hProc, data.lpMessage, sMessage,
				str::LenW(sMessage) * 2, NULL);
		}
	}
	//Allocate space for thread call
	Memory lpThread = VirtualAllocEx(hProc, 0, sizeof(typeData),
		MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (lpThread == NULL)
	{
		//Fatal error
		SetLastError(GetLastError(),
			"Impossible create space for the thread");
		//VirtualFreeEx(hProc, data.iExitCode, sizeof(Int32),
		//	MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(hProc, data.iExitCode, 0, MEM_RELEASE);
		//Free allocation if it was performed
		if (sCaption)
			VirtualFreeEx(hProc, data.lpTitle, 0, MEM_RELEASE);
		if (sMessage)
			VirtualFreeEx(hProc, data.lpMessage, 0, MEM_RELEASE);
		return FALSE;
	}
	WriteProcessMemory(hProc, lpThread,
		(Memory)RemoteMessageBoxThreadW, sizeof(typeData), NULL);
	//Allocate space for thread params
	Memory pParam = VirtualAllocEx(hProc, NULL, sizeof(typeData),
		MEM_COMMIT, PAGE_READWRITE);

	if (pParam == NULL)
	{
		DWORD lastError = GetLastError();
		SetLastError(lastError,
			(lastError == ERROR_ACCESS_DENIED) ?
			"Cannot allocate on the process. "
			"Error: ERROR_ACCESS_DENIED" :
			"Cannot allocate on the process.");
		/*VirtualFreeEx(hProc, lpThread, sizeof(typeData),
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);*/
		VirtualFreeEx(hProc, lpThread, 0, MEM_RELEASE);
		VirtualFreeEx(hProc, data.iExitCode, 0, MEM_RELEASE);
		if (sCaption)
			VirtualFreeEx(hProc, data.lpTitle, 0, MEM_RELEASE);
		if (sMessage)
			VirtualFreeEx(hProc, data.lpMessage, 0, MEM_RELEASE);
		return FALSE;
	}
	WriteProcessMemory(hProc, pParam, &data, sizeof(typeData), NULL);
	//Create the thread
	Handle hNewThread = CreateRemoteThread(hProc, NULL, NULL,
		(LPTHREAD_START_ROUTINE)lpThread, pParam, NULL, NULL);
	if (!HandleGood(hNewThread))
	{
		SetLastError(GetLastError(), "Cannot create the thread.");
		VirtualFreeEx(hProc, lpThread, 0, MEM_RELEASE);
		VirtualFreeEx(hProc, pParam, 0, MEM_RELEASE);
		VirtualFreeEx(hProc, data.iExitCode, 0, MEM_RELEASE);
		if (sCaption)
			VirtualFreeEx(hProc, data.lpTitle, 0, MEM_RELEASE);
		if (sMessage)
			VirtualFreeEx(hProc, data.lpMessage, 0, MEM_RELEASE);
		return FALSE;
	}
	//Wait for MessageBoxA to end
	WaitForSingleObject(hNewThread, INFINITE);
	CloseHandle(hNewThread);
	//Free the allocated space
	VirtualFreeEx(hProc, pParam, 0, MEM_RELEASE);
	VirtualFreeEx(hProc, lpThread, 0, MEM_RELEASE);

	if (sCaption)
		VirtualFreeEx(hProc, data.lpTitle, 0, MEM_RELEASE);
	if (sMessage)
		VirtualFreeEx(hProc, data.lpMessage, 0, MEM_RELEASE);

	//Get the return value
	Int32 exit_ = FALSE;
	ReadProcessMemory(hProc, data.iExitCode, &exit_, sizeof(Int32), NULL);
	VirtualFreeEx(hProc, data.iExitCode, 0, MEM_RELEASE);
	return exit_;//Everything is done!
}