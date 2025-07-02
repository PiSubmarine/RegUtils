#include "PiSubmarine/RegUtils.h"

namespace PiSubmarine
{
	uint16_t ReadUint16LE(const uint8_t* bytes, size_t Start, size_t Num)
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

	void WriteUint16LE(uint16_t value, uint8_t* bytes, size_t Start, size_t Num)
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
}