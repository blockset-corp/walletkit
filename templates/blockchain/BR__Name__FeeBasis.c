//
//  BR__Name__FeeBasis.c
//  WalletKitCore
//
//  Created by __USER__ on __DATE__.
//  Copyright © __YEAR__ Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include <stdint.h>
#include "BR__Name__FeeBasis.h"
#include "support/util/BRUtilMath.h"
#include "walletkit/WKBaseP.h"
#include <stdio.h>
#include <assert.h>


extern BR__Name__FeeBasis
__name__FeeBasisCreate(void /* arguments */) {
    ASSERT_UNIMPLEMENTED;
    return (BR__Name__FeeBasis) { };
}

extern BR__Name__Amount
__name__FeeBasisGetFee (BR__Name__FeeBasis *feeBasis) {
    ASSERT_UNIMPLEMENTED;
}

extern bool
__name__FeeBasisIsEqual (BR__Name__FeeBasis *fb1, BR__Name__FeeBasis *fb2) {
    assert(fb1);
    assert(fb2);

    ASSERT_UNIMPLEMENTED;
    return (fb1 == fb2 ||
            // Compare
            (false));
}
