#include "pch.h"
#include "PE_App.h"
using namespace pe;
using namespace pe::Reserved;

Bool FreeHook(Reserved::Hook& hook)
{
	Bool bVal = static_cast<Bool>(VirtualFree(hook.pOldFunc, 0, MEM_RELEASE));
	memset(&hook, 0, sizeof(Reserved::Hook));
	return bVal;
}

void App::InitHook()
{
	memset(&_DeleteFileA, 0, sizeof(App::hook_t));
	memset(&_DeleteFileW, 0, sizeof(App::hook_t));
	_DeleteFileW.Active = _DeleteFileA.Active = false;
}
Bool App::InternalHook(
	cStringA sFunc,
	cStringA sLib,
	Memory mNewFunc,
	Memory lpHook)
{
	App::hook_t* pHook = static_cast<App::hook_t*>(lpHook);

	if (!FindProcAddressInLibrary(sFunc, sLib, &pHook->hook.pOldFunc))
	{
		return false;
	}
	pHook->hook.memJmp[0] = 0xe9Ui8;
	*(PULONG)&pHook->hook.memJmp[1] =
		(ULONG)mNewFunc - (ULONG)pHook->hook.pNewFunc - 5;
	memcpy(pHook->hook.prevJmp, pHook->hook.pNewFunc, 5U);
	pHook->hook.pOldFunc = VirtualAlloc(0, 4096U, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (pHook->hook.pOldFunc == NULL)
	{
		DWORD lastError = GetLastError();
		SetLastError(lastError,
			(lastError == ERROR_ACCESS_DENIED) ?
			"Impossible to allocate 4096 bytes of data, "
			"reason : ERROR_ACCESS_DENIED" :
			"Impossible to allocate 4096 bytes of data");
		return false;
	}
	memcpy(pHook->hook.pOldFunc, pHook->hook.prevJmp, 5U);
	Dword dwRealFun = (ULONG)pHook->hook.pOldFunc + 5;
	Dword dwNewFunc = (ULONG)pHook->hook.pNewFunc + 5;
	*(LPBYTE)((LPBYTE)dwRealFun) = 0xe9Ui8;
	*(PULONG)((LPBYTE)(dwRealFun + 1U)) = (ULONG)dwNewFunc;
	return InsertHookProcedure(lpHook);
}

Bool App::InsertHookProcedure(Memory lpHook)
{
	App::hook_t* pHook = static_cast<App::hook_t*>(lpHook);
	Dword dwOp;
	if (!VirtualProtect(pHook->hook.pNewFunc, 5, PAGE_EXECUTE_READWRITE, &dwOp))
	{
		DWORD lastError = GetLastError();
		SetLastError(lastError,
			(lastError == ERROR_ACCESS_DENIED) ?
			"Impossible to access memory to restore hook, "
			"reason : ERROR_ACCESS_DENIED" :
			"Impossible to access memory to restore hook");
		return false;
	}
	memcpy(pHook->hook.pNewFunc, pHook->hook.memJmp, 5);
	if (!VirtualProtect(pHook->hook.pNewFunc, 5, dwOp, &dwOp))
	{
		DWORD lastError = GetLastError();
		SetLastError(lastError,
			(lastError == ERROR_ACCESS_DENIED) ?
			"Impossible to access memory to restore hook, "
			"reason : ERROR_ACCESS_DENIED" :
			"Impossible to access memory to restore hook");
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
		DWORD lastError = GetLastError();
		SetLastError(lastError,
			(lastError == ERROR_ACCESS_DENIED) ?
			"Impossible to access memory to restore hook, "
			"reason : ERROR_ACCESS_DENIED" :
			"Impossible to access memory to restore hook");
		return false;
	}
	memcpy(pHook->hook.pNewFunc, pHook->hook.prevJmp, 5);
	return FreeHook(pHook->hook);
}
Bool App::HookDeleteFileA(Reserved::__DeleteFileAProc foo)
{
	if (_DeleteFileA.Active || (!Good()))
	{
		SetLastError(ERROR_INVALID_HANDLE,
			"Must initialize class first!");
		return false;
	}
	if (foo == NULL)
	{
		SetLastError(ERROR_INVALID_HANDLE,
			"Must initialize class first!");
		return false;
	}
	if (InternalHook("DeleteFileA",
		"kernel32.dll", foo, &_DeleteFileA.hook))
	{
		_DeleteFileA.Active = true;
		return true;
	}
	return false;
}
Bool App::HookDeleteFileW(Reserved::__DeleteFileWProc foo)
{
	if (_DeleteFileW.Active || (!Good()))
	{
		SetLastError(ERROR_INVALID_HANDLE,
			"Must initialize class first!");
		return false;
	}
	if (foo == NULL)
	{
		SetLastError(ERROR_INVALID_HANDLE,
			"Must initialize class first!");
		return false;
	}
	if (InternalHook("DeleteFileW",
		"kernel32.dll", foo, &_DeleteFileW.hook))
	{
		_DeleteFileW.Active = true;
		return true;
	}
	return false;
}
Bool App::UnHookDeleteFileA()
{
	if ((!_DeleteFileA.Active) || (!Good()))
	{
		SetLastError(ERROR_INVALID_HANDLE,
			"Must initialize class first!");
		return false;
	}
	if (RestoreHook(&_DeleteFileA.hook))
	{
		_DeleteFileA.Active = false;
		return true;
	}
	return false;
}
Bool App::UnHookDeleteFileW()
{
	if ((!_DeleteFileW.Active) || (!Good()))
	{
		SetLastError(ERROR_INVALID_HANDLE,
			"Must initialize class first!");
		return false;
	}
	if (RestoreHook(&_DeleteFileW.hook))
	{
		_DeleteFileW.Active = false;
		return true;
	}
	return false;
}