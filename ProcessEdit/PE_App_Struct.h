#pragma once
#include "PE_Header.h"
namespace pe
{

	using Key = Wchar;
	using KeyPtr = Key*;
	using LpLpChar = Char**;
	using LpLpWcharT = Wchar**;
	typedef struct {
		SuperAnsiString sPath;
		HMODULE hModule;
	} hLoadedLib;
	using PLoadedLibraries = SuperArray<hLoadedLib>;

	typedef struct
	{

	} ProcessInfo, * pProcessInfo, LPProcessInfo;
	enum enumICON
	{
		EXTRALARGE_SIZE = 0,
		LARGE_SIZE,
		SMALL_SIZE,
		MAX_SIZE
	};




	namespace Reserved {
		using __MessageBoxAProc = int(__stdcall*)(HWND, LPCSTR, LPCSTR, UINT);
		typedef struct
		{
			char* lpTitle;
			char* lpMessage;
			Hwnd hWnd;
			UInt uType;
			Dword dwAddr;
			Int32* iExitCode;
		} *__LPMessageBoxAData, __MessageBoxAData;
		using __MessageBoxWProc = int(__stdcall*)(HWND, LPCWSTR, LPCWSTR, UINT);
		typedef struct
		{
			wchar_t* lpTitle;
			wchar_t* lpMessage;
			Hwnd hWnd;
			UInt uType;
			Dword dwAddr;
			Int32* iExitCode;
		} *__LPMessageBoxWData, __MessageBoxWData;

		using __DeleteFileAProc = BOOL(__stdcall*)(LPCSTR);
		using __DeleteFileWProc = BOOL(__stdcall*)(LPCWSTR);
		typedef struct {
			Byte memJmp[6];
			Byte prevJmp[6];
			Memory pOldFunc;
			Memory pNewFunc;
			Memory pHook;
			Bool isActive;
		} Hook, * LPHook;


		typedef struct {
			Int32* iExitCode;
			Dword dwAddr;
		} BOOLStruct, * lpBOOLStruct;
		using BOOLProcedure = BOOL(__stdcall*)();
		typedef struct {
			Int32* iExitCode;
			Dword dwAddr;
			LPSTR pString;
		} LPCSTRStruct, * lpLPCSTRStruct;
		using LPCSTRProcedure = BOOL(__stdcall*)(LPCSTR);
		typedef struct {
			Int32* iExitCode;
			Dword dwAddr;
			LPWSTR pString;
		} LPCWSTRStruct, * lpLPCWSTRStruct;
		using LPCWSTRProcedure = BOOL(__stdcall*)(LPCWSTR);
		typedef struct {
			Int32* iExitCode;
			Dword dwAddr;
			Dword* dwParam;
		} DWORDStruct, * lpDWORDStruct;
		using DWORDProcedure = BOOL(__stdcall*)(DWORD);
		typedef struct {
			Int32* iExitCode;
			Dword dwAddr;
			CHAR* sOut;
			Handle* hStream;
			Dword* dwWrite;
			Dword** dwWritten;
		} PrintStructA, * lpPrintStructA;
		using PrintProcedureA = BOOL(__stdcall*)(HANDLE, LPCVOID, DWORD, LPDWORD, LPVOID);
		typedef struct {
			Int32* iExitCode;
			Dword dwAddr;
			WCHAR* sOut;
			Handle* hStream;
			Dword* dwWrite;
			Dword** dwWritten;
		} PrintStructW, * lpPrintStructW;
		using PrintProcedureW = BOOL(__stdcall*)(HANDLE, LPCVOID, DWORD, LPDWORD, LPVOID);
	}
}
