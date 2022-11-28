#include "pch.h"
#include "PE_Header.h"
using namespace pe;

//ANSI versions

Bool PE_CALL str::AllocW(StringW* ptr, size_t len)
{
	len *= sizeof(Wchar);
	if (ptr) {
		HGLOBAL hMem = GlobalAlloc(GPTR, len);
		if (hMem)
		{
			*ptr = (StringW)hMem;
			return len == GlobalSize(hMem);
		}
	}
	return false;
}
Bool PE_CALL str::ReAllocW(StringW* ptr, size_t len)
{
	len *= sizeof(Wchar);
	if (ptr) {
		StringW addr = *ptr;
		HGLOBAL hMem = GlobalReAlloc((HGLOBAL)addr, len, GMEM_ZEROINIT);
		if (hMem)
		{
			addr = (StringW)hMem;
			return len == GlobalSize(hMem);
		}
	}
	return false;
}
Bool PE_CALL str::FreeW(StringW* ptr, size_t len)
{
	len *= sizeof(Wchar);
	if (ptr) {
		StringW addr = *ptr;
		if (GlobalFree((HGLOBAL)addr) == NULL)
		{
			addr = NULL;
			return true;
		}
	}
	return false;
}
size_t str::LenW(cStringW s)
{
	return lstrlenW(s);
}
Int32 str::IndexOfW(cStringW s, Wchar w)
{
	//Faster than a for cicle
	if (s)
	{
		cStringW sOrig = s;
		while (*s)
		{
			if (*s == w)
			{
				return s - sOrig;
			}
			s += sizeof(Wchar);
		}
	}
	return EOF;
}
Int32 str::LastIndexOfW(cStringW s, Wchar w)
{
	Int32 len = (Int32)str::LenW(s) - 1;
	for (Int32 i = len; i >= 0; --i)
	{
		if (s[i] == w)
		{
			return i;
		}
	}
	return EOF;
}

Bool str::ContainsW(cStringW s, Wchar w)
{
	//Faster than a for cicle
	if (s)
	{
		while (*s)
		{
			if (*s == w)
			{
				return true;
			}
			s += sizeof(Wchar);
		}
	}
	return false;
}
Bool str::StartsWithW(cStringW s, Wchar w)
{
	//Faster than a for cicle
	if (s && *s == w)
	{
		return true;
	}
	return false;
}
Bool str::EqualsW(cStringW l, cStringW r)
{
	size_t len = str::LenW(l);
	if (len != str::LenW(r))
		return false;
	for (size_t i = 0; i < len; ++i)
	{
		if (l[i] != r[i])
			return false;
	}
	return true;
}
Bool str::EndsWithW(cStringW s, Wchar w)
{
	size_t len = str::LenW(s);
	return len && s[len - 1] == w;
}

Bool PE_CALL str::CatW(StringW destination, cStringW source)
{
	return (Bool)lstrcatW(destination, source);
}
Bool PE_CALL str::CopyW(StringW* destination, cStringW source)
{
	if (!destination)
		return false;
	size_t len = str::LenW(source);
	if (len) {
		if (str::AllocW(destination, len))
		{
			return (Bool)lstrcpyW(*destination, source);
		}
	}
	return false;
}
Bool PE_CALL str::CopyW(StringW* destination, cStringW source, size_t maxLenToCopy)
{
	if (!destination)
		return false;
	size_t len = str::LenW(source);
	len = min(len, maxLenToCopy);
	if (len) {
		if (str::AllocW(destination, len))
		{
			return (Bool)lstrcpyW(*destination, source);
		}
	}
	return false;
}
Bool PE_CALL str::ReplaceW(StringW s, Wchar toReplace, Wchar with)
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
			s += sizeof(Wchar);
		}
	}
	return bReplaced;
}
StringW str::SinceW(StringW s, size_t start)
{
	return &s[start];
}
cStringW str::SinceW(cStringW s, size_t start)
{
	if (s)
		return &s[start];
	return NULL;
}