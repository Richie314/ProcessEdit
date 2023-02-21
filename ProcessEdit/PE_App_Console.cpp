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
void App::Console_t::pReserved(Memory a, Memory b, Memory c)
{
	pParent = a;
	hStream = static_cast<Handle>(b);
	hWnd = static_cast<Hwnd>(c);
	bGood = a != NULL && c != NULL;
}
Bool App::Console_t::Missing()const
{
	return this->pParent == NULL ||
		this->hStream == NULL || this->hWnd == NULL;
}
BOOL __stdcall RemoteVoidProcedureThread(Reserved::BOOLStruct* data)
{
	*(data->iExitCode) = ((Reserved::BOOLProcedure)(data->dwAddr))();
	return *(data->iExitCode);
}
BOOL __stdcall RemoteStringAProcedureThread(Reserved::LPCSTRStruct* data)
{
	*(data->iExitCode) = ((Reserved::LPCSTRProcedure)(data->dwAddr))(data->pString);
	return *(data->iExitCode);
}
BOOL __stdcall RemoteStringWProcedureThread(Reserved::LPCWSTRStruct* data)
{
	*(data->iExitCode) = ((Reserved::LPCWSTRProcedure)(data->dwAddr))(data->pString);
	return *(data->iExitCode);
}
BOOL __stdcall RemoteDWORDProcedureThread(Reserved::DWORDStruct* data)
{
	*(data->iExitCode) = ((Reserved::DWORDProcedure)(data->dwAddr))(*data->dwParam);
	return *(data->iExitCode);
}
BOOL __stdcall RemotePrintAProcedureThread(Reserved::PrintStructA* data)
{
	*(data->iExitCode) =
		((Reserved::PrintProcedureA)(data->dwAddr))(data->hStream, data->sOut,
			*data->dwWrite, *data->dwWritten, NULL);
	return *(data->iExitCode);
}
BOOL __stdcall RemotePrintWProcedureThread(Reserved::PrintStructW* data)
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
BOOL PE_CALL App::Console_t::Create()
{
	App* app = static_cast<App*>(pParent);
	if (app == nullptr)
	{
		return FALSE;
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
BOOL PE_CALL App::Console_t::Create(cStringA sCaption)
{
	App* app = static_cast<App*>(pParent);
	if (app == nullptr)
	{
		return FALSE;
	}
	BOOL bSuccess = app->InjectVoidFunction("AllocConsole", "kernel32.dll",
		RemoteAllocConsoleThread);
	if (bSuccess != FALSE)
	{
		UpdateHandle();
		hWnd = (Hwnd)app->InjectVoidFunction("GetConsoleWindow", "kernel32.dll",
			RemoteGetConsoleWindowThread);
		bSuccess = static_cast<BOOL>(HandleGood(hWnd));
	}
	bGood = HandleGood(hWnd);
	if (bSuccess && sCaption)
	{
		return SetTitle(sCaption);
	}
	return FALSE;
}
BOOL PE_CALL App::Console_t::Create(cStringW sCaption)
{
	App* app = static_cast<App*>(pParent);
	if (app == nullptr)
	{
		return FALSE;
	}
	BOOL bSuccess = app->InjectVoidFunction("AllocConsole", "kernel32.dll",
		RemoteAllocConsoleThread);
	if (bSuccess != FALSE)
	{
		hWnd = (Hwnd)app->InjectVoidFunction("GetConsoleWindow", "kernel32.dll",
			RemoteGetConsoleWindowThread);
		bSuccess = static_cast<BOOL>(HandleGood(hWnd));
	}
	bGood = HandleGood(hWnd);
	if (bSuccess && sCaption)
	{
		UpdateHandle();
		return SetTitle(sCaption);
	}
	return FALSE;
}
BOOL PE_CALL App::Console_t::SetTitle(cStringA sCaption)
{
	App* app = static_cast<App*>(pParent);
	if (app == nullptr)
	{
		return FALSE;
	}
	if (sCaption)
	{
		return app->InjectStringFunctionA("SetConsoleTitleA", "kernel32.dll",
			RemoteSetConsoleAThread, sCaption);
	}
	app->SetLastError(ERROR_INVALID_HANDLE,
		"No active console found");
	return FALSE;
}
BOOL PE_CALL App::Console_t::SetTitle(cStringW sCaption)
{
	App* app = static_cast<App*>(pParent);
	if (app == nullptr)
	{
		return FALSE;
	}
	if (sCaption)
	{
		return app->InjectStringFunctionW("SetConsoleTitleW", "kernel32.dll",
			RemoteSetConsoleWThread, sCaption);
	}
	app->SetLastError(ERROR_INVALID_HANDLE,
		"No active console found");
	return FALSE;
}
BOOL PE_CALL App::Console_t::Destroy()
{
	App* app = static_cast<App*>(pParent);
	if (app == nullptr)
	{
		return FALSE;
	}
	if (app->InjectVoidFunction("FreeConsole", "kernel32.dll",
		RemoteFreeConsoleThread))
	{
		hWnd = NULL;
		hStream = NULL;
		bGood = false;
		return TRUE;
	}
	return FALSE;
}
BOOL PE_CALL App::Console_t::Write(cStringA sData)
{
	App* app = static_cast<App*>(pParent);
	if (app == nullptr)
	{
		return FALSE;
	}
	if (sData)
	{
		return app->InjectPrintFunctionA("WriteConsoleA", "kernel32.dll",
			RemoteWriteConsoleAThread, hStream, sData, NULL);
	}
	app->SetLastError(ERROR_INVALID_HANDLE,
		"No active console found");
	return FALSE;
}
BOOL PE_CALL App::Console_t::Write(cStringW sData)
{
	App* app = static_cast<App*>(pParent);
	if (app == nullptr)
	{
		return FALSE;
	}
	if (sData)
	{
		return app->InjectPrintFunctionW("WriteConsoleW", "kernel32.dll",
			RemoteWriteConsoleWThread, hStream, sData, NULL);
	}
	app->SetLastError(ERROR_INVALID_HANDLE,
		"No active console found");
	return FALSE;
}
void App::Console_t::UpdateHandle()
{
	if (hWnd == NULL)
		return;
	App* app = static_cast<App*>(pParent);
	if (app == nullptr)
	{
		return;
	}
	hStream = (Handle)app->InjectDWORDFunction(
		"GetStdHandle", "kernel32.dll",
		RemoteGetStdHandleThread, STD_OUTPUT_HANDLE);
	bGood = HandleGood(hStream);
}