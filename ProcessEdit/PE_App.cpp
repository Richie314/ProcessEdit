#include "pch.h"
#include "PE_App.h"
#include <Psapi.h>
using namespace pe;
void App::SetLastError(Uint error,
	const StringA& errorStr)
{
	this->__LastError = error;

	this->__LastErrorStr = errorStr;
}
Bool isValidProgramStr(const StringW& str)
{
	if (str::ContainsW(L'.'))
	{
		auto _end = str.substr(str.lastIndexOf(L'.'), str.size);
		return (_end == L".exe" || _end == L".dll");
	}
	return false;
}

template <class A, Bool b>
void GetAllWindowsFromProcessID(
	DWORD dwProcessID, SuperArray<Hwnd, A, b>& vhWnds)
{
	Hwnd hCurWnd = NULL;
	do
	{
		hCurWnd = FindWindowExW(NULL, hCurWnd, NULL, NULL);
		Dword dwProcID = 0;
		GetWindowThreadProcessId(hCurWnd, &dwProcID);
		if (dwProcID == dwProcessID)
		{
			vhWnds.push_back(hCurWnd);
		}
	} while (hCurWnd != NULL);
}


App::App()
	:hProc(INVALID_HANDLE_VALUE),
	hToken(INVALID_HANDLE_VALUE),
	pId(0),
	__LastError(PIE_ERROR_NOERROR),
	__LastErrorTime(0),
	dispose(true)
{
	this->ListLibraries();
};
App::App(const SuperUnicodeString& sProcess, Bool canWrite)
	:hProc(INVALID_HANDLE_VALUE),
	hToken(INVALID_HANDLE_VALUE),
	pId(0),
	__LastError(PIE_ERROR_NOERROR),
	__LastErrorTime(0),
	dispose(true)
{
	if (!isValidProgramStr(sProcess))
		return;
	auto sP = sProcess.replace(L'/', L'\\');
	if (sProcess.contains('\\'))
	{
		Int32 index = sP.lastIndexOf(L'\\') + 1;
		this->sLocation = sP.substr(0, index);
		this->sName = sP.substr(index, sP.size);
	}
	this->pId = this->FindProcId();
	this->OpenStreamProcess(canWrite);
	this->UpdateWindowCount();
	this->ListLibraries();
};
App::App(Dword PID, Bool canWrite)
	:hProc(INVALID_HANDLE_VALUE),
	hToken(INVALID_HANDLE_VALUE),
	pId(0),
	__LastError(PIE_ERROR_NOERROR),
	__LastErrorTime(0),
	dispose(true)
{
	this->OpenProcessFromID(PID, canWrite);
	this->UpdateWindowCount();
	this->ListLibraries();
};
App::App(const ProcessBasicInfo& pbiInfo, Bool canWrite)
	:hProc(INVALID_HANDLE_VALUE),
	hToken(INVALID_HANDLE_VALUE),
	pId(0),
	__LastError(PIE_ERROR_NOERROR),
	__LastErrorTime(0),
	dispose(true)
{
	this->OpenProcessFromID(pbiInfo.dwId, canWrite);
	this->UpdateWindowCount();
	this->ListLibraries();
};
App::App(const pProcessBasicInfo pPBI, Bool canWrite)
	:hProc(INVALID_HANDLE_VALUE),
	hToken(INVALID_HANDLE_VALUE),
	pId(0),
	__LastError(PIE_ERROR_NOERROR),
	__LastErrorTime(0),
	dispose(true)
{
	if (pPBI != PIE_NULL(ProcessBasicInfo))
	{
		this->OpenProcessFromID(pPBI->dwId, canWrite);
		this->UpdateWindowCount();
	}
	this->SetLastError(PIE_ERROR_INVALID_PID | PIE_ERROR_INVALID_PARAM,
		"Trying to initialize with a NULL pointer: expected a \"ProcessBasicInfo\" pointer.");
	this->ListLibraries();
};
App::App(App& otherApp)
	:hProc(INVALID_HANDLE_VALUE),
	hToken(INVALID_HANDLE_VALUE),
	pId(0),
	__LastError(PIE_ERROR_NOERROR),
	__LastErrorTime(0),
	dispose(true),
	ListLoadedLibraries(otherApp.ListLoadedLibraries)
{
	this->fromOtherStream(otherApp);
}
App::~App()
{
	this->Close();
}

