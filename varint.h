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
    void push(const std::bitset<8>& nowBinElm) { this->bin.push_back(UInt8(nowBinElm.to_ulong())); }

    template<typename T, UInt8 LEN>
    void init(T i)
    {
        std::bitset<LEN> bi(i);
        //find first 1 from the highest bit(real highest bit)
        short stopPos = LEN - 1;
        for (;stopPos >= 0;stopPos--)
        {
            if (bi.test(stopPos))
                break;
        }

        bool firstEnter = true;
        std::bitset<8> nowBinElm; //bin中的元素
        short nowPos = 0; //为了标识结束（最后1截止），实际数据是反着存的。前面的是高位（反应在bitset里，下标小的是高位）
        do
        {
            //push to bin
            if (firstEnter)
            {
                firstEnter = false;
            }
            else
            {
                nowBinElm.set(8 - 1); //flag bit set to 1（在这里push一定后面还有，因为即使nowPos是最后一位，这个最后一位也要在下面处理）
                this->push(nowBinElm);
                nowBinElm.reset();
            }

            //move bits to nowBinElm（nowBinElm的最高位是标志位，不在这个循环里写）
            for (short i = 6;i >= 0 && nowPos <= stopPos;i--, nowPos++)
            {
                //将bi中的位写入该元素（i从高位到低位，nowPos从低位到高位）
                nowBinElm[i] = bi[nowPos];
            }
        } while (nowPos <= stopPos);

        if (nowBinElm.any()) //nowBinElm不全为0（因为反着存，最后一位一定是1），说明还得push一下
            this->push(nowBinElm);
    }

public:
    std::vector<UInt8> bin;

    VarInt(UInt8 i) { this->init<UInt8, 8>(i); }
    VarInt(UInt16 i) { this->init<UInt16, 16>(i); }
    VarInt(UInt32 i) { this->init<UInt32, 32>(i); }
    VarInt(UInt64 i) { this->init<UInt64, 64>(i); }
    VarInt(SInt64 i)
    {
        UInt64 zipcode = abs(i) * 2;
        if (i > 0)
            zipcode--; //正数在前
        this->init<UInt64, 64>(zipcode);
    }

    SInt64 AsSInt64() const { return AsSInt64(this->bin); }
    static SInt64 AsSInt64(const std::vector<UInt8>& bin)
    {
        UInt64 zigcode = AsUInt64(bin);
        SInt64 result = 0;
        if (zigcode % 2 == 0)
            result = -SInt64(zigcode) / 2; //偶数下标是负数
        else
            result = zigcode / 2 + 1;
        return result;
    }

    UInt64 AsUInt64() const { return AsUInt64(this->bin); }
    static UInt64 AsUInt64(const std::vector<UInt8>& bin)
    {
        std::bitset<64> result;
        short nowPos = 0;
        for (const UInt8& i : bin)
        {
            std::bitset<8> bi(i);
            if (bi.test(8 - 1)) //not last block
            {
                for (short i = 6;i >= 0;i--, nowPos++)
                {
                    result[nowPos] = bi[i];
                }
            }
            else
            {
                //从低位（下标小）开始查一下最后一个1的位置
                short stopI = 0;
                for (;stopI <= 6;stopI++)
                {
                    if (bi.test(stopI))
                        break;
                }

                for (short i = 6;i >= stopI;i--, nowPos++)
                {
                    result[nowPos] = bi[i];
                }
            }
        }
        return result.to_ullong();
    }
};