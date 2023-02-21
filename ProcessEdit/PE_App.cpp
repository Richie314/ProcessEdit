#include "pch.h"
#include "PE_App.h"
#include <Psapi.h>
#include <utility>
#include <vector>
using namespace pe;

Bool isValidProgramStr(cStringW& name)
{
	Int32 indexOfDot = str::LastIndexOfW(name, L'.');
	if (indexOfDot != EOF)
	{
		cStringW _end = str::SinceW(name, indexOfDot);
		return str::EqualsW(_end, L".exe") || str::EqualsW(_end, L".dll");
	}
	return false;
}

void GetAllWindowsFromProcessID(
	DWORD dwProcessID, List<Hwnd>& vhWnds)
{
	vhWnds.clear();
	Hwnd hCurWnd = NULL;
	do
	{
		hCurWnd = FindWindowExW(NULL, hCurWnd, NULL, NULL);
		Dword dwProcID = 0;
		GetWindowThreadProcessId(hCurWnd, &dwProcID);
		if (dwProcID == dwProcessID)
		{
			vhWnds.push(hCurWnd);
		}
	} while (hCurWnd != NULL);
}
void App::SetLastError(Uint error,
	 cStringA errorStr)
{
	this->__LastError = error;
	str::CopyA(&this->__LastErrorStr, errorStr);
}

App::App()
	:hProc(INVALID_HANDLE_VALUE),
	hToken(INVALID_HANDLE_VALUE),
	pId(0),
	__LastError(0),
	dispose(true)
{
	this->ListLibraries();
};
App::App(cStringW sProcess, Bool canWrite)
	:hProc(INVALID_HANDLE_VALUE),
	hToken(INVALID_HANDLE_VALUE),
	pId(0),
	__LastError(0),
	dispose(true)
{
	if (!isValidProgramStr(sProcess))
		return;
	StringW sProcessEdit;
	str::CopyW(&sProcessEdit, sProcess);
	str::ReplaceW(sProcessEdit, L'/', L'\\');
	if (str::ContainsW(sProcessEdit, L'\\'))
	{
		size_t index = str::LastIndexOfW(sProcessEdit, L'\\') + 1U;
		str::CopyW(&sLocation, sProcessEdit, index);
		str::CopyW(&sName, str::SinceW(sProcessEdit, index));
	}
	pId = FindProcId();
	OpenStreamProcess(canWrite);
	UpdateWindowCount();
	ListLibraries();
};
App::App(Dword PID, Bool canWrite)
	:hProc(INVALID_HANDLE_VALUE),
	hToken(INVALID_HANDLE_VALUE),
	pId(0),
	__LastError(0),
	dispose(true)
{
	OpenProcessFromID(PID, canWrite);
	UpdateWindowCount();
	ListLibraries();
};
App::App(const ProcessBasicInfo& pbiInfo, Bool canWrite)
	:hProc(INVALID_HANDLE_VALUE),
	hToken(INVALID_HANDLE_VALUE),
	pId(0),
	__LastError(0),
	dispose(true)
{
	OpenProcessFromID(pbiInfo.dwId, canWrite);
	UpdateWindowCount();
	ListLibraries();
};
App::App(const pProcessBasicInfo pPBI, Bool canWrite)
	:hProc(INVALID_HANDLE_VALUE),
	hToken(INVALID_HANDLE_VALUE),
	pId(0),
	__LastError(0),
	dispose(true)
{
	if (pPBI)
	{
		OpenProcessFromID(pPBI->dwId, canWrite);
		UpdateWindowCount();
	}
	SetLastError(0,
		"Trying to initialize with a NULL pointer: expected a \"ProcessBasicInfo\" pointer.");
	ListLibraries();
};
App::App(const App& otherApp)
	:hProc(INVALID_HANDLE_VALUE),
	hToken(INVALID_HANDLE_VALUE),
	pId(0),
	__LastError(0),
	dispose(true)
{
	fromOtherStream(otherApp);
}
App::App(App&& otherApp)
	:hProc(INVALID_HANDLE_VALUE),
	hToken(INVALID_HANDLE_VALUE),
	pId(0),
	__LastError(0),
	dispose(true)
{
	fromOtherStream(std::move(otherApp));
}
App::~App()
{
	Close();
}

