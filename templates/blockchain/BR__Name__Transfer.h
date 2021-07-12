//
//  BR__Name__Transfer.h
//  WalletKitCore
//
//  Created by __USER__ on __DATE__.
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef BR__Name__Transfer_h
#define BR__Name__Transfer_h

#include "BR__Name__Base.h"
#include "BR__Name__Transaction.h"
#include "BR__Name__Address.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BR__Name__TransferRecord *BR__Name__Transfer;

// Create a new transfer (transaction operation) for submitting
extern BR__Name__Transfer /* caller must free - __name__TransferFree */
__name__TransferCreateNew(BR__Name__Address from,
                       BR__Name__Address to,
                       BR__Name__UnitMutez amount,
                       BR__Name__FeeBasis feeBasis,
                       int64_t counter,
                       bool delegationOp);

extern BR__Name__Transfer /* caller must free - __name__TransferFree */
__name__TransferCreate(BR__Name__Address from, BR__Name__Address to,
                    BR__Name__UnitMutez amount,
                    BR__Name__UnitMutez fee,
                    BR__Name__Hash hash,
                    uint64_t timestamp, uint64_t blockHeight, int error);

extern BR__Name__Transfer __name__TransferClone (BR__Name__Transfer transfer);
extern void __name__TransferFree(BR__Name__Transfer transfer);

// Getters for all the values
extern BR__Name__Hash __name__TransferGetTransactionId(BR__Name__Transfer transfer);
extern BR__Name__UnitMutez __name__TransferGetAmount(BR__Name__Transfer transfer);
extern BR__Name__UnitMutez __name__TransferGetAmountDirected (BR__Name__Transfer transfer,
                                                        BR__Name__Address address,
                                                        int *negative);
extern BR__Name__UnitMutez __name__TransferGetFee(BR__Name__Transfer transfer);
extern BR__Name__FeeBasis __name__TransferGetFeeBasis (BR__Name__Transfer transfer);
extern void __name__TransferSetFeeBasis (BR__Name__Transfer transfer,
                                      BR__Name__FeeBasis feeBasis);

extern BR__Name__Address // caller owns object, must free with __name__AddressFree
__name__TransferGetSource(BR__Name__Transfer transfer);

extern BR__Name__Address // caller owns object, must free with __name__AddressFree
__name__TransferGetTarget(BR__Name__Transfer transfer);

extern int
__name__TransferHasError(BR__Name__Transfer transfer);

extern BR__Name__Transaction __name__TransferGetTransaction(BR__Name__Transfer transfer);

// Internal
extern int __name__TransferHasSource (BR__Name__Transfer transfer,
                                   BR__Name__Address source);
extern int __name__TransferHasTarget (BR__Name__Transfer transfer,
                                   BR__Name__Address target);

extern uint64_t __name__TransferGetBlockHeight (BR__Name__Transfer transfer);

extern int
__name__TransferIsEqual (BR__Name__Transfer t1, BR__Name__Transfer t2);

#ifdef __cplusplus
}
#endif

#endif // BR__Name__Transfer_h