Procedure App::Close()
{
	if (this->dispose)
	{
		if (HandleGood(this->hProc))
			CloseHandle(this->hProc);
		if (HandleGood(this->hToken))
			CloseHandle(this->hToken);
	}
	this->hwnds.clear();
	this->sLocation.clear();
	this->sName.clear();
	this->sUser.clear();
	this->__LastErrorStr.clear();
	this->dispose = false;
	this->pId = 0;
	this->ListLoadedLibraries.clear();
}

Hwnd App::FindThisWindowFromContent(const SuperUnicodeString& caption)const
{
	for (Hwnd hwnd : this->hwnds)
	{
		if (GetWindowTextSW(hwnd) == caption)
			return hwnd;
	}
	return NULL;
}
Hwnd App::FindThisWindowFromContent(const SuperAnsiString& caption)const
{
	for (Hwnd hwnd : this->hwnds)
	{
		if (GetWindowTextSA(hwnd) == caption)
			return hwnd;
	}
	return NULL;
}

Procedure App::ExtractTokenInfo()
{
	Dword dwProcessTokenInfoAllocSize = 0, error = 0;
	GetTokenInformation(this->hToken, TokenUser,
		NULL, 0, &dwProcessTokenInfoAllocSize);
	error = GetLastError();
	if (error == ERROR_ACCESS_DENIED)
	{
		this->SetLastError(PIE_ERROR_ACCESSERROR,
			"GetTokenInformation failed with error: ACCESS_DENIED");
		return;
	}
	else if (error == ERROR_INSUFFICIENT_BUFFER)
	{
		PTOKEN_USER pUserToken = reinterpret_cast<PTOKEN_USER>
			(Heap.Create(dwProcessTokenInfoAllocSize));
		if (pUserToken == NULL)
		{
			this->SetLastError(PIE_ERROR_MEMORY_ERROR,
				"Not enough space");
			return;
		}
		if (!GetTokenInformation(
			this->hToken,
			TokenUser, pUserToken,
			dwProcessTokenInfoAllocSize,
			&dwProcessTokenInfoAllocSize))
		{
			if (GetLastError() == ERROR_ACCESS_DENIED)
				this->SetLastError(PIE_ERROR_ACCESSERROR,
					"GetTokenInformation failed with error: ACCESS_DENIED");
			else
				this->SetLastError(PIE_ERROR_ACCESSERROR,
					"GetTokenInformation failed with unknown error");

			Heap.Destroy(pUserToken);
			return;
		}
		SID_NAME_USE   snuSIDNameUse;
		wchar_t szUser[256];
		memset(szUser, 0, 512);
		Dword dwUserNameLength = 256;
		wchar_t szDomain[256];
		memset(szUser, 0, 512);
		Dword dwDomainNameLength = 256;
		if (LookupAccountSidW(NULL,
			pUserToken->User.Sid,
			szUser, &dwUserNameLength,
			szDomain, &dwDomainNameLength,
			&snuSIDNameUse))
		{
			this->sUser = L"\\\\";
			this->sUser += szDomain;
			this->sUser += L"\\";
			this->sUser += szUser;
		}
		Heap.Destroy(pUserToken);
	}
}

Procedure App::UpdateWindowCount()
{
	if (this->Good())
		GetAllWindowsFromProcessID(this->pId, this->hwnds);
}

const SuperByteArray App::ReadMemory(const Memory addr, Uint size)const
{
	if (addr == 0 || size == 0 || (!this->Good()))
		return std::move(SuperByteArray());
	Memory buff = Heap.Create(size);
	SIZE_T nbytesRead = 0;
	if (ReadProcessMemory(this->hProc, addr, &buff, size, &nbytesRead))
	{
		SuperByteArray buffArray((Byte*)buff, nbytesRead);
		Heap.Destroy(buff);
		return std::move(buffArray);
	}
	return std::move(SuperByteArray());
}

