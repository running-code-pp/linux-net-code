/**
 * ==================================================
 *  @file byteOrder.cpp
 *  @brief 判断字节序（大端序/小端序）
 *  @author ywj
 *  @date 2025-06-24 22:50
 *  @version 1.0
 *  @copyright Copyright (c) 2025 ywj. All Rights Reserved.
 * ==================================================
 */

#include<stdio.h>

bool isBigEndian()
{
    union data
    {
        char _bytedata[2];
        short _shortdata;
    };
    data d;
    d._shortdata=0x0102;
    //大端序：高位字节存在低地址，低位字节存在高地址
    //小端序：高位字节存在高地址，低位字节存在低地址
    if (d._bytedata[0] == 0x02&&d._bytedata[1] == 0x01)
    {
        return false;
    }
    else
    {
        return true;
    }
    // else if(d._bytedata[0] == 0x01&&d._bytedata[1] == 0x02)
    // {
    //     return true;
    // }
}

int main()
{
    printf("the machine byteOrder is:%s\n",isBigEndian()?"BigEndian":"smallEndian");
    return 0;
}