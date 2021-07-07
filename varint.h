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
    static SInt64 AsSInt64(const std::vector<UInt8>& bin, size_t startPos = 0, size_t endPos = 0)
    {
        UInt64 result = AsUInt64(bin, startPos, endPos);
        if (result % 2 == 0)
			return result / -2;
		else
			return result / 2 + 1;
    }

    UInt64 AsUInt64() const { return AsUInt64(this->bin); }
    static UInt64 AsUInt64(const std::vector<UInt8>& bin, size_t startPos = 0, size_t endPos = 0)
    {
        if (endPos == 0)
            endPos = bin.size();

        if (startPos == endPos)
            return 0;
        else if (endPos - startPos == 1)
            return bin[startPos];
        else
        {
            UInt64 result = 0;
            UInt8* ptr = (UInt8*)&result;
            short nowSub = 0; //小的是低字节

            auto isLast = [](const UInt8& i) { return (i & 0b10000000) == 0; };

            for (size_t i = startPos;i < endPos;i++)
            {
                UInt8 ri = bin[i];
                if (isLast(ri))
                {
                    if (nowSub >= 2) //消除左边多出来的0
                    {
                        short zeroNum = nowSub - 1; //当前最高位左边0的个数（要取多少位）
                        std::bitset<8> beop(ptr[nowSub - 1]); //准备操作当前result最高字节（中的高几位）
                        std::bitset<8> op(ri); //取bin最高字节（中的第几位）
                        for (short i = 0;i < zeroNum;i++)
                        {
                            beop[8 - zeroNum + i] = op[i];
                        }
                        ptr[nowSub - 1] = beop.to_ulong();
                        ri >>= zeroNum; //已经被放置的位移走
                    }
                    ptr[nowSub] = ri; //bin的最后一字节放在最高字节 
                    result >>= 1; //消除else中标志位造成的误差（右边多出来的0）
                    break;
                }
                else
                {
                    result <<= 7; //腾地方（nowSub=1后再进行，左边会有一个多出来的0）
                    //把新的放进来
                    ri <<= 1; //remove flag bit（这步结束之后右边有个多出来的0）
                    ptr[0] = ri; //现在的放到最低字节（下标越大的是越低位）
                    
                    nowSub++; //现在的最高字节到哪了
                }
            }
            
            return result;
        }
    }
};