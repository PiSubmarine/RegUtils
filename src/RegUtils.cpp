#include "PiSubmarine/RegUtils.h"

namespace PiSubmarine
{
	uint16_t ReadUint16LE(const uint8_t* bytes, size_t Start, size_t Num)
	{
		uint16_t result = 0;
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
		return result;
	}
}