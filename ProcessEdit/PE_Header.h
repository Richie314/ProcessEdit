#pragma once

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

#include "PE_Strings.h"
namespace pe
{
	typedef HINSTANCE   Hinst;
	typedef HWND        Hwnd;
	typedef HFONT       HFont;
	typedef HICON       HIcon;
	typedef LRESULT     EventAnalisys;
	typedef tagMSG      Msg, TagMsg;
	typedef HANDLE      Handle;
	typedef OVERLAPPED  Overlapped;
	typedef SYSTEM_INFO SysInfo;

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