Bool P_ENTRY App::WriteMemory(const Memory addr, const SuperByteArray& buffer)
{
	if ((!this->Good()) || addr == NULL || buffer.empty())
		return false;
	SIZE_T nSize = buffer.size;
	if (WriteProcessMemory(this->hProc, addr, buffer.begin(),
		nSize, &nSize))
	{
		return true;
	}
	if (GetLastError() == ERROR_ACCESS_DENIED)
		this->SetLastError(PIE_ERROR_ACCESSERROR,
			"WriteMemory: It appears the block requested was not accessible");
	else
		this->SetLastError(PIE_ERROR_ACCESSERROR,
			"WriteMemory: Operation failed with no specified error");
	return false;
}
Bool P_ENTRY App::WriteMemory(const Memory addr, const SuperAnsiString& buffer)
{
	return this->WriteMemory(addr, std::move(
		SuperByteArray((Byte*)buffer.begin(), buffer.size)));
}
Bool P_ENTRY App::WriteMemory(const Memory addr, const SuperUnicodeString& buffer)
{
	return this->WriteMemory(addr, std::move(
		SuperByteArray((Byte*)buffer.begin(), 2 * buffer.size)));
}
Bool P_ENTRY App::WriteMemory(const Memory addr, const Memory buffer, size_t size)
{
	return this->WriteMemory(addr, std::move(
		SuperByteArray((Byte*)buffer, size)));
}

Dword App::FindProcId()const
{
	PIDlist list = Pie_314::FindProcId(this->sName);
	return ((list.empty()) ? (0) : (list.at(0)));
}

Procedure App::OpenStreamProcess(Bool bWrite)
{
	if (this->pId != 0)
	{
		this->hProc = OpenProcess(PROCESS_VM_READ |
			PROCESS_QUERY_INFORMATION | PROCESS_SUSPEND_RESUME |
			PROCESS_TERMINATE |
			((bWrite) ? (PROCESS_VM_WRITE | PROCESS_VM_OPERATION) : 0),
			FALSE, pId);
		if (HandleBad(this->hProc))
		{
			if (GetLastError() == ERROR_ACCESS_DENIED)
				this->SetLastError(PIE_ERROR_ACCESSERROR,
					"OpenProcess failed with error: ACCESS_DENIED");
			else
				this->SetLastError(PIE_ERROR_HANDLE_ERROR,
					"OpenProcess failed returning INVALID_HANDLE_VALUE");
			return;
		}
		this->Aip.Init(this->hProc, &this->__LastError);
		BOOL bToken = OpenProcessToken(this->hProc,
			TOKEN_READ, &this->hToken);
		if (!(bToken && HandleGood(this->hToken)))
		{
			if (GetLastError() == ERROR_ACCESS_DENIED)
				this->SetLastError(PIE_ERROR_ACCESSERROR,
					"OpenProcessToken failed with error: ACCESS_DENIED");
			else
				this->SetLastError(PIE_ERROR_HANDLE_ERROR,
					"OpenProcessToken returned INVALID_HANDLE_VALUE");
			return;
		}
		this->ExtractTokenInfo();
	}
}

Procedure App::OpenProcessFromID(Dword PID, Bool bWrite)
{
	this->pId = PID;
	if (this->pId == 0)
	{
		this->SetLastError(PIE_ERROR_INVALID_PID, "A PID cannot be 0");
	}
	this->OpenStreamProcess(bWrite);
	this->sName = FindProcNameW(this->pId);
}

Procedure App::FindProcessName()
{
	if (!this->Good())
	{
		this->sLocation.clear();
		this->sName.clear();
		return;
	}
	SuperUnicodeString ret;
	Dword buffSize = 1024;
	ret.resize(1024U);
	if (QueryFullProcessImageNameW(this->hProc, 0,
		ret.begin(), &buffSize))
	{
		ret.resize(buffSize);
		Int32 index = ret.lastIndexOf(L'\\');
		if (index != EOF)
		{
			this->sLocation = ret.substr(0, index + 1);
			this->sName = ret.substr(index + 1, ret.size);
		}
		else {
			this->sName = ret;
		}
	}
	else {
		this->sLocation.clear();
		this->sName.clear();
	}
	ret.clear();
}

SuperUnicodeString App::toStringW()const
{
	if (this->Good())
		return L"Process: " + this->sLocation + this->sName + L"\r\n" +
		L"Owner:\t" + this->sUser + L"\r\n" +
		L"PID:\t" + PrintDecW(this->pId) + L"\r\n" +
		L"HANDLE:\t0x" + PrintHexW((size_t)this->hProc) + L"\r\n";
	return L"Process not specified yet!";
}
SuperAnsiString App::toStringA()const
{
	return this->toStringW().forEach<char>([](wchar_t w) {
		return static_cast<char>(w); });
}

