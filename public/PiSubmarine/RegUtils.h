#pragma once

#include <cstdint>
#include <array>
#include <type_traits>
#include <string.h>
#include <bit>
#include <vector>

namespace PiSubmarine::RegUtils
{
	template<typename T>
	constexpr std::underlying_type_t<T> ToInt(T e)
	{
		return static_cast<std::underlying_type_t<T>>(e);
	}

	template<typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
	constexpr T operator |(T A, T B)
	{
		using U = std::underlying_type_t<T>;
		return static_cast<T>(static_cast<U>(A) | static_cast<U>(B));
	}

	template<typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
	constexpr T operator &(T A, T B)
	{
		using U = std::underlying_type_t<T>;
		return static_cast<T>(static_cast<U>(A) & static_cast<U>(B));
	}

	template<typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
	constexpr bool HasAllFlags(T A, T B)
	{
		return (A & B) == B;
	}

	template<typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
	constexpr bool HasAnyFlag(T A, T B)
	{
		if (B == static_cast<T>(0))
		{
			return true;
		}
		return (A & B) != static_cast<T>(0);
	}

	template<typename T>
	constexpr std::make_signed_t<T> ConvertTwosComplement(T unsignedValue)
	{
		if (unsignedValue & (1 << (sizeof(T) * 8 - 1)))
		{
			unsignedValue |= (~0u << sizeof(T) * 8);
		}
		return static_cast<std::make_signed_t<T>>(unsignedValue);
	}


	template<typename T, size_t BytesNum>
	constexpr T ReadIntReversed(const std::array<uint8_t, BytesNum>& bytes, size_t Start, size_t Num)
	{
		T result = 0;
		for (size_t i = 0; i < Num; i++)
		{
			size_t bitIndex = Start + i;
			size_t byteIndex = bitIndex / 8;
			size_t bitIndexRem = bitIndex % 8;
			if ((bytes[byteIndex] & (1 << bitIndexRem)) != 0)
			{
				result |= (1ULL << (Num - i - 1));
			}
		}
		return result;
	}

	template<typename T, size_t BytesNum, std::endian endianness = std::endian::native>
	constexpr T ReadInt(const std::array<uint8_t, BytesNum>& bytes, size_t Start, size_t Num)
	{
		if constexpr (endianness == std::endian::native)
		{
			T result = 0;
			for (size_t i = 0; i < Num; i++)
			{
				size_t bitIndex = Start + i;
				size_t byteIndex = bitIndex / 8;
				size_t bitIndexRem = bitIndex % 8;
				if ((bytes[byteIndex] & (1 << bitIndexRem)) != 0)
				{
					result |= (1ULL << i);
				}
			}
			return result;
		}
		else
		{
			return ReadIntReversed<T, BytesNum>(bytes, Start, Num);
		}
	}

	template<typename T, size_t BytesNum, std::endian endianness = std::endian::native>
	constexpr T ReadEnum(const std::array<uint8_t, BytesNum>& bytes, size_t Start, size_t Num)
	{
		using U = std::underlying_type_t<T>;
		U result = ReadInt<U, BytesNum, endianness>(bytes, Start, Num);
		return static_cast<T>(result);
	}

	template<typename T, size_t BytesNum, std::endian endianness = std::endian::native>
	constexpr typename std::enable_if<std::is_enum<T>::value, T>::type
		Read(const std::array<uint8_t, BytesNum>& bytes, size_t Start, size_t Num)
	{
		return ReadEnum<T, BytesNum, endianness>(bytes, Start, Num);
	}

	template<typename T, size_t BytesNum, std::endian endianness = std::endian::native>
	constexpr typename std::enable_if<!std::is_enum<T>::value, T>::type
		Read(const std::array<uint8_t, BytesNum>& bytes, size_t Start, size_t Num)
	{
		return ReadInt<T, BytesNum, endianness>(bytes, Start, Num);
	}

	template<size_t BytesOutNum, size_t ByteOffset, size_t BytesInNum>
	constexpr std::array<uint8_t, BytesOutNum> ReadBytes(const std::array<uint8_t, BytesInNum>& bytes)
	{
		static_assert(BytesOutNum + ByteOffset <= BytesInNum);

		std::array<uint8_t, BytesOutNum> result{ 0 };
		for (size_t i = 0; i < BytesOutNum; i++)
		{
			result[i] = bytes[ByteOffset + i];
		}
		return result;
	}

