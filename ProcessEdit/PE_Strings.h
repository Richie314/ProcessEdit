#pragma once
#include "PE_Definitions.h"
namespace pe
{
	using Char = char;
	using Wchar = wchar_t;
	using StringA = char*;
	using cStringA = const char*;
	using StringW = wchar_t*;
	using cStringW = const wchar_t*;
#ifndef EOF
#define EOF (-1)
#endif
#ifdef UNICODE
	using String = StringW;
	using cString = cStringW;
#else
	using String = StringA;
	using cString = cStringA;
#endif
	namespace str
	{
		PE_API Bool PE_CALL CatA(StringA, cStringA);
		PE_API Bool PE_CALL CopyA(StringA*, cStringA);
		PE_API Bool PE_CALL CopyA(StringA*, cStringA, size_t);
		PE_API Bool PE_CALL ReplaceA(StringA, Char, Char);
		PE_API StringA SinceA(StringA, size_t);
		PE_API cStringA SinceA(cStringA, size_t);

		PE_API Bool PE_CALL AllocA(StringA*, size_t);
		PE_API Bool PE_CALL ReAllocA(StringA*, size_t);
		PE_API Bool PE_CALL FreeA(StringA*, size_t);

		PE_API size_t LenA(cStringA);
		PE_API Int32 IndexOfA(cStringA, Char);
		PE_API Int32 LastIndexOfA(cStringA, Char);

		PE_API Bool ContainsA(cStringA, Char);
		PE_API Bool StartsWithA(cStringA, Char);
		PE_API Bool EqualsA(cStringA, cStringA);
		PE_API Bool EndsWithA(cStringA, Char);


		PE_API Bool PE_CALL CatW(StringW, cStringW);
		PE_API Bool PE_CALL CopyW(StringW*, cStringW);
		PE_API Bool PE_CALL CopyW(StringW*, cStringW, size_t);
		PE_API Bool PE_CALL ReplaceW(StringW, Wchar, Wchar);
		PE_API StringW SinceW(StringW, size_t);
		PE_API cStringW SinceW(cStringW, size_t);

		PE_API Bool PE_CALL AllocW(StringW*, size_t);
		PE_API Bool PE_CALL ReAllocW(StringW*, size_t);
		PE_API Bool PE_CALL FreeW(StringW*, size_t);

		PE_API size_t LenW(cStringW);
		PE_API Int32 IndexOfW(cStringW, Wchar);
		PE_API Int32 LastIndexOfW(cStringW, Wchar);

		PE_API Bool ContainsW(cStringW, Wchar);
		PE_API Bool StartsWithW(cStringW, Wchar);
		PE_API Bool EqualsW(cStringW, cStringW);
		PE_API Bool EndsWithW(cStringW, Wchar);


#ifdef UNICODE
		constexpr PE_INLINE Bool PE_CALL Cat(String left, cString right)
		{
			return CatW(left, right);
		}
		constexpr PE_INLINE Bool PE_CALL Alloc(String* block, size_t size)
		{
			return AllocW(block, size);
		}

		constexpr PE_INLINE Bool PE_CALL ReAlloc(String* block, size_t size)
		{
			return ReAllocW(block, size);
		}
		constexpr PE_INLINE Bool PE_CALL Free(String* block, size_t size)
		{
			return FreeW(block, size);
		}
		constexpr PE_INLINE size_t Len(cString s)
		{
			return LenW(s);
		}
		constexpr PE_INLINE Bool Contains(cString s, Wchar w)
		{
			return ContainsW(s, w);
		}
		constexpr PE_INLINE Bool PE_CALL Copy(String* block, cString s)
		{
			return CopyW(block, s);
		}
		constexpr PE_INLINE Int32 IndexOf(cString s, Wchar w)
		{
			return IndexOfW(s, w);
		}
		constexpr PE_INLINE Bool StartsWith(cString s, Wchar w)
		{
			return StartsWithW(s, w);
		}
		constexpr PE_INLINE Bool Equals(cString l, cString r)
		{
			return EqualsW(l, r);
		}
		constexpr PE_INLINE Bool EndsWith(cString s, Wchar w)
		{
			return EndsWithW(s, w);
		}
	}
#else
		
#endif
}