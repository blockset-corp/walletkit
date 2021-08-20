//
//  BRAvalancheFeeBasis.c
//  WalletKitCore
//
//  Created by Amit Shah on 2021-08-04.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include <stdint.h>
#include "BRAvalancheFeeBasis.h"
#include "support/util/BRUtilMath.h"
#include "walletkit/WKBaseP.h"
#include <stdio.h>
#include <assert.h>


extern BRAvalancheFeeBasis
avalancheFeeBasisCreate(BRAvalancheAmount avaxPerTransaction) {
    return (BRAvalancheFeeBasis) {
        avaxPerTransaction
    };
}

extern BRAvalancheAmount
avalancheFeeBasisGetFee (BRAvalancheFeeBasis *feeBasis) {
    return feeBasis->avaxPerTransaction * 1 /* transaction */;
}

extern bool
avalancheFeeBasisIsEqual (BRAvalancheFeeBasis *fb1, BRAvalancheFeeBasis *fb2) {
    assert(fb1);
    assert(fb2);

    return (fb1 == fb2 ||
            (fb1->avaxPerTransaction == fb2->avaxPerTransaction));
}