void App::Close()
{
	if (dispose)
	{
		if (HandleGood(hProc))
			CloseHandle(hProc);
		if (HandleGood(hToken))
			CloseHandle(hToken);
		LoadedLibraries.clear();
	}
	hwnds.clear();
	str::FreeW(&sLocation, 0);
	str::FreeW(&sName, 0);
	str::FreeW(&sUser, 0);
	str::FreeA(&__LastErrorStr, 0);
	dispose = false;
	pId = 0;
}

Hwnd App::FindThisWindowFromContent(cStringW caption)const
{
	StringW s;
	size_t size = str::LenW(caption);
	if (!size || hwnds.isEmpty())
		return NULL;
	if (!str::AllocW(&s, size))
		return NULL;
	Hwnd curr = NULL;
	for (
		const List<Hwnd>* pHwnd = &hwnds;
		pHwnd != nullptr && !pHwnd->isEmpty();
		pHwnd = pHwnd->Next)
	{
		if (GetWindowTextW(*pHwnd->Value, s, size + 1U))
		{
			if (str::EqualsW(s, caption))
			{
				curr = *pHwnd->Value;
				break;
			}
		}
	}
	str::FreeW(&s, size);
	return curr;
}
Hwnd App::FindThisWindowFromContent(cStringA caption)const
{
	StringA s;
	size_t size = str::LenA(caption);
	if (!size)
		return NULL;
	if (!str::AllocA(&s, size))
		return NULL;
	Hwnd curr = NULL;
	for (
		const List<Hwnd>* pHwnd = &hwnds;
		pHwnd != nullptr && !pHwnd->isEmpty();
		pHwnd = pHwnd->Next)
	{
		if (GetWindowTextA(*pHwnd->Value, s, size + 1U))
		{
			if (str::EqualsA(s, caption))
			{
				curr = *pHwnd->Value;
				break;
			}
		}
	}
	str::FreeA(&s, size);
	return curr;
}

void App::ExtractTokenInfo()
{
	Dword dwProcessTokenInfoAllocSize = 0, error = 0;
	GetTokenInformation(hToken, TokenUser,
		NULL, 0, &dwProcessTokenInfoAllocSize);
	error = GetLastError();
	if (error == ERROR_ACCESS_DENIED)
	{
		SetLastError(ERROR_ACCESS_DENIED,
			"GetTokenInformation failed with error: ACCESS_DENIED");
		return;
	}
	if (error == ERROR_INSUFFICIENT_BUFFER)
	{
		PTOKEN_USER pUserToken = new TOKEN_USER;
		if (pUserToken == NULL)
		{
			SetLastError(ERROR_INSUFFICIENT_BUFFER,
				"Not enough space");
			return;
		}
		if (!GetTokenInformation(
			hToken,
			TokenUser, pUserToken,
			dwProcessTokenInfoAllocSize,
			&dwProcessTokenInfoAllocSize))
		{
			if (GetLastError() == ERROR_ACCESS_DENIED)
				SetLastError(ERROR_ACCESS_DENIED,
					"GetTokenInformation failed with error: ACCESS_DENIED");
			else
				SetLastError(ERROR_ACCESS_DENIED,
					"GetTokenInformation failed with unknown error");
			delete pUserToken;
			return;
		}
		SID_NAME_USE   snuSIDNameUse;

		wchar_t szUser[256] = { 0 };
		Dword dwUserNameLength = 256;

		wchar_t szDomain[256] = { 0 };
		Dword dwDomainNameLength = 256;
		if (LookupAccountSidW(NULL,
			pUserToken->User.Sid,
			szUser, &dwUserNameLength,
			szDomain, &dwDomainNameLength,
			&snuSIDNameUse))
		{
			str::ReAllocW(&sUser, 3 + str::Len(szDomain) + str::Len(szUser));
			str::CatW(sUser, L"\\\\");
			str::CatW(sUser, szDomain);
			str::CatW(sUser, L"\\\\");
			str::CatW(sUser, szUser);
		}
		delete pUserToken;
	}
}

void App::UpdateWindowCount()
{
	if (Good())
		GetAllWindowsFromProcessID(pId, hwnds);
}

