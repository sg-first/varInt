#pragma once
#include <vector>
#include <bitset>
#include <cstdint>

using SInt8 = std::int8_t;
using UInt8 = std::uint8_t;
using SInt16 = std::int16_t;
using UInt16 = std::uint16_t;
using SInt32 = std::int32_t;
using UInt32 = std::uint32_t;
using SInt64 = std::int64_t;
using UInt64 = std::uint64_t;

class VarInt
{
private:
    std::vector<UInt8> bin;

    void Encode(UInt64 i)
    {
		int index = 0;
        while(i)
        {
			bin.push_back(i & 0b01111111);
			i >>= 7;
			bin.back() |= i ? 0b10000000 : 0;
        }
    }

public:
    VarInt(UInt64 i) { this->Encode(i); }
    VarInt(SInt64 i) { this->Encode(i > 0 ? (2 * i - 1) : (-2 * i)); }

    std::vector<UInt8> getBin() { return this->bin; }
    SInt64 AsSInt64() const { return AsSInt64(this->bin); }
    UInt64 AsUInt64() const { return AsUInt64(this->bin); }

    static SInt64 AsSInt64(const std::vector<UInt8>& bin, size_t startPos = 0)
    {
        UInt64 result = AsUInt64(bin, startPos);
        if (result % 2 == 0)
			return result / -2;
		else
			return result / 2 + 1;
    }

    static UInt64 AsUInt64(const std::vector<UInt8>& bin, size_t startPos = 0) { return Decode(&bin[startPos]); }

    static UInt64 Decode(const UInt8* bin)
    {
        UInt64 result = 0;

        auto isLast = [](const UInt8* i) { return (*i & 0b10000000) == 0; };

		int bit = 0;
		do
		{
			const UInt8 ri = *bin;
			result |= (static_cast<UInt64>(ri & 0b01111111) << bit);
			bit += 7;
		} while (!isLast(bin++));

		return result;
    }
};