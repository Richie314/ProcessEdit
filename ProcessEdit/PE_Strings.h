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


		PE_API StringW AtoW(cStringA);
		PE_API StringA WtoA(cStringW);
#ifdef UNICODE
		PE_INLINE Bool PE_CALL Cat(String a, cString b)
		{
			return CatW(a, b);
		}
		PE_INLINE Bool PE_CALL Copy(String* a, cString b)
		{
			return CopyW(a, b);
		}
		PE_INLINE Bool PE_CALL Copy(String* a, cString b, size_t s)
		{
			return CopyW(a, b, s);
		}
		PE_INLINE Bool PE_CALL Replace(String s, Wchar a, Wchar b)
		{
			return ReplaceW(s, a, b);
		}
		PE_INLINE String Since(String s, size_t i)
		{
			return SinceW(s, i);
		}
		PE_INLINE cString Since(cString s, size_t i)
		{
			return SinceW(s, i);
		}

		PE_INLINE Bool PE_CALL Alloc(String* s, size_t l)
		{
			return AllocW(s, l);
		}
		PE_INLINE Bool PE_CALL ReAlloc(String* s, size_t l)
		{
			return ReAllocW(s, l);
		}
		PE_INLINE Bool PE_CALL Free(String* s, size_t l)
		{
			return FreeW(s, l);
		}

		PE_INLINE size_t Len(cString s)
		{
			return LenW(s);
		}
		PE_INLINE Int32 IndexOf(cString s, Wchar w)
		{
			return IndexOfW(s, w);
		}
		PE_INLINE Int32 LastIndexOf(cString s, Wchar w)
		{
			return LastIndexOfW(s, w);
		}

		PE_INLINE Bool Contains(cString s, Wchar w)
		{
			return ContainsW(s, w);
		}
		PE_INLINE Bool StartsWith(cString s, Wchar w)
		{
			return StartsWithW(s, w);
		}
		PE_INLINE Bool Equals(cString l, cString r)
		{
			return EqualsW(l, r);
		}
		PE_INLINE Bool EndsWith(cString s, Wchar w)
		{
			return EndsWithW(s, w);
		}
	
#else
		PE_INLINE Bool PE_CALL Cat(String a, cString b)
		{
			return CatA(a, b);
		}
		PE_INLINE Bool PE_CALL Copy(String* a, cString b)
		{
			return CopyA(a, b);
		}
		PE_INLINE Bool PE_CALL Copy(String* a, cString b, size_t s)
		{
			return CopyA(a, b, s);
		}
		PE_INLINE Bool PE_CALL Replace(String s, Wchar a, Wchar b)
		{
			return ReplaceA(s, a, b);
		}
		PE_INLINE String Since(String s, size_t i)
		{
			return SinceA(s, i);
		}
		PE_INLINE cString Since(cString s, size_t i)
		{
			return SinceA(s, i);
		}

		PE_INLINE Bool PE_CALL Alloc(String* s, size_t l)
		{
			return AllocA(s, l);
		}
		PE_INLINE Bool PE_CALL ReAlloc(String* s, size_t l)
		{
			return ReAllocA(s, l);
		}
		PE_INLINE Bool PE_CALL Free(String* s, size_t l)
		{
			return FreeA(s, l);
		}

		PE_INLINE size_t Len(cString s)
		{
			return LenA(s);
		}
		PE_INLINE Int32 IndexOf(cString s, Char c)
		{
			return IndexOfA(s, c);
		}
		PE_INLINE Int32 LastIndexOf(cString s, Char c)
		{
			return LastIndexOfA(s, c);
		}

		PE_INLINE Bool Contains(cString s, Char c)
		{
			return ContainsA(s, c);
		}
		PE_INLINE Bool StartsWith(cString s, Char c)
		{
			return StartsWithA(s, c);
		}
		PE_INLINE Bool Equals(cString l, cString r)
		{
			return EqualsA(l, r);
		}
		PE_INLINE Bool EndsWith(cString s, Char c)
		{
			return EndsWithA(s, c);
		}
#endif
	}
}