	template<size_t BytesInNum>
	std::vector<uint8_t> ReadBytes(const std::array<uint8_t, BytesInNum>& bytes, size_t ByteOffset, size_t BytesOutNum)
	{
		static_assert(BytesOutNum + ByteOffset <= BytesInNum);

		std::vector<uint8_t> result;
		result.resize(BytesOutNum);
		for (size_t i = 0; i < BytesOutNum; i++)
		{
			result[i] = bytes[ByteOffset + i];
		}
		return result;
	}

	template<size_t BytesWriteNum, size_t BytesBufferNum>
	constexpr void WriteBytes(const std::array<uint8_t, BytesWriteNum>& bytesIn, std::array<uint8_t, BytesBufferNum>& bytes, size_t ByteOffset)
	{
		for (size_t i = 0; i < BytesWriteNum; i++)
		{
			bytes[ByteOffset + i] = bytesIn[i];
		}
	}

	template<typename T, size_t BytesNum, std::endian endianness = std::endian::native>
	constexpr void WriteInt(T value, std::array<uint8_t, BytesNum>& bytes, size_t Start, size_t Num)
	{
		if constexpr (endianness == std::endian::native)
		{
			for (size_t i = 0; i < Num; i++)
			{
				size_t bitIndex = Start + i;
				size_t byteIndex = bitIndex / 8;
				size_t bitIndexRem = bitIndex % 8;
				if ((value & (1ULL << i)) != 0)
				{
					bytes[byteIndex] |= (1 << bitIndexRem);
				}
				else
				{
					bytes[byteIndex] &= ~(1 << bitIndexRem);
				}
			}
		}
		else
		{

		}
	}

	template<typename T, size_t BytesNum>
	constexpr void WriteIntReversed(T value, std::array<uint8_t, BytesNum>& bytes, size_t Start, size_t Num)
	{
		for (size_t i = 0; i < Num; i++)
		{
			size_t bitIndex = Start + i;
			size_t byteIndex = bitIndex / 8;
			size_t bitIndexRem = bitIndex % 8;
			if ((value & (1ULL << i)) != 0)
			{
				bytes[Num - byteIndex - 1] |= (1 << bitIndexRem);
			}
			else
			{
				bytes[Num - byteIndex - 1] &= ~(1 << bitIndexRem);
			}
		}
	}

	template<typename T, size_t BytesNum, std::endian endianness = std::endian::native>
	constexpr void WriteEnum(T value, std::array<uint8_t, BytesNum>& bytes, size_t Start, size_t Num)
	{
		using U = std::underlying_type_t<T>;
		U valueInt = static_cast<U>(value);
		WriteInt(valueInt, bytes, Start, Num);
	}

	template<typename T, size_t BytesNum, std::endian endianness = std::endian::native>
	constexpr typename std::enable_if<std::is_enum<T>::value, void>::type
		Write(T value, std::array<uint8_t, BytesNum>& bytes, size_t Start, size_t Num)
	{
		WriteEnum<T, BytesNum, endianness>(value, bytes, Start, Num);
	}

	template<typename T, size_t BytesNum, std::endian endianness = std::endian::native>
	constexpr typename std::enable_if<!std::is_enum<T>::value, void>::type
		Write(T value, std::array<uint8_t, BytesNum>& bytes, size_t Start, size_t Num)
	{
		WriteInt<T, BytesNum, endianness>(value, bytes, Start, Num);
	}

	template<typename TReg, typename TContainer>
	TReg ConstructRegister(const TContainer& data)
	{
		if (data.size() != TReg::GetSize())
		{
			throw std::exception();
		}
		std::array<uint8_t, TReg::GetSize()> arr{ 0 };
		memcpy(arr.data(), data.data(), arr.size());
		return TReg(arr);
	}

	template<typename TReg, size_t ArraySize>
	static void ReadRegister(TReg& reg, const std::array<uint8_t, ArraySize>& byteArray)
	{
		memcpy(reg.GetRegisterByteArray().data(), byteArray.data() + reg.GetOffset(), reg.GetSize());
	}

