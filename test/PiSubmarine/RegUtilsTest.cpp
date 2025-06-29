#include <gtest/gtest.h>
#include "PiSubmarine/RegUtils.h"

namespace PiSubmarine::RegUtils
{
	TEST(ReadIntTest, Native)
	{
		std::array<uint8_t, 5> bytes{ 0b10011000, 0b10100110, 0xFF, 0xFF, 0xFF };

		uint8_t val1 = ReadInt<uint8_t>(bytes.data(), 0, 4);
		EXPECT_EQ(val1, 0b1000);

		uint8_t val2 = ReadInt<uint8_t>(bytes.data(), 1, 5);
		EXPECT_EQ(val2, 0b01100);

		uint8_t val3 = ReadInt<uint8_t>(bytes.data(), 12, 6);
		EXPECT_EQ(val3, 0b111010);
	}

	TEST(ReadIntTest, InverseByteOrder)
	{
		constexpr std::array<uint8_t, 5> bytes{ 0xFF, 0xFF, 0xCD, 0x2C, 0xFF };

		uint8_t val1 = ReadInt<uint8_t, std::endian::big>(bytes.data(), 24, 8);
		EXPECT_EQ(val1, 0x2C);

		uint8_t val2a = ReadInt<uint8_t, std::endian::big>(bytes.data(), 25, 7);
		EXPECT_EQ(val2a, 0x2C);

		uint8_t val2b = ReadInt<uint8_t, std::endian::big>(bytes.data(), 25, 7);
		EXPECT_EQ(val2b, 0x2C);

		uint16_t val3a = ReadInt<uint16_t, std::endian::big>(bytes.data(), 16, 9);
		EXPECT_EQ(val3a, 0x012C);
	}

	TEST(WriteIntTest, InverseByteOrder)
	{
		std::array<uint8_t, 5> bytes{ 0xFF, 0xFF, 0xF0, 0x00, 0xFF };
		constexpr std::array<uint8_t, 5> bytesExpected{ 0xFF, 0xFF, 0xF1, 0x2C, 0xFF };

		WriteInt<uint16_t, std::endian::big>(0x012C, bytes.data(), 16, 9);

		EXPECT_EQ(bytes, bytesExpected);
	}

	TEST(WriteIntTest, Test1)
	{
		std::array<uint8_t, 5> bytes{ 0 };

		WriteInt<uint8_t>(0b11010110, bytes.data(), 0, 4);
		EXPECT_EQ(bytes[0], 0b0110);
	}

	TEST(WriteIntTest, Test2)
	{
		std::array<uint8_t, 5> bytes{ 0 };

		WriteInt<uint8_t>(0b1101, bytes.data(), 12, 4);
		EXPECT_EQ(bytes[1], 0b11010000);
	}

	enum class ReadEnumTestFlags : uint8_t
	{
		Flag1 = (1 << 0),
		Flag2 = (1 << 1),
		Flag3 = (1 << 2),
		Flag4 = (1 << 3),
		Flag5 = (1 << 4),
	};

	TEST(ReadEnumTest, Test1)
	{
		std::array<uint8_t, 5> bytes{ 0b10011000, 0b10100110, 0xFF, 0xFF, 0xFF };

		ReadEnumTestFlags val1 = ReadEnum<ReadEnumTestFlags>(bytes.data(), 0, 4);
		EXPECT_EQ(val1, ReadEnumTestFlags::Flag4);

		ReadEnumTestFlags val2 = ReadEnum<ReadEnumTestFlags>(bytes.data(), 1, 5);
		EXPECT_EQ(val2, ReadEnumTestFlags::Flag3 | ReadEnumTestFlags::Flag4);

		ReadEnumTestFlags val3 = ReadEnum<ReadEnumTestFlags>(bytes.data(), 12, 5);
		EXPECT_EQ(val3, ReadEnumTestFlags::Flag2 | ReadEnumTestFlags::Flag4 | ReadEnumTestFlags::Flag5);
	}

