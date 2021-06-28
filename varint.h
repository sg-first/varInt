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
        std::bitset<8> nowBinElm; //bin�е�Ԫ��
        short nowPos = 0; //Ϊ�˱�ʶ���������1��ֹ����ʵ�������Ƿ��Ŵ�ġ�ǰ����Ǹ�λ����Ӧ��bitset��±�С���Ǹ�λ��
        do
        {
            //push to bin
            if (firstEnter)
            {
                firstEnter = false;
            }
            else
            {
                nowBinElm.set(8 - 1); //flag bit set to 1��������pushһ�����滹�У���Ϊ��ʹnowPos�����һλ��������һλҲҪ�����洦��
                this->push(nowBinElm);
                nowBinElm.reset();
            }

            //move bits to nowBinElm��nowBinElm�����λ�Ǳ�־λ���������ѭ����д��
            for (short i = 6;i >= 0 && nowPos <= stopPos;i--, nowPos++)
            {
                //��bi�е�λд���Ԫ�أ�i�Ӹ�λ����λ��nowPos�ӵ�λ����λ��
                nowBinElm[i] = bi[nowPos];
            }
        } while (nowPos <= stopPos);

        if (nowBinElm.any()) //nowBinElm��ȫΪ0����Ϊ���Ŵ棬���һλһ����1����˵������pushһ��
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
            zipcode--; //������ǰ
        this->init<UInt64, 64>(zipcode);
    }

    SInt64 AsSInt64() const { return AsSInt64(this->bin); }
    static SInt64 AsSInt64(const std::vector<UInt8>& bin)
    {
        UInt64 zigcode = AsUInt64(bin);
        SInt64 result = 0;
        if (zigcode % 2 == 0)
            result = -SInt64(zigcode) / 2; //ż���±��Ǹ���
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
                //�ӵ�λ���±�С����ʼ��һ�����һ��1��λ��
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