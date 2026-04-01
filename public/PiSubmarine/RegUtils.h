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
    template <std::integral T>
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


    template <typename T>
    constexpr std::underlying_type_t<T> ToInt(T e)
    {
        return static_cast<std::underlying_type_t<T>>(e);
    }

#if 0
    // Unsafe due to possible narrowing conversions
    template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>, typename I, typename = std::enable_if_t<
                  std::is_integral_v<I>>>
    [[deprecated]] constexpr bool operator ==(T A, I B)
    {
        using U = std::underlying_type_t<T>;
        return static_cast<U>(A) == static_cast<U>(B);
    }
#else

    template <typename T>
    constexpr bool operator ==(T A, std::underlying_type_t<T> B)
    {
        using U = std::underlying_type_t<T>;
        return static_cast<U>(A) == static_cast<U>(B);
    }
#endif

    template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
    constexpr T operator |(T A, T B)
    {
        using U = std::underlying_type_t<T>;
        return static_cast<T>(static_cast<U>(A) | static_cast<U>(B));
    }

    template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
    constexpr T operator &(T A, T B)
    {
        using U = std::underlying_type_t<T>;
        return static_cast<T>(static_cast<U>(A) & static_cast<U>(B));
    }

    template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
    constexpr T operator ~(T A)
    {
        using U = std::underlying_type_t<T>;
        return static_cast<T>(~static_cast<U>(A));
    }

    template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
    constexpr bool HasAllFlags(T A, T B)
    {
        return (A & B) == B;
    }

    template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
    constexpr bool HasAnyFlag(T A, T B)
    {
        if (B == static_cast<T>(0))
        {
            return true;
        }
        return (A & B) != static_cast<T>(0);
    }

    template <typename T, std::endian endianness = std::endian::native>
    T ReadInt(const uint8_t* bytes, size_t Start, size_t Num)
    {
        // Early exit for 0 bits requested
        if (Num == 0) return 0;

        T result = 0;

        if constexpr (endianness == std::endian::little)
        {
            size_t byteIdx = Start / 8;
            size_t bitOffset = Start % 8;
            size_t bitsRead = 0;

            while (bitsRead < Num)
            {
                // Read up to 8 bits, bounded by remaining bits in the byte or total Num
                size_t bitsToRead = std::min<size_t>(8 - bitOffset, Num - bitsRead);

                // Masking using 1ULL prevents undefined behavior if bitsToRead is 8
                T mask = static_cast<T>((1ULL << bitsToRead) - 1);

                T byteVal = (bytes[byteIdx] >> bitOffset) & mask;
                result |= (byteVal << bitsRead);

                bitsRead += bitsToRead;
                byteIdx++;
                bitOffset = 0; // Read from bit 0 after the first byte
            }
        }
        else
        {
            // Big-endian processing mapping your conceptual virtual word
            size_t bitStart = Start % 8;
            size_t byteStart = Start / 8;
            size_t bytesNum = Num / 8 + (Num % 8 != 0 ? 1 : 0);

            size_t bitIndex = bitStart;
            size_t bitsRead = 0;

            while (bitsRead < Num)
            {
                size_t chunk = bitIndex / 8;
                size_t bitIndexRem = bitIndex % 8;

                // Limit chunk reads to byte boundaries or remaining requested bits
                size_t bitsToRead = std::min<size_t>(8 - bitIndexRem, Num - bitsRead);

                // Replicate the original code's inverted byte-indexing logic:
                // This safely navigates backwards across the conceptual bytesNum boundary
                size_t actual_byte = byteStart + bytesNum - chunk - 1;

                T mask = static_cast<T>((1ULL << bitsToRead) - 1);
                T byteVal = (bytes[actual_byte] >> bitIndexRem) & mask;

                result |= (byteVal << bitsRead);

                bitsRead += bitsToRead;
                bitIndex += bitsToRead;
            }
        }

        return result;
    }

    template <typename T, std::endian endianness = std::endian::native>
    T ReadEnum(const uint8_t* bytes, size_t Start, size_t Num)
    {
        using U = std::underlying_type_t<T>;
        U result = ReadInt<U, endianness>(bytes, Start, Num);
        return static_cast<T>(result);
    }

    template <typename T, std::endian endianness = std::endian::native>
        requires std::is_enum_v<T>
    T Read(const uint8_t* bytes, size_t Start, size_t Num)
    {
        return ReadEnum<T, endianness>(bytes, Start, Num);
    }

    template <typename T, std::endian endianness = std::endian::native>
        requires (!std::is_enum_v<T>)
    T Read(const uint8_t* bytes, size_t Start, size_t Num)
    {
        return ReadInt<T, endianness>(bytes, Start, Num);
    }

    template <typename T, std::endian endianness = std::endian::native>
    void WriteInt(T value, uint8_t* bytes, size_t Start, size_t Num)
    {
        // Early exit for 0 bits requested
        if (Num == 0) return;

        // Cast to uint64_t to prevent undefined behavior when right-shifting.
        // (e.g., if T is uint8_t and we eventually shift by >= 8 bits)
        auto safeValue = static_cast<uint64_t>(value);

        if constexpr (endianness == std::endian::little)
        {
            size_t byteIdx = Start / 8;
            size_t bitOffset = Start % 8;
            size_t bitsWritten = 0;

            while (bitsWritten < Num)
            {
                size_t bitsToWrite = std::min<size_t>(8 - bitOffset, Num - bitsWritten);

                auto mask = static_cast<uint8_t>((1ULL << bitsToWrite) - 1);

                // Extract the relevant bits from the value
                auto valChunk = static_cast<uint8_t>((safeValue >> bitsWritten) & mask);

                // Clear the target bits in the byte, then OR the new ones
                bytes[byteIdx] &= ~(mask << bitOffset);
                bytes[byteIdx] |= (valChunk << bitOffset);

                bitsWritten += bitsToWrite;
                byteIdx++;
                bitOffset = 0; // Write starting from bit 0 in subsequent bytes
            }
        }
        else
        {
            // Big-endian mapping matching the ReadInt logic
            size_t bitStart = Start % 8;
            size_t byteStart = Start / 8;
            size_t bytesNum = Num / 8 + (Num % 8 != 0 ? 1 : 0);

            size_t bitIndex = bitStart;
            size_t bitsWritten = 0;

            while (bitsWritten < Num)
            {
                size_t chunk = bitIndex / 8;
                size_t bitIndexRem = bitIndex % 8;

                size_t bitsToWrite = std::min<size_t>(8 - bitIndexRem, Num - bitsWritten);

                // Identical backward-stepping byte index calculation to ReadInt
                size_t actual_byte = byteStart + bytesNum - chunk - 1;

                auto mask = static_cast<uint8_t>((1ULL << bitsToWrite) - 1);
                auto valChunk = static_cast<uint8_t>((safeValue >> bitsWritten) & mask);

                // Clear the target bits in the byte, then OR the new ones
                bytes[actual_byte] &= ~(mask << bitIndexRem);
                bytes[actual_byte] |= (valChunk << bitIndexRem);

                bitsWritten += bitsToWrite;
                bitIndex += bitsToWrite;
            }
        }
    }

    template <typename T, std::endian endianness = std::endian::native>
    void WriteEnum(T value, uint8_t* bytes, size_t Start, size_t Num)
    {
        using U = std::underlying_type_t<T>;
        U valueInt = static_cast<U>(value);
        WriteInt(valueInt, bytes, Start, Num);
    }

    template <typename T, std::endian endianness = std::endian::native>
        requires std::is_enum_v<T>
    void Write(T value, uint8_t* bytes, size_t Start, size_t Num)
    {
        WriteEnum<T, endianness>(value, bytes, Start, Num);
    }

    template <typename T, std::endian endianness = std::endian::native>
        requires (!std::is_enum_v<T>)
    void Write(T value, uint8_t* bytes, size_t Start, size_t Num)
    {
        WriteInt<T, endianness>(value, bytes, Start, Num);
    }
}
