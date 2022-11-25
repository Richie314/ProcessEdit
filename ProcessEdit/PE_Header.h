#pragma once
#define PE_DLLEXPORT	__declspec(dllexport)
#define PE_DLLIMPORT	__declspec(dllimport)

#ifdef PROCESSEDIT_EXPORTS
#define PE_API		PE_DLLEXPORT
#else
#define PE_API		PE_DLLIMPORT
#endif
#define PE_INLINE inline
#define PE_CALL __stdcall
#include "PE_Strings.h"
#include <Windows.h>
#ifndef _INC_TOOLHELP32
#include <TlHelp32.h>
#endif // !_INC_TOOLHELP32
#ifndef _MEMORYAPI_H_
#include <memoryapi.h>
#endif // !_MEMORYAPI_H_
#ifndef _INC_COMMDLG
#include <commdlg.h>
#endif // !_INC_COMMDLG
#ifndef _INC_SHELLAPI
#include <shellapi.h>
#endif // !_INC_SHELLAPI
#ifndef _WINHTTPX_
#include <winhttp.h>
#endif // !_WINHTTPX_
//same as the one from Microsoft, winnit.h
#define PE_DECLARE_HANDLE(Name) struct Name##__{ int unused;  }; typedef struct Name##__ *Name
namespace pe
{
	using Word = unsigned short;
	using Byte = unsigned char;
	using Memory = void*;
	using Int8 = char;
	using Int16 = short;
	using Int = int;
	using Int32 = int;
	using Int64 = long long;
	using Short = short;
	using Uint8 = Byte;
	using UShort = unsigned short;
	using Long = long long;
	using ULong = unsigned long long;
	using Bool = bool;
	using Boolean = bool;
	using Dword = unsigned long;
	using Uint = unsigned int;
	using UInt = unsigned int;
	using Unsigned = unsigned int;
	using Float = float;
	using Float32 = float;
	using Double = double;
	using Float64 = double;


#define PE_NULL(t) (static_cast<t*>(nullptr))

	struct MemoryConatiner
	{
		Byte* addr = PE_NULL(Byte);
		ULong size = 0;
	} const DefaultMemoryStruct({ PE_NULL(Byte), 0Ui64 });

	typedef HINSTANCE   Hinst;
	typedef HWND        Hwnd;
	typedef HFONT       HFont;
	typedef HICON       HIcon;
	typedef LRESULT     EventAnalisys;
	typedef tagMSG      Msg, TagMsg;
	typedef HANDLE      Handle;
	typedef OVERLAPPED  Overlapped;


	PE_API void	Swap(Memory, Memory, size_t = sizeof(Int32));

	template<class T>
	inline void SwapT(T& L, T& R)
	{
		Swap(&L, &R, sizeof(T));
	}


	inline constexpr Bool HandleGood(const Handle hA)
	{
		return hA != ((Handle)(nullptr)) && hA != INVALID_HANDLE_VALUE;
	}
	inline constexpr Bool HandleBad(const Handle hA)
	{
		return hA == ((Handle)(nullptr)) || hA == INVALID_HANDLE_VALUE;
	}

	inline constexpr Bool HandleGood(const Hwnd hA)
	{
		return HandleGood(static_cast<Handle>(hA));
	}
	inline constexpr Bool HandleBad(const Hwnd hA)
	{
		return HandleBad(static_cast<Handle>(hA));
	}
}
