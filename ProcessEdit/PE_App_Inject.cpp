#include "pch.h"
#include "Pie_App.h"
#include <Psapi.h>
using namespace Pie_314;
using namespace Pie_314::Reserved;

Bool FreeHook(Reserved::Hook& hook)
{
	Bool bVal = static_cast<Bool>(VirtualFree(hook.pOldFunc, 0, MEM_RELEASE));
	memset(&hook, 0, sizeof(Reserved::Hook));
	return bVal;
}

Bool App::InjectDll(const SuperAnsiString& dllPath)
{
	if (!this->Good() ||
		(!dllPath.endsWith({ ".dll", 4 })) || dllPath.length() < 5U)
		return false;
	SuperAnsiString path = dllPath + "\0";
	Memory mem = VirtualAllocEx(this->hProc, 0, path.length(), MEM_COMMIT, PAGE_READWRITE);
	if (mem)
	{
		if (!WriteProcessMemory(this->hProc, mem,
			(LPCVOID)path.c_str(), path.length(), PIE_NULL(Dword)))
		{
			//VirtualFreeEx(this->hProc, mem, path.length(), MEM_RELEASE);
			VirtualFreeEx(this->hProc, mem, 0, MEM_RELEASE);
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR,
				"Cannot write in the process");
			return false;
		}
		HANDLE hLoadThread = CreateRemoteThread(this->hProc, 0, 0,
			(LPTHREAD_START_ROUTINE)GetProcAddress(
				GetModuleHandleA("Kernel32.dll"),
				"LoadLibraryA"), mem, 0, 0);
		if (!HandleGood(hLoadThread))
		{
			//VirtualFreeEx(this->hProc, mem, path.length(), MEM_RELEASE);
			VirtualFreeEx(this->hProc, mem, 0, MEM_RELEASE);
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR | PIE_ERROR_THREAD_ERROR,
				"Cannot create Thread in the process");
			return false;
		}
		WaitForSingleObject(hLoadThread, INFINITE);
		//VirtualFreeEx(this->hProc, mem, path.length(), MEM_RELEASE);
		VirtualFreeEx(this->hProc, mem, 0, MEM_RELEASE);
		return true;
	}
	return false;
}
Bool App::InjectDll(const SuperUnicodeString& dllPath)
{
	if (!this->Good() ||
		(!dllPath.endsWith({ L".dll",4 })) || dllPath.length() < 5U)
		return false;
	SuperUnicodeString path = dllPath + L" ";
	path.last() = L'\0';
	Memory mem = VirtualAllocEx(this->hProc, 0, path.length() * 2,
		MEM_COMMIT, PAGE_READWRITE);
	if (mem)
	{
		if (!WriteProcessMemory(this->hProc, mem,
			(LPCVOID)path.c_str(), path.length() * 2, PIE_NULL(Dword)))
		{
			//VirtualFreeEx(this->hProc, mem, path.length() * 2, MEM_RELEASE);
			VirtualFreeEx(this->hProc, mem, 0, MEM_RELEASE);
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR,
				"Cannot write in the process");
			return false;
		}
		HANDLE hLoadThread = CreateRemoteThread(this->hProc, 0, 0,
			(LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleW(L"Kernel32.dll"),
				"LoadLibraryW"), mem, 0, 0);
		if (!HandleGood(hLoadThread))
		{
			//VirtualFreeEx(this->hProc, mem, path.length(), MEM_RELEASE);
			VirtualFreeEx(this->hProc, mem, 0, MEM_RELEASE);
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR | PIE_ERROR_THREAD_ERROR,
				"Cannot create Thread in the process");
			return false;
		}
		WaitForSingleObject(hLoadThread, INFINITE);
		//VirtualFreeEx(this->hProc, mem, path.length() * 2, MEM_RELEASE);
		VirtualFreeEx(this->hProc, mem, path.length() * 2, MEM_RELEASE);
		return true;
	}
	return false;
}
//////////////////////////////////////////////////////////
UInt P_ENTRY RemoteMessageBoxThreadA(__MessageBoxAData* data)
{
	*(data->iExitCode) = ((__MessageBoxAProc)(data->dwAddr))(data->hWnd,
		data->lpTitle, data->lpMessage, data->uType);
	return *(data->iExitCode);
}
UInt P_ENTRY RemoteMessageBoxThreadW(__MessageBoxWData* data)
{
	*(data->iExitCode) = ((__MessageBoxWProc)(data->dwAddr))(data->hWnd,
		data->lpTitle, data->lpMessage, data->uType);
	return *(data->iExitCode);
}
//////////////////////////////////////////////////////////
Int32 App::InjectMessageBox(Hwnd hWnd, const SuperAnsiString& sCaption,
	const SuperAnsiString& sMessage, UInt dwType)
{
	if (!this->Good()) return 0;
	using typeData = __MessageBoxAData;
	typeData data;
	//Setup the parameters
	memset(&data, 0, sizeof(typeData));
	//Get address for MessageBoxA
	HMODULE hUser32 = LoadLibraryA("user32.dll");
	if (!HandleGood(hUser32))
	{
		this->SetLastError(PIE_ERROR_HANDLE_ERROR,
			"Impossible to load user32.dll");
		return 0;
	}
	data.dwAddr = (Dword)GetProcAddress(hUser32, "MessageBoxA");
	FreeLibrary(hUser32);
	Bool bCaption = sCaption.Good();
	Bool bMessage = sMessage.Good();
	//Allocate space for exit code of MessageBoxA (int)
	data.iExitCode = (Int32*)VirtualAllocEx(this->hProc, NULL, sizeof(Int32),
		MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (data.iExitCode == PIE_NULL(Int32))
	{
		if (GetLastError() == ERROR_ACCESS_DENIED)
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR |
				PIE_ERROR_ACCESSERROR, "Cannot allocate on the process. "
				"Error: ERROR_ACCESS_DENIED");
		else
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR,
				"Cannot allocate on the process.");
		return 0;
	}
	//Allocate space for parameters
	if (bCaption)
	{
		data.lpTitle = (char*)VirtualAllocEx(this->hProc, NULL, sCaption.length(),
			MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (data.lpTitle == PIE_NULL(char))
		{
			//VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			//	MEM_DECOMMIT | MEM_RELEASE); 
			VirtualFreeEx(this->hProc, data.iExitCode, 0, MEM_RELEASE);
			if (GetLastError() == ERROR_ACCESS_DENIED)
				this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR |
					PIE_ERROR_ACCESSERROR, "Cannot allocate on the process. "
					"Error: ERROR_ACCESS_DENIED");
			else
				this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR,
					"Cannot allocate on the process.");
			return 0;
		}
		WriteProcessMemory(this->hProc, data.lpTitle, sCaption.c_str(),
			sCaption.length(), NULL);
	}
	if (bMessage)
	{
		data.lpMessage = (char*)VirtualAllocEx(this->hProc, NULL, sMessage.length(),
			MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (data.lpMessage == PIE_NULL(char))
		{
			if (bCaption)
			{
				//VirtualFreeEx(this->hProc, data.lpTitle, sCaption.length(),
				//	MEM_RELEASE | MEM_DECOMMIT);
				VirtualFreeEx(this->hProc, data.lpTitle, 0, MEM_RELEASE);
			}
			//VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			//	MEM_DECOMMIT | MEM_RELEASE);
			VirtualFreeEx(this->hProc, data.iExitCode, 0, MEM_RELEASE);

			if (GetLastError() == ERROR_ACCESS_DENIED)
				this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR |
					PIE_ERROR_ACCESSERROR, "Cannot allocate on the process. "
					"Error: ERROR_ACCESS_DENIED");
			else
				this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR,
					"Cannot allocate on the process.");
			return 0;
		}
		WriteProcessMemory(this->hProc, data.lpMessage, sMessage.c_str(),
			sCaption.length(), NULL);
	}
	//Allocate space for thread call
	Memory lpThread = VirtualAllocEx(this->hProc, 0, sizeof(typeData),
		MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (lpThread == NULL)
	{
		this->SetLastError(PIE_ERROR_THREAD_ERROR | PIE_ALLOCATOR_PROCESS_ERROR,
			"Impossible create space for the thread");
		//VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
		//	MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.iExitCode, 0, MEM_RELEASE);
		if (bCaption)
			VirtualFreeEx(this->hProc, data.lpTitle, 0, MEM_RELEASE);
		if (bMessage)
			VirtualFreeEx(this->hProc, data.lpMessage, 0, MEM_RELEASE);
		return 0;
	}
	WriteProcessMemory(this->hProc, lpThread,
		(Memory)RemoteMessageBoxThreadA, sizeof(typeData), NULL);
	//Allocate space for thread params
	Memory pParam = VirtualAllocEx(this->hProc, NULL, sizeof(typeData),
		MEM_COMMIT, PAGE_READWRITE);
	if (pParam == NULL)
	{
		if (GetLastError() == ERROR_ACCESS_DENIED)
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR |
				PIE_ERROR_ACCESSERROR, "Cannot allocate on the process. "
				"Error: ERROR_ACCESS_DENIED");
		else
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR,
				"Cannot allocate on the process.");
		/*VirtualFreeEx(this->hProc, lpThread, sizeof(typeData),
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);*/
		VirtualFreeEx(this->hProc, lpThread, 0, MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.iExitCode, 0, MEM_RELEASE);
		if (bCaption)
			VirtualFreeEx(this->hProc, data.lpTitle, 0, MEM_RELEASE);
		if (bMessage)
			VirtualFreeEx(this->hProc, data.lpMessage, 0, MEM_RELEASE);
		return 0;
	}
	WriteProcessMemory(this->hProc, pParam, &data, sizeof(typeData), NULL);
	//Create the thread
	Handle hNewThread = CreateRemoteThread(this->hProc, NULL, NULL,
		(LPTHREAD_START_ROUTINE)lpThread, pParam, NULL, NULL);
	if (!HandleGood(hNewThread))
	{
		this->SetLastError(PIE_ERROR_THREAD_ERROR, "Cannot create the thread.");
		VirtualFreeEx(this->hProc, lpThread, 0, MEM_RELEASE);
		VirtualFreeEx(this->hProc, pParam, 0, MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.iExitCode, 0, MEM_RELEASE);
		if (bCaption)
			VirtualFreeEx(this->hProc, data.lpTitle, 0, MEM_RELEASE);
		if (bMessage)
			VirtualFreeEx(this->hProc, data.lpMessage, 0, MEM_RELEASE);
		return 0;
	}
	//Wait for MessageBoxA to end
	WaitForSingleObject(hNewThread, INFINITE);
	CloseHandle(hNewThread);
	//Free the allocated space
	VirtualFreeEx(this->hProc, pParam, 0, MEM_RELEASE);
	VirtualFreeEx(this->hProc, lpThread, 0, MEM_RELEASE);

	if (bCaption)
		VirtualFreeEx(this->hProc, data.lpTitle, 0, MEM_RELEASE);
	if (bMessage)
		VirtualFreeEx(this->hProc, data.lpMessage, 0, MEM_RELEASE);

	//Get the return value
	Int32 exit_ = 0;
	ReadProcessMemory(this->hProc, data.iExitCode, &exit_, sizeof(Int32), NULL);
	VirtualFreeEx(this->hProc, data.iExitCode, 0, MEM_RELEASE);
	return exit_;//Everything is done!
}
Int32 App::InjectMessageBox(Hwnd hWnd, const SuperUnicodeString& sCaption,
	const SuperUnicodeString& sMessage, UInt dwType)
{
	if (!this->Good()) return 0;
	using typeData = __MessageBoxWData;
	typeData data;
	//Setup the parameters
	memset(&data, 0, sizeof(typeData));
	//Get address for MessageBoxW
	HMODULE hUser32 = LoadLibraryW(L"user32.dll");
	if (!HandleGood(hUser32))
	{
		this->SetLastError(PIE_ERROR_HANDLE_ERROR,
			"Impossible to load user32.dll");
		return 0;
	}
	data.dwAddr = (Dword)GetProcAddress(hUser32, "MessageBoxW");
	FreeLibrary(hUser32);
	Bool bCaption = sCaption.Good();
	Bool bMessage = sMessage.Good();
	//Allocate space for exit code of MessageBoxW (int)
	data.iExitCode = (Int32*)VirtualAllocEx(this->hProc, NULL, sizeof(Int32),
		MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (data.iExitCode == PIE_NULL(Int32))
	{
		if (GetLastError() == ERROR_ACCESS_DENIED)
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR |
				PIE_ERROR_ACCESSERROR, "Cannot allocate on the process. "
				"Error: ERROR_ACCESS_DENIED");
		else
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR,
				"Cannot allocate on the process.");
		return 0;
	}
	//Allocate space for parameters
	if (bCaption)
	{
		data.lpTitle = (wchar_t*)VirtualAllocEx(this->hProc, NULL, sCaption.length() * 2,
			MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (data.lpTitle == PIE_NULL(wchar_t))
		{
			VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
				MEM_DECOMMIT | MEM_RELEASE);
			if (GetLastError() == ERROR_ACCESS_DENIED)
				this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR |
					PIE_ERROR_ACCESSERROR, "Cannot allocate on the process. "
					"Error: ERROR_ACCESS_DENIED");
			else
				this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR,
					"Cannot allocate on the process.");
			return 0;
		}
		WriteProcessMemory(this->hProc, data.lpTitle, sCaption.c_str(),
			sCaption.length() * 2, NULL);
	}
	if (bMessage)
	{
		data.lpMessage = (wchar_t*)VirtualAllocEx(this->hProc, NULL, sMessage.length() * 2,
			MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (data.lpMessage == PIE_NULL(wchar_t))
		{
			if (bCaption)
			{
				VirtualFreeEx(this->hProc, data.lpTitle, 2 * sCaption.length(),
					MEM_RELEASE | MEM_DECOMMIT);
			}
			VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
				MEM_DECOMMIT | MEM_RELEASE);
			if (GetLastError() == ERROR_ACCESS_DENIED)
				this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR |
					PIE_ERROR_ACCESSERROR, "Cannot allocate on the process. "
					"Error: ERROR_ACCESS_DENIED");
			else
				this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR,
					"Cannot allocate on the process.");
			return 0;
		}
		WriteProcessMemory(this->hProc, data.lpMessage, sMessage.c_str(),
			2 * sCaption.length(), NULL);
	}
	//Allocate space for thread call
	Memory lpThread = VirtualAllocEx(this->hProc, 0, sizeof(typeData),
		MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (lpThread == NULL)
	{
		this->SetLastError(PIE_ERROR_THREAD_ERROR | PIE_ALLOCATOR_PROCESS_ERROR,
			"Impossible create space for the thread");
		VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		if (bCaption)
			VirtualFreeEx(this->hProc, data.lpTitle, 2 * sCaption.length(),
				MEM_RELEASE | MEM_DECOMMIT);
		if (bMessage)
			VirtualFreeEx(this->hProc, data.lpMessage, 2 * sMessage.length(),
				MEM_RELEASE | MEM_DECOMMIT);
		return 0;
	}
	WriteProcessMemory(this->hProc, lpThread,
		(Memory)RemoteMessageBoxThreadW, sizeof(typeData), NULL);
	//Allocate space for thread params
	Memory pParam = VirtualAllocEx(this->hProc, NULL, sizeof(typeData),
		MEM_COMMIT, PAGE_READWRITE);
	if (pParam == NULL)
	{
		if (GetLastError() == ERROR_ACCESS_DENIED)
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR |
				PIE_ERROR_ACCESSERROR, "Cannot allocate on the process. "
				"Error: ERROR_ACCESS_DENIED");
		else
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR,
				"Cannot allocate on the process.");
		VirtualFreeEx(this->hProc, lpThread, sizeof(typeData),
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		if (bCaption)
			VirtualFreeEx(this->hProc, data.lpTitle, 2 * sCaption.length(),
				MEM_RELEASE | MEM_DECOMMIT);
		if (bMessage)
			VirtualFreeEx(this->hProc, data.lpMessage, 2 * sMessage.length(),
				MEM_RELEASE | MEM_DECOMMIT);
		return 0;
	}
	WriteProcessMemory(this->hProc, pParam, &data, sizeof(typeData), NULL);
	//Create the thread
	Handle hNewThread = CreateRemoteThread(this->hProc, NULL, NULL,
		(LPTHREAD_START_ROUTINE)lpThread, pParam, NULL, NULL);
	if (!HandleGood(hNewThread))
	{
		this->SetLastError(PIE_ERROR_THREAD_ERROR, "Cannot create the thread.");
		VirtualFreeEx(this->hProc, lpThread, sizeof(typeData),
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(this->hProc, pParam, sizeof(typeData),
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		if (bCaption)
			VirtualFreeEx(this->hProc, data.lpTitle, 2 * sCaption.length(),
				MEM_RELEASE | MEM_DECOMMIT);
		if (bMessage)
			VirtualFreeEx(this->hProc, data.lpMessage, 2 * sMessage.length(),
				MEM_RELEASE | MEM_DECOMMIT);
		return 0;
	}
	//Wait for MessageBoxA to end
	WaitForSingleObject(hNewThread, INFINITE);
	CloseHandle(hNewThread);
	//Free the allocated space
	VirtualFreeEx(this->hProc, pParam, sizeof(typeData),
		MEM_RELEASE | MEM_DECOMMIT);
	VirtualFreeEx(this->hProc, lpThread, sizeof(typeData),
		MEM_RELEASE | MEM_DECOMMIT);

	if (bCaption)
		VirtualFreeEx(this->hProc, data.lpTitle, 2 * sCaption.length(),
			MEM_RELEASE | MEM_DECOMMIT);
	if (bMessage)
		VirtualFreeEx(this->hProc, data.lpMessage, 2 * sMessage.length(),
			MEM_RELEASE | MEM_DECOMMIT);

	//Get the return value
	Int32 exit_ = 0;
	ReadProcessMemory(this->hProc, data.iExitCode, &exit_, sizeof(Int32), NULL);
	VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
		MEM_DECOMMIT | MEM_RELEASE);
	return exit_;//Everything is done!
}
//////////////////////////////////////////////////////////
#pragma warning(disable: 6388)
Bool App::FindProcAddressInLibrary(LPCSTR sFunc, LPCSTR sLib, Memory pVal)
{
	if (!this->Good())
		return false;
	size_t contains = this->ListLoadedLibraries.size;
	for (size_t i = 0; i < contains; i++)
	{
		if (this->ListLoadedLibraries[i].sPath.compare(sLib))
			contains = i;
	}
	if (contains < this->ListLoadedLibraries.size)
	{
		HMODULE hModule = this->ListLoadedLibraries[contains].hModule;
		if (!HandleGood(hModule)) {
			this->SetLastError(PIE_ERROR_HANDLE_ERROR,
				SuperAnsiString("Impossible to get the HMODULE of ") + sLib);
			return false;
		}
		*((FARPROC*)pVal) = GetProcAddress(hModule, sFunc);
		if (*((FARPROC*)pVal) == NULL)
		{
			this->SetLastError(PIE_ERROR_MEMORY_ERROR | PIE_ERROR_HANDLE_ERROR,
				SuperAnsiString("Impossible to find the address of ") + sFunc +
				" in " + sLib);
			return false;
		}
		return true;
	}
	HMODULE hLib = LoadLibraryExA(sLib, this->hProc,
		LOAD_LIBRARY_SEARCH_DEFAULT_DIRS |
		LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32);
	if (HandleGood(hLib))
	{
		this->ListLoadedLibraries.push_back({ sLib, hLib });
		*((FARPROC*)pVal) = GetProcAddress(hLib, sFunc);
		if (*((FARPROC*)pVal))
		{
			return true;
		}
		this->SetLastError(PIE_ERROR_MEMORY_ERROR | PIE_ERROR_HANDLE_ERROR,
			SuperAnsiString("Impossible to find the address of ") + sFunc +
			" in " + sLib);
		return false;
	}
	this->SetLastError(PIE_ERROR_HANDLE_ERROR, "Impossible to load the requested library");
	return false;
}
Bool App::InternalHook(const char* sFunc,
	const char* sLib, Memory mNewFunc, Memory lpHook)
{
	App::hook_t* pHook = static_cast<App::hook_t*>(lpHook);

	if (!this->FindProcAddressInLibrary(sFunc, sLib, &pHook->hook.pOldFunc))
	{
		return false;
	}
	pHook->hook.memJmp[0] = 0xe9Ui8;
	*(PULONG)&pHook->hook.memJmp[1] = (ULONG)mNewFunc - (ULONG)pHook->hook.pNewFunc - 5;
	memcpy(pHook->hook.prevJmp, pHook->hook.pNewFunc, 5U);
	pHook->hook.pOldFunc = VirtualAlloc(0, 4096U, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (pHook->hook.pOldFunc == NULL)
	{
		if (GetLastError() == ERROR_ACCESS_DENIED)
			this->SetLastError(PIE_ERROR_MEMORY_ERROR | PIE_ERROR_ACCESSERROR,
				"Impossible to allocate 4096 bytes of data, reason: ERROR_ACCESS_DENIED");
		else
			this->SetLastError(PIE_ERROR_MEMORY_ERROR,
				"Impossible to allocate 4096 bytes of data");
		return false;
	}
	memcpy(pHook->hook.pOldFunc, pHook->hook.prevJmp, 5U);
	Dword dwRealFun = (ULONG)pHook->hook.pOldFunc + 5;
	Dword dwNewFunc = (ULONG)pHook->hook.pNewFunc + 5;
	*(LPBYTE)((LPBYTE)dwRealFun) = 0xe9Ui8;
	*(PULONG)((LPBYTE)(dwRealFun + 1U)) = (ULONG)dwNewFunc;
	return this->InsertHookProcedure(lpHook);
}

Bool App::InsertHookProcedure(Memory lpHook)
{
	App::hook_t* pHook = static_cast<App::hook_t*>(lpHook);
	Dword dwOp;
	if (!VirtualProtect(pHook->hook.pNewFunc, 5, PAGE_EXECUTE_READWRITE, &dwOp))
	{
		if (GetLastError() == ERROR_ACCESS_DENIED)
		{
			this->SetLastError(PIE_ERROR_ACCESSERROR, "Impossible to access memory to restore hook");
			return false;
		}
		this->SetLastError(PIE_ERROR_MEMORY_ERROR, "Impossible to access memory to restore hook");
		return false;
	}
	memcpy(pHook->hook.pNewFunc, pHook->hook.memJmp, 5);
	if (!VirtualProtect(pHook->hook.pNewFunc, 5, dwOp, &dwOp))
	{
		if (GetLastError() == ERROR_ACCESS_DENIED)
		{
			this->SetLastError(PIE_ERROR_ACCESSERROR, "Impossible to access memory to restore hook");
			return false;
		}
		this->SetLastError(PIE_ERROR_MEMORY_ERROR, "Impossible to access memory to restore hook");
		return false;
	}
	return true;
}
Bool App::RestoreHook(Memory lpHook)
{
	App::hook_t* pHook = static_cast<App::hook_t*>(lpHook);
	Dword dwOp = 0;
	if (!VirtualProtect(pHook->hook.pNewFunc,
		5, PAGE_EXECUTE_READWRITE, &dwOp))
	{
		if (GetLastError() == ERROR_ACCESS_DENIED)
		{
			this->SetLastError(PIE_ERROR_ACCESSERROR, "Impossible to access memory to restore hook");
			return false;
		}
		this->SetLastError(PIE_ERROR_MEMORY_ERROR, "Impossible to access memory to restore hook");
		return false;
	}
	memcpy(pHook->hook.pNewFunc, pHook->hook.prevJmp, 5);
	return FreeHook(pHook->hook);
}
Bool App::HookDeleteFileA(Reserved::__DeleteFileAProc foo)
{
	if (this->_DeleteFileA.Active || (!this->Good()))
	{
		this->SetLastError(PIE_ERROR_INVALID_CALL,
			"Must initialize class first!");
		return false;
	}
	if (foo == NULL)
	{
		this->SetLastError(PIE_ERROR_INVALID_CALL | PIE_ERROR_MEMORY_ERROR,
			"Must initialize class first!");
		return false;
	}
	if (this->InternalHook("DeleteFileA",
		"kernel32.dll", foo, &this->_DeleteFileA.hook))
	{
		this->_DeleteFileA.Active = true;
		return true;
	}
	return false;
}
Bool App::HookDeleteFileW(Reserved::__DeleteFileWProc foo)
{
	if (this->_DeleteFileW.Active || (!this->Good()))
	{
		this->SetLastError(PIE_ERROR_INVALID_CALL,
			"Must initialize class first!");
		return false;
	}
	if (foo == NULL)
	{
		this->SetLastError(PIE_ERROR_INVALID_CALL | PIE_ERROR_MEMORY_ERROR,
			"Must initialize class first!");
		return false;
	}
	if (this->InternalHook("DeleteFileW",
		"kernel32.dll", foo, &this->_DeleteFileW.hook))
	{
		this->_DeleteFileW.Active = true;
		return true;
	}
	return false;
}
Bool App::UnHookDeleteFileA()
{
	if ((!this->_DeleteFileA.Active) || (!this->Good()))
	{
		this->SetLastError(PIE_ERROR_INVALID_CALL,
			"Must initialize class first!");
		return false;
	}
	if (this->RestoreHook(&this->_DeleteFileA.hook))
	{
		this->_DeleteFileA.Active = false;
		return true;
	}
	return false;
}
Bool App::UnHookDeleteFileW()
{
	if ((!this->_DeleteFileW.Active) || (!this->Good()))
	{
		this->SetLastError(PIE_ERROR_INVALID_CALL,
			"Must initialize class first!");
		return false;
	}
	if (this->RestoreHook(&this->_DeleteFileW.hook))
	{
		this->_DeleteFileW.Active = false;
		return true;
	}
	return false;
}


Procedure App::InitHook()
{
	memset(&this->_DeleteFileA, 0, sizeof(App::hook_t));
	memset(&this->_DeleteFileW, 0, sizeof(App::hook_t));
	this->_DeleteFileW.Active = this->_DeleteFileA.Active = false;
}

Procedure App::ListLibraries()
{
	this->ListLoadedLibraries.clear();
	if (this->Good())
	{
		HMODULE aModule[512];
		Dword dwCbNeeded;
		if (K32EnumProcessModules(this->hProc, aModule, sizeof(HMODULE) * 512, &dwCbNeeded))
		{
			for (size_t i = 0; i < (dwCbNeeded / sizeof(HMODULE)); i++)
			{
				char sModName[MAX_PATH];
				memset(sModName, 0, MAX_PATH);
				if (K32GetModuleFileNameExA(this->hProc, aModule[i], sModName, MAX_PATH))
				{
					this->ListLoadedLibraries.push_back({ sModName, aModule[i] });
				}
			}
		}
	}
}



BOOL App::InjectVoidFunction(const char* sName, const char* sLocation, Memory pFunc)
{
	if (!this->Good())
	{
		this->SetLastError(PIE_ERROR_INVALID_CALL,
			"Must select a process first!");
		return 0;
	}
	BOOLStruct data;
	memset(&data, 0, sizeof(BOOLStruct));
	//Get address for target function
	HMODULE hLoadedLib = LoadLibraryA(sLocation);
	if (!HandleGood(hLoadedLib))
	{
		this->SetLastError(PIE_ERROR_HANDLE_ERROR,
			"Impossible to load" + SuperAnsiString(sLocation));
		return 0;
	}
	data.dwAddr = (Dword)GetProcAddress(hLoadedLib, sName);
	FreeLibrary(hLoadedLib);
	//Allocate space for exit code of MessageBoxA (int)
	data.iExitCode = (Int32*)VirtualAllocEx(this->hProc, NULL, sizeof(Int32),
		MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (data.iExitCode == PIE_NULL(Int32))
	{
		if (GetLastError() == ERROR_ACCESS_DENIED)
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR |
				PIE_ERROR_ACCESSERROR, "Cannot allocate on the process. "
				"Error: ERROR_ACCESS_DENIED");
		else
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR,
				"Cannot allocate on the process.");
		return 0;
	}
	//Allocate space for thread call
	Memory lpThread = VirtualAllocEx(this->hProc, 0, sizeof(BOOLStruct),
		MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (lpThread == NULL)
	{
		this->SetLastError(PIE_ERROR_THREAD_ERROR | PIE_ALLOCATOR_PROCESS_ERROR,
			"Impossible create space for the thread");
		VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		return 0;
	}
	WriteProcessMemory(this->hProc, lpThread,
		pFunc, sizeof(BOOLStruct), NULL);
	//Allocate space for thread params
	Memory pParam = VirtualAllocEx(this->hProc, NULL, sizeof(BOOLStruct),
		MEM_COMMIT, PAGE_READWRITE);
	if (pParam == NULL)
	{
		if (GetLastError() == ERROR_ACCESS_DENIED)
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR |
				PIE_ERROR_ACCESSERROR, "Cannot allocate on the process. "
				"Error: ERROR_ACCESS_DENIED");
		else
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR,
				"Cannot allocate on the process.");
		VirtualFreeEx(this->hProc, lpThread, sizeof(BOOLStruct),
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		return 0;
	}
	WriteProcessMemory(this->hProc, pParam, &data, sizeof(BOOLStruct), NULL);
	//Create the thread
	Handle hNewThread = CreateRemoteThread(this->hProc, NULL, NULL,
		(LPTHREAD_START_ROUTINE)lpThread, pParam, NULL, NULL);
	if (!HandleGood(hNewThread))
	{
		this->SetLastError(PIE_ERROR_THREAD_ERROR, "Cannot create the thread.");
		VirtualFreeEx(this->hProc, lpThread, sizeof(BOOLStruct),
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(this->hProc, pParam, sizeof(BOOLStruct),
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		return 0;
	}
	//Wait for MessageBoxA to end
	WaitForSingleObject(hNewThread, INFINITE);
	CloseHandle(hNewThread);
	//Free the allocated space
	VirtualFreeEx(this->hProc, pParam, sizeof(BOOLStruct),
		MEM_RELEASE | MEM_DECOMMIT);
	VirtualFreeEx(this->hProc, lpThread, sizeof(BOOLStruct),
		MEM_RELEASE | MEM_DECOMMIT);

	//Get the return value
	BOOL bExit = 0;
	ReadProcessMemory(this->hProc, data.iExitCode, &bExit, sizeof(Int32), NULL);
	VirtualFreeEx(this->hProc, data.iExitCode, sizeof(BOOL),
		MEM_DECOMMIT | MEM_RELEASE);
	return bExit;//Everything is done!
}
BOOL App::InjectStringFunctionA(const char* sName, const char* sLocation,
	Memory pFunc, const char* sParam)
{
	if (!this->Good())
	{
		this->SetLastError(PIE_ERROR_INVALID_CALL,
			"Must initialize class first!");
		return 0;
	}
	using typeData = LPCSTRStruct;
	typeData data;
	//Setup the parameters
	memset(&data, 0, sizeof(typeData));
	HMODULE hLoadedLib = LoadLibraryA(sLocation);
	if (!HandleGood(hLoadedLib))
	{
		this->SetLastError(PIE_ERROR_HANDLE_ERROR,
			"Impossible to load " + SuperAnsiString(sLocation));
		return 0;
	}
	data.dwAddr = (Dword)GetProcAddress(hLoadedLib, sName);
	FreeLibrary(hLoadedLib);
	data.iExitCode = (Int32*)VirtualAllocEx(this->hProc, NULL, sizeof(Int32),
		MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (data.iExitCode == PIE_NULL(Int32))
	{
		if (GetLastError() == ERROR_ACCESS_DENIED)
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR |
				PIE_ERROR_ACCESSERROR, "Cannot allocate on the process. "
				"Error: ERROR_ACCESS_DENIED");
		else
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR,
				"Cannot allocate on the process.");
		return 0;
	}
	size_t sLength = strlen(sParam) + 1;
	data.pString = (char*)VirtualAllocEx(this->hProc, NULL, sLength,
		MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (data.pString == PIE_NULL(char))
	{
		VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		if (GetLastError() == ERROR_ACCESS_DENIED)
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR |
				PIE_ERROR_ACCESSERROR, "Cannot allocate on the process. "
				"Error: ERROR_ACCESS_DENIED");
		else
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR,
				"Cannot allocate on the process.");
		return 0;
	}
	WriteProcessMemory(this->hProc, data.pString, sParam,
		sLength, NULL);

	//Allocate space for thread call
	Memory lpThread = VirtualAllocEx(this->hProc, 0, sizeof(typeData),
		MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (lpThread == NULL)
	{
		this->SetLastError(PIE_ERROR_THREAD_ERROR | PIE_ALLOCATOR_PROCESS_ERROR,
			"Impossible create space for the thread");
		VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.pString, sLength,
			MEM_RELEASE | MEM_DECOMMIT);
		return 0;
	}
	WriteProcessMemory(this->hProc, lpThread,
		pFunc, sizeof(typeData), NULL);
	//Allocate space for thread params
	Memory pParam = VirtualAllocEx(this->hProc, NULL, sizeof(typeData),
		MEM_COMMIT, PAGE_READWRITE);
	if (pParam == NULL)
	{
		if (GetLastError() == ERROR_ACCESS_DENIED)
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR |
				PIE_ERROR_ACCESSERROR, "Cannot allocate on the process. "
				"Error: ERROR_ACCESS_DENIED");
		else
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR,
				"Cannot allocate on the process.");
		VirtualFreeEx(this->hProc, lpThread, sizeof(typeData),
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.pString, sLength,
			MEM_RELEASE | MEM_DECOMMIT);
		return 0;
	}
	WriteProcessMemory(this->hProc, pParam, &data, sizeof(typeData), NULL);
	//Create the thread
	Handle hNewThread = CreateRemoteThread(this->hProc, NULL, NULL,
		(LPTHREAD_START_ROUTINE)lpThread, pParam, NULL, NULL);
	if (!HandleGood(hNewThread))
	{
		this->SetLastError(PIE_ERROR_THREAD_ERROR, "Cannot create the thread.");
		VirtualFreeEx(this->hProc, lpThread, sizeof(typeData),
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(this->hProc, pParam, sizeof(typeData),
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.pString, sLength,
			MEM_RELEASE | MEM_DECOMMIT);
		return 0;
	}
	//Wait for pFunc
	WaitForSingleObject(hNewThread, INFINITE);
	CloseHandle(hNewThread);
	//Free the allocated space
	VirtualFreeEx(this->hProc, pParam, sizeof(typeData),
		MEM_RELEASE | MEM_DECOMMIT);
	VirtualFreeEx(this->hProc, lpThread, sizeof(typeData),
		MEM_RELEASE | MEM_DECOMMIT);

	VirtualFreeEx(this->hProc, data.pString, sLength,
		MEM_RELEASE | MEM_DECOMMIT);

	//Get the return value
	Int32 iExitCode = 0;
	ReadProcessMemory(this->hProc, data.iExitCode, &iExitCode, sizeof(Int32), NULL);
	VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
		MEM_DECOMMIT | MEM_RELEASE);
	return iExitCode;//Everything is done!
}
BOOL App::InjectStringFunctionW(const char* sName, const char* sLocation,
	Memory pFunc, const wchar_t* sParam)
{
	if (!this->Good())
	{
		this->SetLastError(PIE_ERROR_INVALID_CALL,
			"Must initialize class first!");
		return 0;
	}
	using typeData = LPCWSTRStruct;
	typeData data;
	//Setup the parameters
	memset(&data, 0, sizeof(typeData));
	HMODULE hLoadedLib = LoadLibraryA(sLocation);
	if (!HandleGood(hLoadedLib))
	{
		this->SetLastError(PIE_ERROR_HANDLE_ERROR,
			"Impossible to load " + SuperAnsiString(sLocation));
		return 0;
	}
	data.dwAddr = (Dword)GetProcAddress(hLoadedLib, sName);
	FreeLibrary(hLoadedLib);
	data.iExitCode = (Int32*)VirtualAllocEx(this->hProc, NULL, sizeof(Int32),
		MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (data.iExitCode == PIE_NULL(Int32))
	{
		if (GetLastError() == ERROR_ACCESS_DENIED)
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR |
				PIE_ERROR_ACCESSERROR, "Cannot allocate on the process. "
				"Error: ERROR_ACCESS_DENIED");
		else
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR,
				"Cannot allocate on the process.");
		return 0;
	}
	size_t sLength = wcslen(sParam) * 2 + 2;
	data.pString = (wchar_t*)VirtualAllocEx(this->hProc, NULL, sLength,
		MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (data.pString == PIE_NULL(wchar_t))
	{
		VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		if (GetLastError() == ERROR_ACCESS_DENIED)
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR |
				PIE_ERROR_ACCESSERROR, "Cannot allocate on the process. "
				"Error: ERROR_ACCESS_DENIED");
		else
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR,
				"Cannot allocate on the process.");
		return 0;
	}
	WriteProcessMemory(this->hProc, data.pString, sParam,
		sLength, NULL);

	//Allocate space for thread call
	Memory lpThread = VirtualAllocEx(this->hProc, 0, sizeof(typeData),
		MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (lpThread == NULL)
	{
		this->SetLastError(PIE_ERROR_THREAD_ERROR | PIE_ALLOCATOR_PROCESS_ERROR,
			"Impossible create space for the thread");
		VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.pString, sLength,
			MEM_RELEASE | MEM_DECOMMIT);
		return 0;
	}
	WriteProcessMemory(this->hProc, lpThread,
		pFunc, sizeof(typeData), NULL);
	//Allocate space for thread params
	Memory pParam = VirtualAllocEx(this->hProc, NULL, sizeof(typeData),
		MEM_COMMIT, PAGE_READWRITE);
	if (pParam == NULL)
	{
		if (GetLastError() == ERROR_ACCESS_DENIED)
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR |
				PIE_ERROR_ACCESSERROR, "Cannot allocate on the process. "
				"Error: ERROR_ACCESS_DENIED");
		else
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR,
				"Cannot allocate on the process.");
		VirtualFreeEx(this->hProc, lpThread, sizeof(typeData),
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.pString, sLength,
			MEM_RELEASE | MEM_DECOMMIT);
		return 0;
	}
	WriteProcessMemory(this->hProc, pParam, &data, sizeof(typeData), NULL);
	//Create the thread
	Handle hNewThread = CreateRemoteThread(this->hProc, NULL, NULL,
		(LPTHREAD_START_ROUTINE)lpThread, pParam, NULL, NULL);
	if (!HandleGood(hNewThread))
	{
		this->SetLastError(PIE_ERROR_THREAD_ERROR, "Cannot create the thread.");
		VirtualFreeEx(this->hProc, lpThread, sizeof(typeData),
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(this->hProc, pParam, sizeof(typeData),
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.pString, sLength,
			MEM_RELEASE | MEM_DECOMMIT);
		return 0;
	}
	//Wait for pFunc
	WaitForSingleObject(hNewThread, INFINITE);
	CloseHandle(hNewThread);
	//Free the allocated space
	VirtualFreeEx(this->hProc, pParam, sizeof(typeData),
		MEM_RELEASE | MEM_DECOMMIT);
	VirtualFreeEx(this->hProc, lpThread, sizeof(typeData),
		MEM_RELEASE | MEM_DECOMMIT);

	VirtualFreeEx(this->hProc, data.pString, sLength,
		MEM_RELEASE | MEM_DECOMMIT);

	//Get the return value
	Int32 iExitCode = 0;
	ReadProcessMemory(this->hProc, data.iExitCode, &iExitCode, sizeof(Int32), NULL);
	VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
		MEM_DECOMMIT | MEM_RELEASE);
	return iExitCode;//Everything is done!
}
BOOL App::InjectDWORDFunction(const char* sName, const char* sLocation,
	Memory pFunc, Dword dwParam)
{
	if (!this->Good())
	{
		this->SetLastError(PIE_ERROR_INVALID_CALL,
			"Must initialize class first!");
		return 0;
	}
	using typeData = DWORDStruct;
	typeData data;
	//Setup the parameters
	memset(&data, 0, sizeof(typeData));
	HMODULE hLoadedLib = LoadLibraryA(sLocation);
	if (!HandleGood(hLoadedLib))
	{
		this->SetLastError(PIE_ERROR_HANDLE_ERROR,
			"Impossible to load " + SuperAnsiString(sLocation));
		return 0;
	}
	data.dwAddr = (Dword)GetProcAddress(hLoadedLib, sName);
	FreeLibrary(hLoadedLib);
	data.iExitCode = (Int32*)VirtualAllocEx(this->hProc, NULL, sizeof(Int32),
		MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (data.iExitCode == PIE_NULL(Int32))
	{
		if (GetLastError() == ERROR_ACCESS_DENIED)
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR |
				PIE_ERROR_ACCESSERROR, "Cannot allocate on the process. "
				"Error: ERROR_ACCESS_DENIED");
		else
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR,
				"Cannot allocate on the process.");
		return 0;
	}
	data.dwParam = (DWORD*)VirtualAllocEx(this->hProc, NULL, sizeof(DWORD),
		MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (data.dwParam == PIE_NULL(DWORD))
	{
		VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		if (GetLastError() == ERROR_ACCESS_DENIED)
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR |
				PIE_ERROR_ACCESSERROR, "Cannot allocate on the process. "
				"Error: ERROR_ACCESS_DENIED");
		else
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR,
				"Cannot allocate on the process.");
		return 0;
	}
	Dword dwCopy = dwParam;
	WriteProcessMemory(this->hProc, data.dwParam, &dwCopy,
		sizeof(DWORD), NULL);

	//Allocate space for thread call
	Memory lpThread = VirtualAllocEx(this->hProc, 0, sizeof(typeData),
		MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (lpThread == NULL)
	{
		this->SetLastError(PIE_ERROR_THREAD_ERROR | PIE_ALLOCATOR_PROCESS_ERROR,
			"Impossible create space for the thread");
		VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.dwParam, sizeof(DWORD),
			MEM_RELEASE | MEM_DECOMMIT);
		return 0;
	}
	WriteProcessMemory(this->hProc, lpThread,
		pFunc, sizeof(typeData), NULL);
	//Allocate space for thread params
	Memory pParam = VirtualAllocEx(this->hProc, NULL, sizeof(typeData),
		MEM_COMMIT, PAGE_READWRITE);
	if (pParam == NULL)
	{
		if (GetLastError() == ERROR_ACCESS_DENIED)
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR |
				PIE_ERROR_ACCESSERROR, "Cannot allocate on the process. "
				"Error: ERROR_ACCESS_DENIED");
		else
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR,
				"Cannot allocate on the process.");
		VirtualFreeEx(this->hProc, lpThread, sizeof(typeData),
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.dwParam, sizeof(DWORD),
			MEM_RELEASE | MEM_DECOMMIT);
		return 0;
	}
	WriteProcessMemory(this->hProc, pParam, &data, sizeof(typeData), NULL);
	//Create the thread
	Handle hNewThread = CreateRemoteThread(this->hProc, NULL, NULL,
		(LPTHREAD_START_ROUTINE)lpThread, pParam, NULL, NULL);
	if (!HandleGood(hNewThread))
	{
		this->SetLastError(PIE_ERROR_THREAD_ERROR, "Cannot create the thread.");
		VirtualFreeEx(this->hProc, lpThread, sizeof(typeData),
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(this->hProc, pParam, sizeof(typeData),
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.dwParam, sizeof(DWORD),
			MEM_RELEASE | MEM_DECOMMIT);
		return 0;
	}
	//Wait for pFunc
	WaitForSingleObject(hNewThread, INFINITE);
	CloseHandle(hNewThread);
	//Free the allocated space
	VirtualFreeEx(this->hProc, pParam, sizeof(typeData),
		MEM_RELEASE | MEM_DECOMMIT);
	VirtualFreeEx(this->hProc, lpThread, sizeof(typeData),
		MEM_RELEASE | MEM_DECOMMIT);

	VirtualFreeEx(this->hProc, data.dwParam, sizeof(DWORD),
		MEM_RELEASE | MEM_DECOMMIT);

	//Get the return value
	Int32 iExitCode = 0;
	ReadProcessMemory(this->hProc, data.iExitCode, &iExitCode, sizeof(Int32), NULL);
	VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
		MEM_DECOMMIT | MEM_RELEASE);
	return iExitCode;//Everything is done!
}
BOOL App::InjectPrintFunctionA(const char* sName, const char* sLocation,
	Memory pFunc, Handle hStream, const SuperAnsiString& sParam,
	Dword* dwWritten)
{
	if (!this->Good())
	{
		this->SetLastError(PIE_ERROR_INVALID_CALL,
			"Must initialize class first!");
		return 0;
	}
	PrintStructA data;
	//Setup the parameters
	memset(&data, 0, sizeof(PrintStructA));
	HMODULE hLoadedLib = LoadLibraryA(sLocation);
	if (!HandleGood(hLoadedLib))
	{
		this->SetLastError(PIE_ERROR_HANDLE_ERROR,
			"Impossible to load " + SuperAnsiString(sLocation));
		return 0;
	}
	data.dwAddr = (Dword)GetProcAddress(hLoadedLib, sName);
	FreeLibrary(hLoadedLib);


	data.iExitCode = (Int32*)VirtualAllocEx(this->hProc, NULL, sizeof(Int32),
		MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (data.iExitCode == PIE_NULL(Int32))
	{
		if (GetLastError() == ERROR_ACCESS_DENIED)
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR |
				PIE_ERROR_ACCESSERROR, "Cannot allocate on the process. "
				"Error: ERROR_ACCESS_DENIED");
		else
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR,
				"Cannot allocate on the process.");
		return 0;
	}

	SuperAnsiString ssParam = sParam;
	ssParam.push_back('\0');
	data.sOut = (char*)VirtualAllocEx(this->hProc, NULL, ssParam.length(),
		MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (data.sOut == PIE_NULL(char))
	{
		VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		if (GetLastError() == ERROR_ACCESS_DENIED)
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR |
				PIE_ERROR_ACCESSERROR, "Cannot allocate on the process. "
				"Error: ERROR_ACCESS_DENIED");
		else
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR,
				"Cannot allocate on the process.");
		return 0;
	}
	WriteProcessMemory(this->hProc, data.sOut, ssParam.c_str(),
		ssParam.length(), NULL);

	data.dwWrite = (Dword*)VirtualAllocEx(this->hProc, NULL,
		sizeof(Dword), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (data.dwWrite == PIE_NULL(Dword))
	{
		if (GetLastError() == ERROR_ACCESS_DENIED)
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR |
				PIE_ERROR_ACCESSERROR, "Cannot allocate on the process. "
				"Error: ERROR_ACCESS_DENIED");
		else
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR,
				"Cannot allocate on the process.");
		VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.sOut, ssParam.length(),
			MEM_RELEASE | MEM_DECOMMIT);
		return 0;
	}
	Dword dwLength = ssParam.length();
	WriteProcessMemory(this->hProc, data.dwWrite,
		&dwLength, sizeof(Dword), NULL);

	Dword* lpWritten = (Dword*)VirtualAllocEx(this->hProc, NULL,
		sizeof(Dword), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (lpWritten == PIE_NULL(Dword))
	{
		if (GetLastError() == ERROR_ACCESS_DENIED)
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR |
				PIE_ERROR_ACCESSERROR, "Cannot allocate on the process. "
				"Error: ERROR_ACCESS_DENIED");
		else
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR,
				"Cannot allocate on the process.");
		VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.sOut, ssParam.length(),
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(this->hProc, data.dwWrite,
			sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
		return 0;
	}
	dwLength = (dwWritten == PIE_NULL(Dword)) ? 0 : *dwWritten;
	WriteProcessMemory(this->hProc, data.dwWrite,
		&dwLength, sizeof(Dword), NULL);

	data.dwWritten = (Dword**)VirtualAllocEx(this->hProc, NULL,
		sizeof(Dword*), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (data.dwWritten == PIE_NULL(Dword*))
	{
		if (GetLastError() == ERROR_ACCESS_DENIED)
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR |
				PIE_ERROR_ACCESSERROR, "Cannot allocate on the process. "
				"Error: ERROR_ACCESS_DENIED");
		else
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR,
				"Cannot allocate on the process.");
		VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.sOut, ssParam.length(),
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(this->hProc, data.dwWrite,
			sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, lpWritten,
			sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
		return 0;
	}
	WriteProcessMemory(this->hProc, data.dwWrite,
		&lpWritten, sizeof(Dword), NULL);

	//Allocate space for thread call
	Memory lpThread = VirtualAllocEx(this->hProc, 0, sizeof(PrintStructA),
		MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (lpThread == NULL)
	{
		this->SetLastError(PIE_ERROR_THREAD_ERROR | PIE_ALLOCATOR_PROCESS_ERROR,
			"Impossible create space for the thread");
		VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.sOut, ssParam.length(),
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.dwWrite,
			sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, lpWritten,
			sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.dwWritten,
			sizeof(Dword*), MEM_DECOMMIT | MEM_RELEASE);
		return 0;
	}
	WriteProcessMemory(this->hProc, lpThread,
		pFunc, sizeof(PrintStructA), NULL);
	//Allocate space for thread params
	Memory pParam = VirtualAllocEx(this->hProc, NULL, sizeof(PrintStructA),
		MEM_COMMIT, PAGE_READWRITE);
	if (pParam == NULL)
	{
		if (GetLastError() == ERROR_ACCESS_DENIED)
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR |
				PIE_ERROR_ACCESSERROR, "Cannot allocate on the process. "
				"Error: ERROR_ACCESS_DENIED");
		else
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR,
				"Cannot allocate on the process.");
		VirtualFreeEx(this->hProc, lpThread, sizeof(PrintStructA),
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.sOut, ssParam.length(),
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.dwWrite,
			sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, lpWritten,
			sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.dwWritten,
			sizeof(Dword*), MEM_DECOMMIT | MEM_RELEASE);
		return 0;
	}
	WriteProcessMemory(this->hProc, pParam, &data, sizeof(PrintStructA), NULL);
	//Create the thread
	Handle hNewThread = CreateRemoteThread(this->hProc, NULL, NULL,
		(LPTHREAD_START_ROUTINE)lpThread, pParam, NULL, NULL);
	if (!HandleGood(hNewThread))
	{
		this->SetLastError(PIE_ERROR_THREAD_ERROR, "Cannot create the thread.");
		VirtualFreeEx(this->hProc, lpThread, sizeof(PrintStructA),
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(this->hProc, pParam, sizeof(PrintStructA),
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.sOut, ssParam.length(),
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.dwWrite,
			sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, lpWritten,
			sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.dwWritten,
			sizeof(Dword*), MEM_DECOMMIT | MEM_RELEASE);
		return 0;
	}
	//Wait for pFunc
	WaitForSingleObject(hNewThread, INFINITE);
	CloseHandle(hNewThread);
	//Free the allocated space
	VirtualFreeEx(this->hProc, pParam, sizeof(PrintStructA),
		MEM_RELEASE | MEM_DECOMMIT);
	VirtualFreeEx(this->hProc, lpThread, sizeof(PrintStructA),
		MEM_RELEASE | MEM_DECOMMIT);

	//Get the return value
	Int32 iExitCode = 0;
	ReadProcessMemory(this->hProc, data.iExitCode, &iExitCode, sizeof(Int32), NULL);
	if (dwWritten)
	{
		ReadProcessMemory(this->hProc, lpWritten,
			dwWritten, sizeof(Dword*), NULL);
	}
	VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
		MEM_DECOMMIT | MEM_RELEASE);
	VirtualFreeEx(this->hProc, data.sOut, ssParam.length(),
		MEM_DECOMMIT | MEM_RELEASE);
	VirtualFreeEx(this->hProc, data.dwWrite,
		sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
	VirtualFreeEx(this->hProc, lpWritten,
		sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
	VirtualFreeEx(this->hProc, data.dwWritten,
		sizeof(Dword*), MEM_DECOMMIT | MEM_RELEASE);
	return iExitCode;//Everything is done!
}
BOOL App::InjectPrintFunctionW(const char* sName, const char* sLocation,
	Memory pFunc, Handle hStream, const SuperUnicodeString& sParam,
	Dword* dwWritten)
{
	if (!this->Good())
	{
		this->SetLastError(PIE_ERROR_INVALID_CALL,
			"Must initialize class first!");
		return 0;
	}
	PrintStructW data;
	//Setup the parameters
	memset(&data, 0, sizeof(PrintStructW));
	HMODULE hLoadedLib = LoadLibraryA(sLocation);
	if (!HandleGood(hLoadedLib))
	{
		this->SetLastError(PIE_ERROR_HANDLE_ERROR,
			"Impossible to load " + SuperAnsiString(sLocation));
		return 0;
	}
	data.dwAddr = (Dword)GetProcAddress(hLoadedLib, sName);
	FreeLibrary(hLoadedLib);


	data.iExitCode = (Int32*)VirtualAllocEx(this->hProc, NULL, sizeof(Int32),
		MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (data.iExitCode == PIE_NULL(Int32))
	{
		if (GetLastError() == ERROR_ACCESS_DENIED)
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR |
				PIE_ERROR_ACCESSERROR, "Cannot allocate on the process. "
				"Error: ERROR_ACCESS_DENIED");
		else
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR,
				"Cannot allocate on the process.");
		return 0;
	}

	SuperUnicodeString ssParam = sParam;
	ssParam.push_back(L'\0');
	data.sOut = (wchar_t*)VirtualAllocEx(this->hProc, NULL, ssParam.length() * 2,
		MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (data.sOut == PIE_NULL(wchar_t))
	{
		VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		if (GetLastError() == ERROR_ACCESS_DENIED)
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR |
				PIE_ERROR_ACCESSERROR, "Cannot allocate on the process. "
				"Error: ERROR_ACCESS_DENIED");
		else
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR,
				"Cannot allocate on the process.");
		return 0;
	}
	WriteProcessMemory(this->hProc, data.sOut, ssParam.c_str(),
		ssParam.length() * 2, NULL);

	data.dwWrite = (Dword*)VirtualAllocEx(this->hProc, NULL,
		sizeof(Dword), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (data.dwWrite == PIE_NULL(Dword))
	{
		if (GetLastError() == ERROR_ACCESS_DENIED)
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR |
				PIE_ERROR_ACCESSERROR, "Cannot allocate on the process. "
				"Error: ERROR_ACCESS_DENIED");
		else
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR,
				"Cannot allocate on the process.");
		VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.sOut, ssParam.length() * 2,
			MEM_RELEASE | MEM_DECOMMIT);
		return 0;
	}
	Dword dwLength = ssParam.length() * 2;
	WriteProcessMemory(this->hProc, data.dwWrite,
		&dwLength, sizeof(Dword), NULL);

	Dword* lpWritten = (Dword*)VirtualAllocEx(this->hProc, NULL,
		sizeof(Dword), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (lpWritten == PIE_NULL(Dword))
	{
		if (GetLastError() == ERROR_ACCESS_DENIED)
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR |
				PIE_ERROR_ACCESSERROR, "Cannot allocate on the process. "
				"Error: ERROR_ACCESS_DENIED");
		else
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR,
				"Cannot allocate on the process.");
		VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.sOut, ssParam.length() * 2,
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(this->hProc, data.dwWrite,
			sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
		return 0;
	}
	dwLength = (dwWritten == PIE_NULL(Dword)) ? 0 : *dwWritten;
	WriteProcessMemory(this->hProc, data.dwWrite,
		&dwLength, sizeof(Dword), NULL);

	data.dwWritten = (Dword**)VirtualAllocEx(this->hProc, NULL,
		sizeof(Dword*), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (data.dwWritten == PIE_NULL(Dword*))
	{
		if (GetLastError() == ERROR_ACCESS_DENIED)
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR |
				PIE_ERROR_ACCESSERROR, "Cannot allocate on the process. "
				"Error: ERROR_ACCESS_DENIED");
		else
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR,
				"Cannot allocate on the process.");
		VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.sOut, ssParam.length() * 2,
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(this->hProc, data.dwWrite,
			sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, lpWritten,
			sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
		return 0;
	}
	WriteProcessMemory(this->hProc, data.dwWrite,
		&lpWritten, sizeof(Dword), NULL);

	//Allocate space for thread call
	Memory lpThread = VirtualAllocEx(this->hProc, 0, sizeof(PrintStructW),
		MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (lpThread == NULL)
	{
		this->SetLastError(PIE_ERROR_THREAD_ERROR | PIE_ALLOCATOR_PROCESS_ERROR,
			"Impossible create space for the thread");
		VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.sOut, ssParam.length() * 2,
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.dwWrite,
			sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, lpWritten,
			sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.dwWritten,
			sizeof(Dword*), MEM_DECOMMIT | MEM_RELEASE);
		return 0;
	}
	WriteProcessMemory(this->hProc, lpThread,
		pFunc, sizeof(PrintStructA), NULL);
	//Allocate space for thread params
	Memory pParam = VirtualAllocEx(this->hProc, NULL, sizeof(PrintStructW),
		MEM_COMMIT, PAGE_READWRITE);
	if (pParam == NULL)
	{
		if (GetLastError() == ERROR_ACCESS_DENIED)
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR |
				PIE_ERROR_ACCESSERROR, "Cannot allocate on the process. "
				"Error: ERROR_ACCESS_DENIED");
		else
			this->SetLastError(PIE_ALLOCATOR_PROCESS_ERROR,
				"Cannot allocate on the process.");
		VirtualFreeEx(this->hProc, lpThread, sizeof(PrintStructW),
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.sOut, ssParam.length() * 2,
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.dwWrite,
			sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, lpWritten,
			sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.dwWritten,
			sizeof(Dword*), MEM_DECOMMIT | MEM_RELEASE);
		return 0;
	}
	WriteProcessMemory(this->hProc, pParam, &data, sizeof(PrintStructW), NULL);
	//Create the thread
	Handle hNewThread = CreateRemoteThread(this->hProc, NULL, NULL,
		(LPTHREAD_START_ROUTINE)lpThread, pParam, NULL, NULL);
	if (!HandleGood(hNewThread))
	{
		this->SetLastError(PIE_ERROR_THREAD_ERROR, "Cannot create the thread.");
		VirtualFreeEx(this->hProc, lpThread, sizeof(PrintStructW),
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(this->hProc, pParam, sizeof(PrintStructW),
			MEM_RELEASE | MEM_DECOMMIT);
		VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.sOut, ssParam.length() * 2,
			MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.dwWrite,
			sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, lpWritten,
			sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
		VirtualFreeEx(this->hProc, data.dwWritten,
			sizeof(Dword*), MEM_DECOMMIT | MEM_RELEASE);
		return 0;
	}
	//Wait for pFunc
	WaitForSingleObject(hNewThread, INFINITE);
	CloseHandle(hNewThread);
	//Free the allocated space
	VirtualFreeEx(this->hProc, pParam, sizeof(PrintStructW),
		MEM_RELEASE | MEM_DECOMMIT);
	VirtualFreeEx(this->hProc, lpThread, sizeof(PrintStructW),
		MEM_RELEASE | MEM_DECOMMIT);

	//Get the return value
	Int32 iExitCode = 0;
	ReadProcessMemory(this->hProc, data.iExitCode, &iExitCode,
		sizeof(Int32), NULL);
	if (dwWritten)
	{
		ReadProcessMemory(this->hProc, lpWritten,
			dwWritten, sizeof(Dword*), NULL);
	}
	VirtualFreeEx(this->hProc, data.iExitCode, sizeof(Int32),
		MEM_DECOMMIT | MEM_RELEASE);
	VirtualFreeEx(this->hProc, data.sOut, ssParam.length() * 2,
		MEM_DECOMMIT | MEM_RELEASE);
	VirtualFreeEx(this->hProc, data.dwWrite,
		sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
	VirtualFreeEx(this->hProc, lpWritten,
		sizeof(Dword), MEM_DECOMMIT | MEM_RELEASE);
	VirtualFreeEx(this->hProc, data.dwWritten,
		sizeof(Dword*), MEM_DECOMMIT | MEM_RELEASE);
	return iExitCode;
}