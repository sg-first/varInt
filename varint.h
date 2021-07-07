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
            short nowSub = 0; //С���ǵ��ֽ�

            auto isLast = [](const UInt8& i) { return (i & 0b10000000) == 0; };

            for (size_t i = startPos;i < endPos;i++)
            {
                UInt8 ri = bin[i];
                if (isLast(ri))
                {
                    if (nowSub >= 2) //������߶������0
                    {
                        short zeroNum = nowSub - 1; //��ǰ���λ���0�ĸ�����Ҫȡ����λ��
                        std::bitset<8> beop(ptr[nowSub - 1]); //׼��������ǰresult����ֽڣ��еĸ߼�λ��
                        std::bitset<8> op(ri); //ȡbin����ֽڣ��еĵڼ�λ��
                        for (short i = 0;i < zeroNum;i++)
                        {
                            beop[8 - zeroNum + i] = op[i];
                        }
                        ptr[nowSub - 1] = beop.to_ulong();
                        ri >>= zeroNum; //�Ѿ������õ�λ����
                    }
                    ptr[nowSub] = ri; //bin�����һ�ֽڷ�������ֽ� 
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