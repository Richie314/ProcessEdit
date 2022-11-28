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
//same as the one from Microsoft, winnit.h
#define PE_DECLARE_HANDLE(Name) struct Name##__{ int unused;  }; typedef struct Name##__ *Name

#define PE_NULL(t) (static_cast<t*>(nullptr))
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
		Byte* addr = PE_NULL(Byte);
		ULong size = 0;
	} const DefaultMemoryStruct({ PE_NULL(Byte), 0Ui64 });

	template <class Type>
	struct Array
	{
		Type* addr;
		size_t count;
		PE_INLINE constexpr Type& operator [] (size_t index)
		{
			return addr[index];
		}
		PE_INLINE constexpr const Type& operator [] (size_t index)const
		{
			return addr[index];
		}
		PE_INLINE constexpr void Allocate(size_t size)
		{
			count = size;
			addr = new Type[size];
		}
		PE_INLINE constexpr void DeAllocate()
		{
			count = 0;
			if (addr)
			{
				delete[] addr;
			}
			addr = PE_NULL(Type);
		}
		PE_INLINE constexpr void CopyFrom(const Array<Type>& other)
		{
			DeAllocate();
			if (other.addr && other.count)
			{
				Allocate(other.count);
				for (size_t i = 0; i < count; i++)
				{
					addr[i] = other.addr[i];
				}
			}
		}
	};
}