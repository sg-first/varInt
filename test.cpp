#include "varint.h"
using namespace std;

int main()
{
    for (UInt64 i = 0;i < 1000000;i++)
    {
        VarInt v(i);
        auto r = v.AsUInt64();
        if (r != i)
            cout << "fuck" << endl;
    }
    for (SInt64 i = -10000;i < 10000;i++)
    {
        VarInt v(i);
        SInt64 r = v.AsSInt64();
        if (r != i)
            cout << "fuck" << endl;
    }
}