Bool App::ReadMemory(
	const Memory addr, Uint bytesToRead,
	Memory* destination)const
{
	if (addr == NULL || destination == NULL ||bytesToRead == 0 || (!Good()))
		return false;
	if (*destination == NULL)
	{
		return false;
	}
	SIZE_T nbytesRead = 0;
	if (ReadProcessMemory(hProc, addr, destination, bytesToRead, &nbytesRead))
	{
		return true;
	}
	return false;
}

Bool PE_CALL App::WriteMemory(const Memory addr, const Memory buffer, size_t size)
{
	if ((!Good()) || addr == NULL || size == 0 || buffer == NULL)
		return false;
	SIZE_T nSize = size;
	if (WriteProcessMemory(hProc, addr, buffer,
		nSize, &nSize))
	{
		return true;
	}
	if (GetLastError() == ERROR_ACCESS_DENIED)
		SetLastError(ERROR_ACCESS_DENIED,
			"WriteMemory: It appears the block requested was not accessible");
	else
		SetLastError(ERROR_ACCESS_DENIED,
			"WriteMemory: Operation failed with no specified error");
	return false;
}


Dword App::FindProcId()const
{
	PIDList list = pe::FindProcId(sName);
	if (list.isEmpty())
		return 0;
	return *list.Value;
}

void App::OpenStreamProcess(Bool bWrite)
{
	if (pId != 0)
	{
		hProc = OpenProcess(PROCESS_VM_READ |
			PROCESS_QUERY_INFORMATION | PROCESS_SUSPEND_RESUME |
			PROCESS_TERMINATE |
			((bWrite) ? (PROCESS_VM_WRITE | PROCESS_VM_OPERATION) : 0),
			FALSE, pId);
		if (HandleBad(hProc))
		{
			if (GetLastError() == ERROR_ACCESS_DENIED)
			{
				SetLastError(ERROR_ACCESS_DENIED,
					"OpenProcess failed with error: ACCESS_DENIED");
				return;
			}
			SetLastError(ERROR_INVALID_HANDLE,
				"OpenProcess failed returning INVALID_HANDLE_VALUE");
			return;
		}
		BOOL bToken = OpenProcessToken(hProc,
			TOKEN_READ, &hToken);
		if (!(bToken && HandleGood(hToken)))
		{
			if (GetLastError() == ERROR_ACCESS_DENIED)
				SetLastError(ERROR_ACCESS_DENIED,
					"OpenProcessToken failed with error: ACCESS_DENIED");
			else
				SetLastError(ERROR_INVALID_HANDLE,
					"OpenProcessToken returned INVALID_HANDLE_VALUE");
			return;
		}
		ExtractTokenInfo();
	}
}

void App::OpenProcessFromID(Dword PID, Bool bWrite)
{
	pId = PID;
	if (pId == 0)
	{
		SetLastError(0, "A PID cannot be 0");
	}
	this->OpenStreamProcess(bWrite);
	this->sName = FindProcNameW(this->pId);
}

void App::FindProcessName()
{
	if (!this->Good())
	{
		str::FreeW(&this->sLocation, 0);
		str::FreeW(&this->sName, 0);
		return;
	}
	StringW ret;
	Dword buffSize = 1024;
	str::AllocW(&ret, buffSize);
	if (QueryFullProcessImageNameW(hProc, 0,
		ret, &buffSize))
	{
		str::ReAllocW(&ret, buffSize);
		Int32 index = str::LastIndexOfW(ret, L'\\');
		if (index != EOF)
		{
			str::CopyW(&sLocation, ret, index + 1);
			str::CopyW(&sName, str::SinceW(ret, index + 1));
		}
		else {
			str::CopyW(&sName, ret);
		}
	}
	else {
		str::FreeW(&sLocation, 0);
		str::FreeW(&sName, 0);
	}
	str::FreeW(&ret, 0);
}


App& App::operator = (App&& app)
{
	return fromOtherStream(std::move(app));
}

