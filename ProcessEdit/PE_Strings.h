#pragma once
namespace pe
{
	using Char = char;
	using Wchar = wchar_t;
	using StringA = Char*;
	using StringW = Wchar*;
#ifdef UNICODE
	using String = StringW;
#else
	using String = StringA;
#endif
	namespace str
	{

		PE_API Bool PE_CALL CatA(StringA*, const StringA);
		PE_API Bool PE_CALL CopyA(StringA*, const StringA);

		PE_API Bool PE_CALL AllocA(StringA*, size_t);
		PE_API Bool PE_CALL ReAllocA(StringA*, size_t);
		PE_API Bool PE_CALL FreeA(StringA*, size_t);

		PE_API size_t LenA(const StringA);
		PE_API Int IndexOfA(const StringA, Char);
		PE_API Int LastIndexOfA(const StringA, Char);

		PE_API Bool ContainsA(const StringA, Char);
		PE_API Bool StartsWithA(const StringA, Char);
		PE_API Bool EqualsA(const StringA, const StringA);
		PE_API Bool EndsWithA(const StringA, Char);


		PE_API Bool PE_CALL CatW(StringW*, const StringW);
		PE_API Bool PE_CALL AllocW(StringW*, size_t);
		PE_API Bool PE_CALL ReAllocW(StringW*, size_t);
		PE_API Bool PE_CALL FreeW(StringW*, size_t);
		PE_API size_t LenW(const StringW);
		PE_API Bool ContainsW(const StringW, Wchar);
		PE_API Bool PE_CALL CopyW(StringW*, const StringW);
		PE_API Int IndexOfW(const StringW, Wchar);
		PE_API Bool StartsWithW(const StringW, Wchar);
		PE_API Bool EqualsW(const StringW, const StringW);
		PE_API Bool EndsWithW(const StringW, Wchar);


#ifdef UNICODE
		constexpr PE_INLINE Bool PE_CALL Cat(String* left, const String right)
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
		constexpr PE_INLINE size_t Len(const String s)
		{
			return LenW(s);
		}
		constexpr PE_INLINE Bool Contains(const String s, Wchar w)
		{
			return ContainsW(s, w);
		}
		constexpr PE_INLINE Bool PE_CALL Copy(String* block, const String s)
		{
			return CopyW(block, s);
		}
		constexpr PE_INLINE Int IndexOf(const String s, Wchar w)
		{
			return IndexOfW(s, w);
		}
		constexpr PE_INLINE Bool StartsWith(const String s, Wchar w)
		{
			return StartsWithW(s, w);
		}
		constexpr PE_INLINE Bool Equals(const String l, const String r)
		{
			return EqualsW(l, r);
		}
		constexpr PE_INLINE Bool EndsWith(const String s, Wchar w)
		{
			return EndsWithW(s, w);
		}
	}
#else
		constexpr PE_INLINE Bool PE_CALL Cat(String* left, const String right)
		{
			return CatA(left, right);
		}
		constexpr PE_INLINE Bool PE_CALL Alloc(String* block, size_t size)
		{
			return AllocA(block, size);
		}

		constexpr PE_INLINE Bool PE_CALL ReAlloc(String* block, size_t size)
		{
			return ReAllocA(block, size);
		}
		constexpr PE_INLINE Bool PE_CALL Free(String* block, size_t size)
		{
			return FreeA(block, size);
		}
		constexpr PE_INLINE size_t Len(const String s)
		{
			return LenA(s);
		}
		constexpr PE_INLINE Bool Contains(const String s, Char c)
		{
			return ContainsA(s, c);
		}
		constexpr PE_INLINE Bool PE_CALL Copy(String* block, const String s)
		{
			return CopyA(block, s);
		}
		constexpr PE_INLINE Int IndexOf(const String s, Char c)
		{
			return IndexOfA(s, c);
		}
		constexpr PE_INLINE Bool StartsWith(const String s, Char c)
		{
			return StartsWithA(s, c);
		}
		constexpr PE_INLINE Bool Equals(const String l, const String r)
		{
			return EqualsA(l, r);
		}
		constexpr PE_INLINE Bool EndsWith(const String s, Char c)
		{
			return EndsWithA(s, c);
		}
#endif
}