App& App::operator=(App& app)
{
	return this->fromOtherStream(app);
}

App& App::fromOtherStream(App& app)
{
	this->Close();
	app.dispose = false;
	this->dispose = true;
	this->hProc = app.hProc;
	this->hToken = app.hToken;
	this->hwnds = app.hwnds;
	this->pId = app.pId;
	this->sLocation = app.sLocation;
	this->sName = app.sName;
	this->sUser = app.sUser;
	this->__LastError = app.__LastError;
	this->__LastErrorStr = app.__LastErrorStr;
	this->__LastErrorTime = app.__LastErrorTime;
	this->Aip.Init(this->hProc, &this->__LastError);
	return *this;
}

Bool P_ENTRY App::Terminate(UInt exitCode)
{
	if (!this->Good())
	{
		this->SetLastError(PIE_ERROR_INVALID_CALL, "Must initialize class first!");
		return false;
	}
	if (TerminateProcess(this->hProc, exitCode))
	{
		this->hProc = INVALID_HANDLE_VALUE;
		return true;
	}
	this->SetLastError(PIE_ERROR_CANNOT_TERMINATE,
		"Cannot terminate the process.");
	return false;
}

Bool App::isRunning()
{
	if (this->Good())
	{
		Dword exitCode = 0;
		if (GetExitCodeProcess(this->hProc, &exitCode))
			if (exitCode == STILL_ACTIVE)
				return true;
	}
	return false;
}

Bool App::getExitCode(Dword& dwExit)const
{
	if (this->Good())
		if (GetExitCodeProcess(this->hProc, &dwExit))
			return dwExit != STILL_ACTIVE;
	return false;
}

Handle App::getProcAddr()
{
	return this->hProc;
}

const Handle App::getProcAddr()const
{
	return this->hProc;
}


PIDlist Pie_314::FindProcId(const SuperUnicodeString& name)
{
	if (name.empty())
		return std::move(PIDlist());
	PROCESSENTRY32W pEntry;
	memset(&pEntry, 0, sizeof(PROCESSENTRY32W));
	pEntry.dwSize = sizeof(PROCESSENTRY32W);
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (!Pie_314::HandleGood(hSnapshot))
		return std::move(PIDlist());
	if (!Process32FirstW(hSnapshot, &pEntry))
	{
		CloseHandle(hSnapshot);
		return std::move(PIDlist());
	}
	if (!name.compare(pEntry.szExeFile))
	{
		CloseHandle(hSnapshot);
		return std::move(PIDlist());
	}
	PIDlist list;
	list.push_back(pEntry.th32ProcessID);
	while (Process32NextW(hSnapshot, &pEntry))
	{
		if (name.compare(pEntry.szExeFile))
		{
			list.push_back(pEntry.th32ProcessID);
		}
	}
	CloseHandle(hSnapshot);
	return std::move(list);
}
SuperAnsiString Pie_314::FindProcNameA(Pie_314::Dword PID)
{
	if (PID == 0)
	{
		Pie_314::setLastErrorNum(PIE_ERROR_INVALID_PID);
		return std::move(Pie_314::SuperAnsiString());
	}
	HANDLE hProcess = OpenProcess(
		PROCESS_QUERY_LIMITED_INFORMATION,
		FALSE, PID);
	if (HandleGood(hProcess))
	{
		SuperAnsiString ret;
		DWORD buffSize = 1024;
		ret.resize(1024U);
		if (QueryFullProcessImageNameA(hProcess,
			0, ret.begin(), &buffSize))
		{
			ret.resize(buffSize);
		}
		else {
			ret.clear();
		}
		CloseHandle(hProcess);
		return std::move(ret);
	}
	Pie_314::setLastErrorNum(PIE_ERROR_INVALID_PID);
	return std::move(Pie_314::SuperAnsiString());
}
SuperUnicodeString Pie_314::FindProcNameW(Pie_314::Dword PID)
{
	if (PID == 0)
	{
		Pie_314::setLastErrorNum(PIE_ERROR_INVALID_PID);
		return std::move(Pie_314::SuperUnicodeString());
	}
	HANDLE hProcess = OpenProcess(
		PROCESS_QUERY_LIMITED_INFORMATION,
		FALSE, PID);
	if (HandleGood(hProcess))
	{
		SuperUnicodeString ret;
		DWORD buffSize = 1024;
		ret.resize(1024U);
		if (QueryFullProcessImageNameW(hProcess, 0, ret.begin(), &buffSize))
		{
			ret.resize(buffSize);
		}
		else {
			ret.clear();
		}
		CloseHandle(hProcess);
		return std::move(ret);
	}
	Pie_314::setLastErrorNum(PIE_ERROR_INVALID_PID);
	return std::move(Pie_314::SuperUnicodeString());
}

