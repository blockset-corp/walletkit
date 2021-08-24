//
//  BRAvalancheSupport.h
//  WalletKitCore
//
//  Created by Amit Shah on 2021-08-04.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef BRAvalancheSupport_h
#define BRAvalancheSupport_h

#include "BRAvalancheBase.h"

// returns the number of characters written to str including NULL terminator, or total strLen needed if str is NULL
extern size_t
BRAvalancheCB58CheckEncode(char *str, size_t strLen, const uint8_t *data, size_t dataLen);

extern char *
BRAvalancheCB58CheckEncodeCreate (const uint8_t *data, size_t dataLen);

extern size_t
BRAvalancheCB58CheckDecode(uint8_t *data, size_t dataLen, const char *str);

extern uint8_t *
BRAvalancheCB58CheckDecodeCreate(const char *str, size_t *dataLen);

#endif // defined(BRAvalancheSupport_h)