App& App::fromOtherStream(App&& app)
{
	Close();
	app.dispose = false;
	dispose = true;
	hProc = app.hProc;
	hToken = app.hToken;
	hwnds.CopyFrom(app.hwnds);
	LoadedLibraries.CopyFrom(app.LoadedLibraries);
	pId = app.pId;
	str::CopyW(&sLocation, app.sLocation);
	str::CopyW(&sName, app.sName);
	str::CopyW(&sUser, app.sUser);
	__LastError = app.__LastError;
	str::CopyA(&__LastErrorStr, app.__LastErrorStr);
	return *this;
}
App& App::operator = (const App& app)
{
	return fromOtherStream(app);
}

App& App::fromOtherStream(const App& app)
{
	Close();
	dispose = true;
	hProc = app.hProc;
	hToken = app.hToken;
	hwnds.CopyFrom(app.hwnds);
	LoadedLibraries.CopyFrom(app.LoadedLibraries);
	pId = app.pId;
	str::CopyW(&sLocation, app.sLocation);
	str::CopyW(&sName, app.sName);
	str::CopyW(&sUser, app.sUser);
	__LastError = app.__LastError;
	str::CopyA(&__LastErrorStr, app.__LastErrorStr);
	return *this;
}

Bool PE_CALL App::Terminate(UInt exitCode)
{
	if (!Good())
	{
		SetLastError(ERROR_INVALID_HANDLE, "Must initialize class first!");
		return false;
	}
	if (TerminateProcess(hProc, exitCode))
	{
		hProc = INVALID_HANDLE_VALUE;
		return true;
	}
	SetLastError(GetLastError(),
		"Cannot terminate the process.");
	return false;
}

Bool App::isRunning()
{
	if (Good())
	{
		Dword exitCode = 0;
		if (GetExitCodeProcess(hProc, &exitCode))
			if (exitCode == STILL_ACTIVE)
				return true;
	}
	return false;
}

Bool App::getExitCode(Dword& dwExit)
{
	if (Good())
		if (GetExitCodeProcess(hProc, &dwExit))
			return dwExit != STILL_ACTIVE;
	return false;
}

Handle App::getProcAddr()
{
	return hProc;
}

const Handle App::getProcAddr()const
{
	return hProc;
}


PIDList pe::FindProcId(cStringW name)
{
	PIDList list;
	if (str::LenW(name) == 0)
		return list;
	PROCESSENTRY32W pEntry;
	memset(&pEntry, 0, sizeof(PROCESSENTRY32W));
	pEntry.dwSize = sizeof(PROCESSENTRY32W);
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (pe::HandleBad(hSnapshot))
		return list;
	if (!Process32FirstW(hSnapshot, &pEntry))
	{
		CloseHandle(hSnapshot);
		return list;
	}
	do {
		if (!str::EqualsW(name, pEntry.szExeFile))
		{
			list.push(pEntry.th32ProcessID);
		}
	} while (Process32NextW(hSnapshot, &pEntry));
	CloseHandle(hSnapshot);
	return list;
}
StringA pe::FindProcNameA(Dword PID)
{
	if (PID == 0)
	{
		return NULL;
	}
	HANDLE hProcess = OpenProcess(
		PROCESS_QUERY_LIMITED_INFORMATION,
		FALSE, PID);
	if (HandleGood(hProcess))
	{
		StringA ret;
		DWORD buffSize = 1024;
		str::AllocA(&ret, buffSize);
		if (QueryFullProcessImageNameA(hProcess,
			0, ret, &buffSize))
		{
			str::ReAllocA(&ret, buffSize);
		}
		else {
			str::FreeA(&ret, 0);
		}
		CloseHandle(hProcess);
		return ret;
	}
	return NULL;
}
StringW pe::FindProcNameW(Dword PID)
{
	if (PID == 0)
	{
		return NULL;
	}
	HANDLE hProcess = OpenProcess(
		PROCESS_QUERY_LIMITED_INFORMATION,
		FALSE, PID);
	if (HandleGood(hProcess))
	{
		StringW ret;
		DWORD buffSize = 1024;
		str::AllocW(&ret, buffSize);
		if (QueryFullProcessImageNameW(hProcess, 0, ret, &buffSize))
		{
			str::ReAllocW(&ret, buffSize);
		}
		else {
			str::Free(&ret, 0);
		}
		CloseHandle(hProcess);
		return ret;
	}
	return NULL;
}