	TEST(WriteEnumTest, Test1)
	{
		std::array<uint8_t, 5> bytes{ 0b10011000, 0b10100110, 0xFF, 0xFF, 0xFF };

		ReadEnumTestFlags val1 = ReadEnumTestFlags::Flag2 | ReadEnumTestFlags::Flag4 | ReadEnumTestFlags::Flag5;
		WriteEnum<ReadEnumTestFlags>(val1, bytes.data(), 12, 5);
		ReadEnumTestFlags val2 = ReadEnum<ReadEnumTestFlags>(bytes.data(), 12, 5);
		EXPECT_EQ(val1, val2);
	}

	TEST(HasAllFlagsTest, ShouldBeTrue)
	{
		ReadEnumTestFlags val1 = ReadEnumTestFlags::Flag2 | ReadEnumTestFlags::Flag4 | ReadEnumTestFlags::Flag5;
		ReadEnumTestFlags val2 = ReadEnumTestFlags::Flag4 | ReadEnumTestFlags::Flag5;
		bool Result = HasAllFlags(val1, val2);
		EXPECT_TRUE(Result);
	}

	TEST(HasAllFlagsTest, SecondHasExtraFlag)
	{
		ReadEnumTestFlags val1 = ReadEnumTestFlags::Flag2 | ReadEnumTestFlags::Flag4 | ReadEnumTestFlags::Flag5;
		ReadEnumTestFlags val2 = ReadEnumTestFlags::Flag1 | ReadEnumTestFlags::Flag4 | ReadEnumTestFlags::Flag5;
		EXPECT_FALSE(HasAllFlags(val1, val2));
	}

	TEST(HasAllFlagsTest, SecondZero)
	{
		ReadEnumTestFlags val1 = ReadEnumTestFlags::Flag2 | ReadEnumTestFlags::Flag4 | ReadEnumTestFlags::Flag5;
		ReadEnumTestFlags val2 = static_cast<ReadEnumTestFlags>(0);
		EXPECT_TRUE(HasAllFlags(val1, val2));
	}

	TEST(HasAllFlagsTest, BothZero)
	{
		ReadEnumTestFlags val1 = static_cast<ReadEnumTestFlags>(0);
		ReadEnumTestFlags val2 = static_cast<ReadEnumTestFlags>(0);
		EXPECT_TRUE(HasAllFlags(val1, val2));
	}

	TEST(HasAllFlagsTest, FirstZero)
	{
		ReadEnumTestFlags val1 = static_cast<ReadEnumTestFlags>(0);
		ReadEnumTestFlags val2 = ReadEnumTestFlags::Flag2;
		EXPECT_FALSE(HasAllFlags(val1, val2));
	}

	TEST(HasAnyFlagTest, ShouldBeTrue)
	{
		ReadEnumTestFlags val1 = ReadEnumTestFlags::Flag2 | ReadEnumTestFlags::Flag4 | ReadEnumTestFlags::Flag5;
		ReadEnumTestFlags val2 = ReadEnumTestFlags::Flag1 | ReadEnumTestFlags::Flag4 | ReadEnumTestFlags::Flag5;
		EXPECT_TRUE(HasAnyFlag(val1, val2));
	}

	TEST(HasAnyFlagTest, ShouldBeFalse)
	{
		ReadEnumTestFlags val1 = ReadEnumTestFlags::Flag2 | ReadEnumTestFlags::Flag4 | ReadEnumTestFlags::Flag5;
		ReadEnumTestFlags val2 = ReadEnumTestFlags::Flag3;
		EXPECT_FALSE(HasAnyFlag(val1, val2));
	}

	TEST(HasAnyFlagTest, FirstZero)
	{
		ReadEnumTestFlags val1 = static_cast<ReadEnumTestFlags>(0);
		ReadEnumTestFlags val2 = ReadEnumTestFlags::Flag3;
		EXPECT_FALSE(HasAnyFlag(val1, val2));
	}

	TEST(HasAnyFlagTest, SecondZero)
	{
		ReadEnumTestFlags val1 = ReadEnumTestFlags::Flag1;
		ReadEnumTestFlags val2 = static_cast<ReadEnumTestFlags>(0);
		EXPECT_TRUE(HasAnyFlag(val1, val2));
	}

	TEST(HasAnyFlagTest, BothZero)
	{
		ReadEnumTestFlags val1 = static_cast<ReadEnumTestFlags>(0);
		ReadEnumTestFlags val2 = static_cast<ReadEnumTestFlags>(0);
		EXPECT_TRUE(HasAnyFlag(val1, val2));
	}

}