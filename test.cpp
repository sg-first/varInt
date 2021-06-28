#include "varint.h"
using namespace std;

int main()
{
    for (UInt8 i = 0;i < 128;i++)
    {
        VarInt v(i);
        if (v.AsUInt64() != i)
            cout << "fuck" << endl;
    }
    for (SInt64 i = -10000;i < 1000;i++)
    {
        VarInt v(i);
        SInt64 r = v.AsSInt64();
        if (r != i)
            cout << "fuck" << endl;
    }
}