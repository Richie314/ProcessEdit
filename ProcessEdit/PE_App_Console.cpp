#include "pch.h"
#include "PE_App.h"
using namespace pe;

App::Console_t::Console_t() :hWnd(NULL), pParent(NULL), hStream(NULL), bGood(false)
{}
App::Console_t::Console_t(Memory a, Handle s, Hwnd w) :
	hWnd(w), pParent(a), hStream(s), bGood(a != NULL && w != NULL)
{}
App::Console_t::~Console_t()
{}
Procedure App::Console_t::pReserved(Memory a, Memory b, Memory c)
{
	this->pParent = a;
	this->hStream = static_cast<Handle>(b);
	this->hWnd = static_cast<Hwnd>(c);
	this->bGood = a != NULL && c != NULL;
}
Bool App::Console_t::Missing()const
{
	return this->pParent == NULL ||
		this->hStream == NULL || this->hWnd == NULL;
}
BOOL __stdcall RemoteVoidProcedureThread(Reserved::lpBOOLStruct data)
{
	*(data->iExitCode) = ((Reserved::BOOLProcedure)(data->dwAddr))();
	return *(data->iExitCode);
}
BOOL __stdcall RemoteStringAProcedureThread(Reserved::lpLPCSTRStruct data)
{
	*(data->iExitCode) = ((Reserved::LPCSTRProcedure)(data->dwAddr))(data->pString);
	return *(data->iExitCode);
}
BOOL __stdcall RemoteStringWProcedureThread(Reserved::lpLPCWSTRStruct data)
{
	*(data->iExitCode) = ((Reserved::LPCWSTRProcedure)(data->dwAddr))(data->pString);
	return *(data->iExitCode);
}
BOOL __stdcall RemoteDWORDProcedureThread(Reserved::lpDWORDStruct data)
{
	*(data->iExitCode) = ((Reserved::DWORDProcedure)(data->dwAddr))(*data->dwParam);
	return *(data->iExitCode);
}
BOOL __stdcall RemotePrintAProcedureThread(Reserved::lpPrintStructA data)
{
	*(data->iExitCode) =
		((Reserved::PrintProcedureA)(data->dwAddr))(data->hStream, data->sOut,
			*data->dwWrite, *data->dwWritten, NULL);
	return *(data->iExitCode);
}
BOOL __stdcall RemotePrintWProcedureThread(Reserved::lpPrintStructW data)
{
	*(data->iExitCode) = ((Reserved::PrintProcedureW)(data->dwAddr))(data->hStream, data->sOut,
		*data->dwWrite, *data->dwWritten, NULL);
	return *(data->iExitCode);
}
#define RemoteAllocConsoleThread     RemoteVoidProcedureThread
#define RemoteGetConsoleWindowThread RemoteVoidProcedureThread
#define RemoteFreeConsoleThread      RemoteVoidProcedureThread
#define RemoteSetConsoleAThread      RemoteStringAProcedureThread
#define RemoteSetConsoleWThread      RemoteStringWProcedureThread
#define RemoteGetStdHandleThread     RemoteDWORDProcedureThread
#define RemoteWriteConsoleAThread    RemotePrintAProcedureThread
#define RemoteWriteConsoleWThread    RemotePrintWProcedureThread
BOOL P_ENTRY App::Console_t::Create()
{
	App* app = static_cast<App*>(this->pParent);
	if (app == PIE_NULL(App))
	{
		setLastErrorNum(PIE_ERROR_INVALID_CALL);
		return 0;
	}
	BOOL bSuccess = app->InjectVoidFunction("AllocConsole", "kernel32.dll",
		RemoteAllocConsoleThread);
	if (bSuccess != FALSE)
	{
		this->hWnd = (Hwnd)app->InjectVoidFunction("GetConsoleWindow", "kernel32.dll",
			RemoteGetConsoleWindowThread);
		bSuccess = static_cast<BOOL>(HandleGood(this->hWnd));
	}
	this->bGood = HandleGood(this->hWnd);
	this->UpdateHandle();
	return bSuccess;
}
BOOL P_ENTRY App::Console_t::Create(const SuperAnsiString& sCaption)
{
	App* app = static_cast<App*>(this->pParent);
	if (app == PIE_NULL(App))
	{
		setLastErrorNum(PIE_ERROR_INVALID_CALL);
		return 0;
	}
	BOOL bSuccess = app->InjectVoidFunction("AllocConsole", "kernel32.dll",
		RemoteAllocConsoleThread);
	if (bSuccess != FALSE)
	{
		this->UpdateHandle();
		this->hWnd = (Hwnd)app->InjectVoidFunction("GetConsoleWindow", "kernel32.dll",
			RemoteGetConsoleWindowThread);
		bSuccess = static_cast<BOOL>(HandleGood(this->hWnd));
	}
	this->bGood = HandleGood(this->hWnd);
	if (bSuccess && sCaption.Good())
	{
		return this->SetTitle(sCaption);
	}
	return FALSE;
}
BOOL P_ENTRY App::Console_t::Create(const SuperUnicodeString& sCaption)
{
	App* app = static_cast<App*>(this->pParent);
	if (app == PIE_NULL(App))
	{
		setLastErrorNum(PIE_ERROR_INVALID_CALL);
		return 0;
	}
	BOOL bSuccess = app->InjectVoidFunction("AllocConsole", "kernel32.dll",
		RemoteAllocConsoleThread);
	if (bSuccess != FALSE)
	{
		this->hWnd = (Hwnd)app->InjectVoidFunction("GetConsoleWindow", "kernel32.dll",
			RemoteGetConsoleWindowThread);
		bSuccess = static_cast<BOOL>(HandleGood(this->hWnd));
	}
	this->bGood = HandleGood(this->hWnd);
	if (bSuccess && sCaption.Good())
	{
		this->UpdateHandle();
		bSuccess = app->InjectStringFunctionW("SetConsoleTitleW", "kernel32.dll",
			RemoteSetConsoleWThread, sCaption.c_str());
		return this->SetTitle(sCaption);
	}
	return FALSE;
}
BOOL P_ENTRY App::Console_t::SetTitle(const SuperAnsiString& sCaption)
{
	App* app = static_cast<App*>(this->pParent);
	if (app == PIE_NULL(App))
	{
		setLastErrorNum(PIE_ERROR_INVALID_CALL);
		return FALSE;
	}
	if (sCaption.Good())
	{
		return app->InjectStringFunctionA("SetConsoleTitleA", "kernel32.dll",
			RemoteSetConsoleAThread, sCaption.c_str());
	}
	app->SetLastError(PIE_ERROR_INVALID_CALL,
		"No active console found");
	return FALSE;
}
BOOL P_ENTRY App::Console_t::SetTitle(const SuperUnicodeString& sCaption)
{
	App* app = static_cast<App*>(this->pParent);
	if (app == PIE_NULL(App))
	{
		setLastErrorNum(PIE_ERROR_INVALID_CALL);
		return FALSE;
	}
	if (sCaption.Good())
	{
		return app->InjectStringFunctionW("SetConsoleTitleW", "kernel32.dll",
			RemoteSetConsoleWThread, sCaption.c_str());
	}
	app->SetLastError(PIE_ERROR_INVALID_CALL,
		"No active console found");
	return FALSE;
}
BOOL P_ENTRY App::Console_t::Destroy()
{
	App* app = static_cast<App*>(this->pParent);
	if (app == PIE_NULL(App))
	{
		setLastErrorNum(PIE_ERROR_INVALID_CALL);
		return FALSE;
	}
	if (app->InjectVoidFunction("FreeConsole", "kernel32.dll",
		RemoteFreeConsoleThread))
	{
		this->hWnd = NULL;
		this->hStream = NULL;
		this->bGood = false;
		return TRUE;
	}
	return FALSE;
}
BOOL P_ENTRY App::Console_t::Write(const SuperAnsiString& sData)
{
	App* app = static_cast<App*>(this->pParent);
	if (app == PIE_NULL(App))
	{
		setLastErrorNum(PIE_ERROR_INVALID_CALL);
		return FALSE;
	}
	if (sData.Good())
	{
		return app->InjectPrintFunctionA("WriteConsoleA", "kernel32.dll",
			RemoteWriteConsoleAThread, this->hStream, sData, NULL);
	}
	app->SetLastError(PIE_ERROR_INVALID_CALL,
		"No active console found");
	return FALSE;
}
BOOL P_ENTRY App::Console_t::Write(const SuperUnicodeString& sData)
{
	App* app = static_cast<App*>(this->pParent);
	if (app == PIE_NULL(App))
	{
		setLastErrorNum(PIE_ERROR_INVALID_CALL);
		return FALSE;
	}
	if (sData.Good())
	{
		return app->InjectPrintFunctionW("WriteConsoleW", "kernel32.dll",
			RemoteWriteConsoleWThread, this->hStream, sData, NULL);
	}
	app->SetLastError(PIE_ERROR_INVALID_CALL,
		"No active console found");
	return FALSE;
}
Procedure App::Console_t::UpdateHandle()
{
	if (this->hWnd == NULL)
		return;
	App* app = static_cast<App*>(this->pParent);
	if (app == PIE_NULL(App))
	{
		return;
	}
	this->hStream = (Handle)app->InjectDWORDFunction(
		"GetStdHandle", "kernel32.dll",
		RemoteGetStdHandleThread, STD_OUTPUT_HANDLE);
	this->bGood = HandleGood(this->hStream);
}