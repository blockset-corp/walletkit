//
//  WKSupportXTZ.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-08-27.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKXTZ.h"
#include "tezos/BRTezosBase.h"
#include "walletkit/WKHashP.h"
#include "walletkit/WKAmountP.h"
#include "support/util/BRUtilMath.h"
#include "support/BRBase58.h"


private_extern WKAmount
wkAmountCreateAsXTZ (WKUnit unit,
                         WKBoolean isNegative,
                         BRTezosUnitMutez value) {
    return wkAmountCreate (unit, isNegative, uint256Create ((uint64_t)value));
}

private_extern BRTezosUnitMutez
tezosMutezCreate (WKAmount amount) {
    UInt256 value = wkAmountGetValue (amount);
    return (BRTezosUnitMutez) value.u64[0];
}

// MARK: - Hash

static uint32_t
tezosHashSetValue (const BRTezosHash *hash) {
    return (uint32_t) ((UInt256 *) hash->bytes)->u32[0];
}

private_extern WKHash
wkHashCreateAsXTZ (BRTezosHash hash) {
    return wkHashCreateInternal (tezosHashSetValue (&hash),
                                     TEZOS_HASH_BYTES,
                                     hash.bytes,
                                     WK_NETWORK_TYPE_XTZ);
}

private_extern WKHash
wkHashCreateFromStringAsXTZ(const char *input) {
    size_t length = BRBase58CheckDecode(NULL, 0, input);
    assert(length == TEZOS_HASH_BYTES);
    BRTezosHash hash;
    BRBase58CheckDecode(hash.bytes, length, input);
    return wkHashCreateAsXTZ (hash);
}

private_extern BRTezosHash
wkHashAsXTZ (WKHash hash) {
    assert (TEZOS_HASH_BYTES == hash->bytesCount);
    BRTezosHash xtz;
    memcpy (xtz.bytes, hash->bytes, TEZOS_HASH_BYTES);
    return xtz;
}

// MARK: -

private_extern const char **
tezosGetTransactionAttributeKeys (int asRequired,
                                  size_t *count) {
    
    static size_t requiredCount = 0;
    static const char **requiredNames = NULL;
    
    static size_t optionalCount = 3;
    static const char *optionalNames[] = {
        FIELD_OPTION_DELEGATION_OP,
        FIELD_OPTION_DELEGATE,
        FIELD_OPTION_OPERATION_TYPE
    };
    
    if (asRequired) { *count = requiredCount; return requiredNames; }
    else {            *count = optionalCount; return optionalNames; }
}