	template<auto Offset, size_t Size>
	struct Register
	{
		constexpr static size_t GetOffset()
		{
			return static_cast<size_t>(Offset);
		}

		constexpr static size_t GetSize()
		{
			return Size;
		}

		constexpr Register()
		{

		}

		constexpr Register(const std::array<uint8_t, Size>& data) : Data(data)
		{

		}

		constexpr Register(const Register<Offset, Size>& other) : Data(other.Data)
		{

		}

		constexpr auto GetOffsetAndData() const
		{
			constexpr size_t offsetInt = static_cast<size_t>(Offset);
			std::array<uint8_t, sizeof(Offset) + Size> bytes{ 0 };

			if constexpr (std::endian::native == std::endian::little)
			{
				for (size_t i = 0; i < sizeof(Offset); i++)
				{
					bytes[sizeof(Offset) - i - 1] = offsetInt >> (i * 8);
				}
			}
			else
			{
				for (size_t i = 0; i < sizeof(Offset); i++)
				{
					bytes[i] = offsetInt >> (i * 8);
				}
			}
			WriteBytes(Data, bytes, sizeof(Offset));

			return bytes;
		}

		constexpr const std::array<uint8_t, Size>& GetRegisterByteArray() const
		{
			return Data;
		}

		constexpr std::array<uint8_t, Size>& GetRegisterByteArray()
		{
			return Data;
		}

		void Clear()
		{
			Data = { 0 };
		}

	protected:
		std::array<uint8_t, Size> Data{ 0 };
	};

	enum class Access : uint8_t
	{
		Read = 0,
		ReadWrite = 1
	};

	template<typename TFieldType, size_t RegisterSize, std::endian endianness = std::endian::native>
	struct FieldBase
	{
		FieldBase(std::array<uint8_t, RegisterSize>& data) :
			RegisterData(data)
		{

		}

		using FieldType = TFieldType;

	protected:
		std::array<uint8_t, RegisterSize>& RegisterData;
	};

	template<typename TFieldType, size_t Offset, size_t BitLength, size_t RegisterSize, std::endian endianness = std::endian::native>
	struct FieldReadable : public FieldBase<TFieldType, RegisterSize, endianness>
	{
		FieldReadable(std::array<uint8_t, RegisterSize>& data) :
			FieldBase<TFieldType, RegisterSize>(data)
		{

		}

		constexpr TFieldType Get() const
		{
			return Read<TFieldType>(FieldBase<TFieldType, RegisterSize, endianness>::RegisterData, Offset, BitLength);
		}

		constexpr operator TFieldType() const
		{
			return Get();
		}
	};

	template<typename TFieldType, size_t Offset, size_t BitLength, size_t RegisterSize, std::endian endianness = std::endian::native>
	struct FieldWritable : public FieldReadable<TFieldType, Offset, BitLength, RegisterSize, endianness>
	{
		FieldWritable(std::array<uint8_t, RegisterSize>& data) :
			FieldReadable<TFieldType, Offset, BitLength, RegisterSize>(data)
		{

		}

		void Set(TFieldType value)
		{
			Write<TFieldType, endianness>(value, FieldBase<TFieldType, RegisterSize>::RegisterData, Offset, BitLength);
		}

		constexpr operator TFieldType() const
		{
			return FieldReadable<TFieldType, Offset, BitLength, RegisterSize>::Get();
		}

		FieldWritable& operator=(const TFieldType& value)
		{
			Set(value);
			return *this;
		}
	};

	template<typename TFieldType, size_t Offset, size_t BitLength, Access Access, size_t RegisterSize>
	struct Field :
		public std::conditional<
		HasAnyFlag(Access, Access::ReadWrite),
		FieldWritable<TFieldType, Offset, BitLength, RegisterSize>,
		FieldReadable<TFieldType, Offset, BitLength, RegisterSize>
		>::type
	{
		constexpr Field(std::array<uint8_t, RegisterSize>& data) :
			std::conditional<
			HasAnyFlag(Access, Access::ReadWrite),
			FieldWritable<TFieldType, Offset, BitLength, RegisterSize>,
			FieldReadable<TFieldType, Offset, BitLength, RegisterSize>
			>::type(data)
		{

		}


	};
}