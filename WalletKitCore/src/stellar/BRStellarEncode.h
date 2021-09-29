//
//  BRStellarSerialize.h
//  WalletKitCore
//
//  Created by Carl Cherry on 5/21/2019
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef BRStellar_serialize_h
#define BRStellar_serialize_h

#include "BRStellarBase.h"
#include "BRStellarPrivateStructs.h"
#include "BRStellarTransaction.h"
#include "support/BRArray.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ST_XDR_SUCCESS                  ( 0)
#define ST_XDR_FAILURE                  (-1)
#define ST_XDR_UNSUPPORTED_OPERATION    (-2)
    
extern size_t stellarSerializeTransaction(BRStellarAddress from,
                                          BRStellarAddress to,
                                          BRStellarFee fee,
                                          BRStellarAmount amount,
                                          BRStellarSequence sequence,
                                          BRStellarMemo *memo,
                                          int32_t version,
                                          uint8_t *signature,
                                          uint8_t **buffer);

#ifdef __cplusplus
}
#endif

#endif // BRStellar_serialize_h
