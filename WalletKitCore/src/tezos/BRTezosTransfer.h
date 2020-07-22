//
//  BRTezosTransfer.h
//  Core
//
//  Created by Ehsan Rezaie on 2020-06-17.
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef BRTezosTransfer_h
#define BRTezosTransfer_h

#include "BRTezosBase.h"
#include "BRTezosTransaction.h"
#include "BRTezosAddress.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BRTezosTransferRecord *BRTezosTransfer;

// Create a new transfer (transaction operation) for submitting
extern BRTezosTransfer /* caller must free - tezosTransferFree */
tezosTransferCreateNew(BRTezosAddress from, BRTezosAddress to, BRTezosUnitMutez amount);

extern BRTezosTransfer /* caller must free - tezosTransferFree */
tezosTransferCreate(BRTezosAddress from, BRTezosAddress to,
                    BRTezosUnitMutez amount,
                    BRTezosUnitMutez fee,
                    BRTezosTransactionHash hash,
                    uint64_t timestamp, uint64_t blockHeight, int error);

extern BRTezosTransfer tezosTransferClone (BRTezosTransfer transfer);
extern void tezosTransferFree(BRTezosTransfer transfer);

// Getters for all the values
extern BRTezosTransactionHash tezosTransferGetTransactionId(BRTezosTransfer transfer);
extern BRTezosUnitMutez tezosTransferGetAmount(BRTezosTransfer transfer);
extern BRTezosUnitMutez tezosTransferGetFee(BRTezosTransfer transfer);
extern BRTezosFeeBasis tezosTransferGetFeeBasis (BRTezosTransfer transfer);

extern BRTezosAddress // caller owns object, must free with tezosAddressFree
tezosTransferGetSource(BRTezosTransfer transfer);

extern BRTezosAddress // caller owns object, must free with tezosAddressFree
tezosTransferGetTarget(BRTezosTransfer transfer);

extern int
tezosTransferHasError(BRTezosTransfer transfer);

extern BRTezosTransaction tezosTransferGetTransaction(BRTezosTransfer transfer);

// Internal
extern int tezosTransferHasSource (BRTezosTransfer transfer,
                                   BRTezosAddress source);

extern uint64_t tezosTransferGetBlockHeight (BRTezosTransfer transfer);

#ifdef __cplusplus
}
#endif

#endif // BRTezosTransfer_h
