#pragma once
#include "PE_Header.h"
namespace pe
{

	using Key = Wchar;
	using KeyPtr = Key*;
	using LpLpChar = Char**;
	using LpLpWcharT = Wchar**;
	typedef struct {
		StringA sPath;
		HMODULE hModule;
	} hLoadedLib;

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


		struct BasicStruct 
		{
			Int32* iExitCode;
			Dword dwAddr;
		};
		using BOOLStruct = BasicStruct;
		using BOOLProcedure = BOOL(__stdcall*)();

		struct LPCSTRStruct : public BasicStruct
		{
			LPSTR pString;
		};
		using LPCSTRProcedure = BOOL(__stdcall*)(LPCSTR);

		struct LPCWSTRStruct : public BasicStruct
		{
			LPWSTR pString;
		};
		using LPCWSTRProcedure = BOOL(__stdcall*)(LPCWSTR);

		struct DWORDStruct : public BasicStruct
		{
			Dword* dwParam;
		};
		using DWORDProcedure = BOOL(__stdcall*)(DWORD);

		struct PrintStructA : public BasicStruct 
		{
			CHAR* sOut;
			Handle* hStream;
			Dword* dwWrite;
			Dword** dwWritten;
		};
		using PrintProcedureA = BOOL(__stdcall*)(HANDLE, LPCVOID, DWORD, LPDWORD, LPVOID);
		
		struct PrintStructW : public BasicStruct 
		{
			WCHAR* sOut;
			Handle* hStream;
			Dword* dwWrite;
			Dword** dwWritten;
		};
		using PrintProcedureW = BOOL(__stdcall*)(HANDLE, LPCVOID, DWORD, LPDWORD, LPVOID);
	}
}
