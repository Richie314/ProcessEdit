#include "pch.h"
#include "PE_Header.h"
using namespace pe;
StringA str::WtoA(cStringW s)
{
	if (!s)
		return nullptr;
	size_t len = str::LenW(s) + 1U;//Copy also the \0 final character
	StringA sA = nullptr;
	str::AllocA(&sA, len);
	if (sA)
		for (size_t i = 0; i < len; ++i)
		{
			sA[i] = static_cast<Char>(s[i]);
		}
	return sA;
}
StringW str::AtoW(cStringA s)
{
	if (!s)
		return nullptr;
	size_t len = str::LenA(s) + 1U;//Copy also the \0 final character
	StringW sW = nullptr;
	str::AllocW(&sW, len);
	if (sW)
		for (size_t i = 0; i < len; ++i)
		{
			sW[i] = static_cast<Wchar>(s[i]);
		}
	return sW;
}