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

    void init(UInt64 i)
    {
        if (i == 0)
            return;

        UInt8* ptr = (UInt8*)&i;

        std::bitset<64> bi(i);
        //find first 1 from the highest bit(real highest bit)
        short stopPos = 63;
        for (;stopPos >= 0;stopPos--)
        {
            if (bi.test(stopPos))
                break;
        }

        auto isEnd = [&ptr, &stopPos](short nextHightestBit) { return nextHightestBit>=stopPos && ((ptr[0] & 0b10000000) == 0); }; //这个不对，还得找最高位位置

        short nextHightestBit = 7; //下一次要处理部分的最高位（下标）
        while (true)
        {
            UInt8 temp = ptr[0];
            if (isEnd(nextHightestBit)) //看到没到最后一位（如果当前位首位是1，还需要再处理一次。因此条件是检测首位为0）
            {
                //满足第二个条件首位一定是0，所以不用设flag bit
                bin.push_back(temp); //最高位放在最后
                break;
            }
            else
            {
                temp |= 0b10000000;
                this->bin.insert(this->bin.begin(), temp); //越高位下标越小
                i >>= 7; //把已处理完的7位去掉
                nextHightestBit += 7;
            }
        }
    }

public:
    VarInt(UInt64 i) { this->init(i); }
    VarInt(SInt64 i)
    {
        UInt64 zipcode = abs(i) * 2;
        if (i > 0)
            zipcode--; //正数在前
        this->init(zipcode);
    }

    std::vector<UInt8> getBin() { return this->bin; }
    SInt64 AsSInt64() const { return AsSInt64(this->bin); }
    static SInt64 AsSInt64(const std::vector<UInt8>& bin, size_t startPos = 0, size_t endPos = 0)
    {
        UInt64 zigcode = AsUInt64(bin, startPos, endPos);
        SInt64 result = 0;
        if (zigcode % 2 == 0)
            result = -SInt64(zigcode) / 2; //偶数下标是负数
        else
            result = zigcode / 2 + 1;
        return result;
    }

    UInt64 AsUInt64() const { return AsUInt64(this->bin); }
    static UInt64 AsUInt64(const std::vector<UInt8>& bin, size_t startPos = 0, size_t endPos = 0)
    {
        if (bin.empty())
            return 0;
        else if (bin.size() == 1)
            return bin[0];
        else
        {
            if (endPos == 0)
                endPos = bin.size();

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
                        if ((ri & 0b00000001) == 1) //取最高字节中的最低位，并将ptr[nowSub]最高位与其设为一致
                            ptr[nowSub - 1] |= 0b10000000;
                        ri >>= 1; //最高字节中最低位已被移走
                        
                    }
                    ptr[nowSub] = ri; //最后一字节放在最高字节 
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