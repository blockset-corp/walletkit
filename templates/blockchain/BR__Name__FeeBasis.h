//
//  BR__Name__FeeBasis.h
//  WalletKitCore
//
//  Created by __USER__ on __DATE__.
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef BR__Name__FeeBasis_h
#define BR__Name__FeeBasis_h

#include <stdint.h>
#include "BR__Name__Base.h"

#ifdef __cplusplus
extern "C" {
#endif

#define __NAME___DEFAULT_MUTEZ_PER_BYTE 1

typedef enum {
    FEE_BASIS_INITIAL,      // for fee estimation
    FEE_BASIS_ESTIMATE,     // for submit
    FEE_BASIS_ACTUAL        // when included
} BR__Name__FeeBasisType;

typedef struct
{
    BR__Name__FeeBasisType type;
    union {
        struct {
            BR__Name__UnitMutez    mutezPerKByte;
            double              sizeInKBytes;
            int64_t             gasLimit;
            int64_t             storageLimit;
        } initial;
        
        struct {
            BR__Name__UnitMutez    calculatedFee;
            int64_t             gasLimit;
            int64_t             storageLimit;
            int64_t             counter;
        } estimate;
        
        struct {
            BR__Name__UnitMutez fee;
        } actual;
    } u;
} BR__Name__FeeBasis;


extern BR__Name__FeeBasis
__name__DefaultFeeBasis(BR__Name__UnitMutez mutezPerKByte);

extern BR__Name__FeeBasis
__name__FeeBasisCreateEstimate(BR__Name__UnitMutez mutezPerKByte,
                            double sizeInBytes,
                            int64_t gasLimit,
                            int64_t storageLimit,
                            int64_t counter);

extern BR__Name__FeeBasis
__name__FeeBasisCreateActual(BR__Name__UnitMutez fee);

extern BR__Name__UnitMutez
__name__FeeBasisGetFee(BR__Name__FeeBasis *feeBasis);

extern bool
__name__FeeBasisIsEqual(BR__Name__FeeBasis *fb1, BR__Name__FeeBasis *fb2);

private_extern int64_t
__name__FeeBasisGetGasLimit(BR__Name__FeeBasis feeBasis);

private_extern int64_t
__name__FeeBasisGetStorageLimit(BR__Name__FeeBasis feeBasis);


#ifdef __cplusplus
}
#endif

#endif /* BR__Name__FeeBasis_h */
