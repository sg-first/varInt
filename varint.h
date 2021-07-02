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

        auto isEnd = [&ptr, &stopPos](short nextHightestBit) { return nextHightestBit>=stopPos && ((ptr[0] & 0b10000000) == 0); }; //������ԣ����������λλ��

        short nextHightestBit = 7; //��һ��Ҫ�����ֵ����λ���±꣩
        while (true)
        {
            UInt8 temp = ptr[0];
            if (isEnd(nextHightestBit)) //����û�����һλ�������ǰλ��λ��1������Ҫ�ٴ���һ�Ρ���������Ǽ����λΪ0��
            {
                //����ڶ���������λһ����0�����Բ�����flag bit
                bin.push_back(temp); //���λ�������
                break;
            }
            else
            {
                temp |= 0b10000000;
                this->bin.insert(this->bin.begin(), temp); //Խ��λ�±�ԽС
                i >>= 7; //���Ѵ������7λȥ��
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
            zipcode--; //������ǰ
        this->init(zipcode);
    }

    std::vector<UInt8> getBin() { return this->bin; }
    SInt64 AsSInt64() const { return AsSInt64(this->bin); }
    static SInt64 AsSInt64(const std::vector<UInt8>& bin, size_t startPos = 0, size_t endPos = 0)
    {
        UInt64 zigcode = AsUInt64(bin, startPos, endPos);
        SInt64 result = 0;
        if (zigcode % 2 == 0)
            result = -SInt64(zigcode) / 2; //ż���±��Ǹ���
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
            short nowSub = 0; //С���ǵ��ֽ�

            auto isLast = [](const UInt8& i) { return (i & 0b10000000) == 0; };

            for (size_t i = startPos;i < endPos;i++)
            {
                UInt8 ri = bin[i];
                if (isLast(ri))
                {
                    if (nowSub >= 2) //������߶������0
                    {
                        if ((ri & 0b00000001) == 1) //ȡ����ֽ��е����λ������ptr[nowSub]���λ������Ϊһ��
                            ptr[nowSub - 1] |= 0b10000000;
                        ri >>= 1; //����ֽ������λ�ѱ�����
                        
                    }
                    ptr[nowSub] = ri; //���һ�ֽڷ�������ֽ� 
                    result >>= 1; //����else�б�־λ��ɵ����ұ߶������0��
                    break;
                }
                else
                {
                    result <<= 7; //�ڵط���nowSub=1���ٽ��У���߻���һ���������0��
                    //���µķŽ���
                    ri <<= 1; //remove flag bit���ⲽ����֮���ұ��и��������0��
                    ptr[0] = ri; //���ڵķŵ�����ֽڣ��±�Խ�����Խ��λ��
                    
                    nowSub++; //���ڵ�����ֽڵ�����
                }
            }
            
            return result;
        }
    }
};