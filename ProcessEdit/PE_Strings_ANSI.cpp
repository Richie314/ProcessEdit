#include "pch.h"
#include "PE_Header.h"
using namespace pe;

//ANSI versions

Bool PE_CALL str::AllocA(StringA* ptr, size_t len)
{
	if (ptr) {
		HGLOBAL hMem = GlobalAlloc(GPTR, len);
		if (hMem)
		{
			*ptr = (StringA)hMem;
			return len == GlobalSize(hMem);
		}
	}
	return false;
}
Bool PE_CALL str::ReAllocA(StringA* ptr, size_t len)
{
	if (ptr) {
		StringA addr = *ptr;
		HGLOBAL hMem = GlobalReAlloc((HGLOBAL)addr, len, GMEM_ZEROINIT);
		if (hMem)
		{
			addr = (StringA)hMem;
			return len == GlobalSize(hMem);
		}
	}
	return false;
}
Bool PE_CALL str::FreeA(StringA* ptr, size_t len)
{
	if (ptr) {
		StringA addr = *ptr;
		if (GlobalFree((HGLOBAL)addr) == NULL)
		{
			addr = NULL;
			return true;
		}
	}
	return false;
}
size_t str::LenA(cStringA s)
{
	return lstrlenA(s);
}
Int32 str::IndexOfA(cStringA s, Char c)
{
	//Faster than a for cicle
	if (s)
	{
		cStringA sOrig = s;
		while (*s)
		{
			if (*s == c)
			{
				return s - sOrig;
			}
			++s;
		}
	}
	return EOF;
}
Int32 str::LastIndexOfA(cStringA s, Char c)
{
	Int32 len = (Int32)str::LenA(s) - 1;
	for (Int32 i = len; i >= 0; --i)
	{
		if (s[i] == c)
		{
			return i;
		}
	}
	return EOF;
}

Bool str::ContainsA(cStringA s, Char c)
{
	//Faster than a for cicle
	if (s)
	{
		while (*s)
		{
			if (*s == c)
			{
				return true;
			}
			++s;
		}
	}
	return false;
}
Bool str::StartsWithA(cStringA s, Char c)
{
	//Faster than a for cicle
	if (s && *s == c)
	{
		return true;
	}
	return false;
}
Bool str::EqualsA(cStringA l, cStringA r)
{
	size_t len = str::LenA(l);
	if (len != str::LenA(r))
		return false;
	for (size_t i = 0; i < len; ++i)
	{
		if (l[i] != r[i])
			return false;
	}
	return true;
}
Bool str::EndsWithA(cStringA s, Char c)
{
	size_t len = str::LenA(s);
	return len && s[len - 1] == c;
}

Bool PE_CALL str::CatA(StringA destination, cStringA source)
{
	return (Bool)lstrcatA(destination, source);
}
Bool PE_CALL str::CopyA(StringA* destination, cStringA source)
{
	if (!destination)
		return false;
	size_t len = str::LenA(source);
	if (len) {
		if (str::AllocA(destination, len))
		{
			return (Bool)lstrcpyA(*destination, source);
		}
	}
	return false;
}
Bool PE_CALL str::CopyA(StringA* destination, cStringA source, size_t maxLenToCopy)
{
	if (!destination)
		return false;
	size_t len = str::LenA(source);
	len = min(len, maxLenToCopy);
	if (len) {
		if (str::AllocA(destination, len))
		{
			return (Bool)lstrcpyA(*destination, source);
		}
	}
	return false;
}
Bool PE_CALL str::ReplaceA(StringA s, Char toReplace, Char with)
{
	Bool bReplaced = false;
	if (s)
	{
		while (*s)
		{
			if (*s == toReplace)
			{
				*s = with;
				bReplaced = true;
			}
			++s;
		}
	}
	return bReplaced;
}
StringA str::SinceA(StringA s, size_t start)
{
	return s + start;
}
cStringA str::SinceA(cStringA s, size_t start)
{
	if (s)
		return s + start;
	return NULL;
}