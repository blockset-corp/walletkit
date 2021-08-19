//
//  File.c
//  
//
//  Created by Amit on 17/08/2021.
//

#include "BRAvaxEncode.h"
#include "support/BRInt.h"



uint8_t * avaxPackUInt32(uint32_t value, uint8_t * buffer)
{
    UInt32SetBE(buffer, value);
    return buffer ;
}

uint8_t * avaxPackUInt64(uint64_t value, uint8_t* buffer)
{
    UInt64SetBE(buffer, value);
    return buffer;
}

