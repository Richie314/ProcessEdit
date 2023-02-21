#pragma once
#define PE_DLLEXPORT	__declspec(dllexport)
#define PE_DLLIMPORT	__declspec(dllimport)

#include <string>
#include <cstdlib>
#ifdef PROCESSEDIT_EXPORTS
#define PE_API		PE_DLLEXPORT
#else
#define PE_API		PE_DLLIMPORT
#endif
#define PE_INLINE inline
#define PE_CALL __stdcall
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

	struct MemoryConatiner
	{
		Byte* addr = nullptr;
		ULong size = 0;
	} const DefaultMemoryStruct({ nullptr, 0Ui64 });

	template <typename Type>
	struct Array
	{
		Type* addr;
		size_t count;
		PE_INLINE Type& operator [] (size_t index)
		{
			return addr[index];
		}
		PE_INLINE const Type& operator [] (size_t index)const
		{
			return addr[index];
		}
		PE_INLINE void Allocate(size_t size)
		{
			count = size;
			addr = new Type[size];
		}
		PE_INLINE void DeAllocate()
		{
			count = 0;
			if (addr)
			{
				delete[] addr;
			}
			addr = nullptr;
		}
		PE_INLINE constexpr void CopyFrom(const Array<Type>& other)
		{
			DeAllocate();
			if (other.addr && other.count)
			{
				Allocate(other.count);
				memcpy_s(
					addr, other.count * sizeof(Type),
					other.addr, other.count * sizeof(Type));
			}
		}
		PE_INLINE void Add(const Type& elem)
		{
			Array<Type> copy;
			copy.CopyFrom(*this);
			Allocate(copy.count + 1U);
			memcpy_s(
				addr, copy.count * sizeof(Type),
				copy.addr, copy.count * sizeof(Type));
			addr[copy.count] = elem;
			copy.DeAllocate();
		}
	};
	template <typename Type>
	struct List
	{
		PE_INLINE List(): Value(nullptr), Next(nullptr) {};
		PE_INLINE List(const Type& startElem) : Value(new Type(startElem)), Next(nullptr) {};
		PE_INLINE List(const List<Type>& other): Value(nullptr), Next(nullptr)
		{
			CopyFrom(other);
		}
		PE_INLINE List<Type>& CopyFrom(const List& other)
		{
			clear();
			List<Type>* copied = (List<Type>*)malloc(sizeof(List<Type>));
			if (copied == nullptr)
				return *this;;
			copied->Value = nullptr; copied->Next = nullptr;
			const List<Type>* curr = &other;
			while (curr != nullptr && !curr->isEmpty()) {
				copied->push(*curr->Value);
				curr = curr->Next;
			}
			if (!copied->isEmpty())
			{
				Value = copied->Value;
				Next = copied->Next;
			}
			free(copied);
			return *this;
		}
		PE_INLINE ~List()
		{
			if (Value != nullptr)
				delete Value;
			if (Next != nullptr)
				delete Next;
		};
		Type* Value;
		List<Type>* Next;
		PE_INLINE List<Type>& push(const Type& elem)
		{
			if (Value == nullptr)
			{
				Value = new Type(elem);
				return *this;
			}
			if (Next != nullptr)
				return Next->push(elem);
			Next = new List<Type>(elem);
			return *this;
		}
		PE_INLINE List<Type>& deleteNext()
		{
			if (Next != nullptr)
				delete Next;
			return *this;
		}
		PE_INLINE Type* lastElem()
		{
			if (Next != nullptr)
				return Next->last();
			return Value;
		}
		PE_INLINE const Type* lastElem()const
		{
			if (Next != nullptr)
				return Next->last();
			return Value;
		}
		PE_INLINE List<Type>& clear()
		{
			if (Value != nullptr)
				delete Value;
			if (Next != nullptr)
				Next->clear();
			return *this;
		}
		PE_INLINE Bool isEmpty()const
		{
			return Value == nullptr;
		}
		PE_INLINE Bool hasNext()const
		{
			return Next != nullptr;
		}
	};
}