Procedure ExtractProcessInfoReserved(ProcessBasicInfo& pbi, Handle hToken,
	TOKEN_INFORMATION_CLASS tokenInfo, SuperUnicodeString& sDataW)
{

	Dword dwProcessTokenInfoAllocSize = 0;
	GetTokenInformation(hToken, tokenInfo,
		NULL, 0, &dwProcessTokenInfoAllocSize);
	if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
		PTOKEN_USER pUserToken = reinterpret_cast<PTOKEN_USER>
			(Heap.Create(dwProcessTokenInfoAllocSize));
		if (pUserToken == NULL)
		{
			return;
		}
		if (!GetTokenInformation(
			hToken,
			tokenInfo, pUserToken,
			dwProcessTokenInfoAllocSize,
			&dwProcessTokenInfoAllocSize))
		{
			Heap.Destroy(pUserToken);
			return;
		}
		SID_NAME_USE   snuSIDNameUse;
		wchar_t szUser[256];
		memset(szUser, 0, 512);
		Dword dwUserNameLength = 256;
		wchar_t szDomain[256];
		memset(szUser, 0, 512);
		Dword dwDomainNameLength = 256;
		if (LookupAccountSidW(NULL,
			pUserToken->User.Sid,
			szUser, &dwUserNameLength,
			szDomain, &dwDomainNameLength,
			&snuSIDNameUse))
		{
			sDataW = L"\\\\";
			sDataW += szDomain;
			sDataW += L"\\";
			sDataW += szUser;
		}
		Heap.Destroy(pUserToken);
	}
}
SuperProcessInfoArray Pie_314::ListAllProcess()
{
	Dword pProcs[512], dwCBNeeded;
	if (!K32EnumProcesses(pProcs, 512 * sizeof(Dword), &dwCBNeeded))
	{
		return SuperProcessInfoArray();
	}
	Dword dwLen = dwCBNeeded / sizeof(Dword);
	Dword dwCount = 0;
	SuperProcessInfoArray aProcs(dwLen);
	for (Dword i = 0; i < dwLen; i++)
	{
		if (pProcs[i])
		{
			Handle hProc = OpenProcess(
				PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
				FALSE, pProcs[i]);
			if (HandleGood(hProc))
			{
				ProcessBasicInfo& pbi = aProcs[dwCount];
				memset(&pbi, 0, aProcs.getSingleElementSize());
				pbi.dwId = pProcs[i];
				Handle hToken = NULL;
				if (OpenProcessToken(hProc,
					TOKEN_READ, &hToken))
				{
					SuperUnicodeString sDataW;
					ExtractProcessInfoReserved(pbi, hToken,
						TokenUser, sDataW);
					memcpy_s(pbi.swOwner, 64, sDataW.c_str(), sizeof(Wchar) * sDataW.size);
					//ExtractProcessInfoReserved(pbi, hToken,
						//TokenOwner, sDataW);
					//memcpy_s(pbi.swOwner, 64, sDataW.c_str(), sizeof(Wchar) * sDataW.size);
					sDataW = FindProcNameW(pbi.dwId);
					int index = sDataW.lastIndexOf(L'\\');
					if (index >= 0)
					{
						auto s2 = sDataW.substr(index + 1);
						memcpy_s(pbi.swName, 64, s2.c_str(), 2 * s2.size);
						s2 = sDataW.substr(0, index + 1);
						memcpy_s(pbi.swName, 128, s2.c_str(), 2 * s2.size);
					}
					else {
						memcpy_s(pbi.swName, 64, sDataW.c_str(), 2 * sDataW.size);
					}
					CloseHandle(hToken);
				}
				CloseHandle(hProc);
			}
		}
	}
}