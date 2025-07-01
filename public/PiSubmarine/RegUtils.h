#pragma once

#include <cstdint>
#include <array>
#include <type_traits>
#include <string.h>
#include <bit>
#include <vector>
#include <algorithm>

namespace PiSubmarine::RegUtils
{

	template<std::integral T>
	constexpr T Byteswap(T value) noexcept
	{
#if !defined(__cpp_lib_byteswap)
		static_assert(std::has_unique_object_representations_v<T>,
			"T may not have padding bits");
		auto value_representation = std::bit_cast<std::array<std::byte, sizeof(T)>>(value);
		std::ranges::reverse(value_representation);
		return std::bit_cast<T>(value_representation);
#else
		return std::byteswap(value);
#endif
	}


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
	constexpr T operator ~(T A)
	{
		using U = std::underlying_type_t<T>;
		return static_cast<T>(~static_cast<U>(A));
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
	constexpr std::make_signed_t<T> ConvertTwosComplement(T unsignedValue) requires (std::is_unsigned_v<T>())
	{
		return static_cast<std::make_signed_t<T>>(unsignedValue);
	}

	template<typename T, std::endian endianness = std::endian::native>
	T ReadInt(const uint8_t* bytes, size_t Start, size_t Num)
	{
		T result = 0;

		if constexpr (endianness == std::endian::little)
		{
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
		}
		else
		{
			size_t bytesNum = Num / 8;
			if (Num % 8)
			{
				bytesNum++;
			}

			size_t bitStart = Start % 8;
			size_t byteStart = Start / 8;
			const uint8_t* targetBytes = bytes + byteStart;

			for (size_t i = 0; i < Num; i++)
			{
				size_t bitIndex = bitStart + i;
				size_t byteIndex = bytesNum - bitIndex / 8 - 1;
				size_t bitIndexRem = bitIndex % 8;
				if ((targetBytes[byteIndex] & (1 << bitIndexRem)) != 0)
				{
					result |= (1ULL << i);
				}
			}
		}

		return result;
	}

	template<typename T, std::endian endianness = std::endian::native>
	T ReadEnum(const uint8_t* bytes, size_t Start, size_t Num)
	{
		using U = std::underlying_type_t<T>;
		U result = ReadInt<U, endianness>(bytes, Start, Num);
		return static_cast<T>(result);
	}

	template<typename T, std::endian endianness = std::endian::native>
		requires std::is_enum_v<T>
	T Read(const uint8_t* bytes, size_t Start, size_t Num)
	{
		return ReadEnum<T, endianness>(bytes, Start, Num);
	}

	template<typename T, std::endian endianness = std::endian::native>
		requires (!std::is_enum_v<T>)
	T Read(const uint8_t* bytes, size_t Start, size_t Num)
	{
		return ReadInt<T, endianness>(bytes, Start, Num);
	}

	template<typename T, std::endian endianness = std::endian::native>
	void WriteInt(T value, uint8_t* bytes, size_t Start, size_t Num)
	{
		if constexpr (endianness == std::endian::little)
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
			size_t bytesNum = Num / 8;
			if (Num % 8)
			{
				bytesNum++;
			}

			size_t bitStart = Start % 8;
			size_t byteStart = Start / 8;
			uint8_t* targetBytes = bytes + byteStart;

			for (size_t i = 0; i < Num; i++)
			{
				size_t bitIndex = bitStart + i;
				size_t byteIndex = bytesNum - bitIndex / 8 - 1;
				size_t bitIndexRem = bitIndex % 8;
				if ((value & (1ULL << i)) != 0)
				{
					targetBytes[byteIndex] |= (1 << bitIndexRem);
				}
				else
				{
					targetBytes[byteIndex] &= ~(1 << bitIndexRem);
				}
			}
		}
	}

	template<typename T, std::endian endianness = std::endian::native>
	void WriteEnum(T value, uint8_t* bytes, size_t Start, size_t Num)
	{
		using U = std::underlying_type_t<T>;
		U valueInt = static_cast<U>(value);
		WriteInt(valueInt, bytes, Start, Num);
	}

	template<typename T, std::endian endianness = std::endian::native>
		requires std::is_enum_v<T>
	void Write(T value, uint8_t* bytes, size_t Start, size_t Num)
	{
		WriteEnum<T, endianness>(value, bytes, Start, Num);
	}

	template<typename T, std::endian endianness = std::endian::native>
		requires (!std::is_enum_v<T>)
	void Write(T value, uint8_t* bytes, size_t Start, size_t Num)
	{
		WriteInt<T, endianness>(value, bytes, Start, Num);
	}

}