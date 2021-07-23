//
//  BR__Name__FeeBasis.h
//  WalletKitCore
//
//  Created by __USER__ on __DATE__.
//  Copyright © __YEAR__ Breadwinner AG. All rights reserved.
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


typedef struct
{
} BR__Name__FeeBasis;

extern BR__Name__FeeBasis
__name__FeeBasisCreate(void /* arguments */);

extern BR__Name__Amount
__name__FeeBasisGetFee(BR__Name__FeeBasis *feeBasis);

extern bool
__name__FeeBasisIsEqual(BR__Name__FeeBasis *fb1, BR__Name__FeeBasis *fb2);

#ifdef __cplusplus
}
#endif

#endif /* BR__Name__FeeBasis_h */
