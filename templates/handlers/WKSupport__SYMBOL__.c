//
//  WKSupport__SYMBOL__.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-08-27.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WK__SYMBOL__.h"
#include "__name__/BR__Name__Base.h"
#include "walletkit/WKHashP.h"
#include "walletkit/WKAmountP.h"
#include "support/util/BRUtilMath.h"
#include "support/BRBase58.h"


private_extern WKAmount
wkAmountCreateAs__SYMBOL__ (WKUnit unit,
                         WKBoolean isNegative,
                         BR__Name__UnitMutez value) {
    return wkAmountCreate (unit, isNegative, uint256Create ((uint64_t)value));
}

private_extern BR__Name__UnitMutez
__name__MutezCreate (WKAmount amount) {
    UInt256 value = wkAmountGetValue (amount);
    return (BR__Name__UnitMutez) value.u64[0];
}

// MARK: - Hash

static uint32_t
__name__HashSetValue (const BR__Name__Hash *hash) {
    return (uint32_t) ((UInt256 *) hash->bytes)->u32[0];
}

private_extern WKHash
wkHashCreateAs__SYMBOL__ (BR__Name__Hash hash) {
    return wkHashCreateInternal (__name__HashSetValue (&hash),
                                     __NAME___HASH_BYTES,
                                     hash.bytes,
                                     WK_NETWORK_TYPE___SYMBOL__);
}

private_extern WKHash
wkHashCreateFromStringAs__SYMBOL__(const char *input) {
    size_t length = BRBase58CheckDecode(NULL, 0, input);
    assert(length == __NAME___HASH_BYTES);
    BR__Name__Hash hash;
    BRBase58CheckDecode(hash.bytes, length, input);
    return wkHashCreateAs__SYMBOL__ (hash);
}

private_extern BR__Name__Hash
wkHashAs__SYMBOL__ (WKHash hash) {
    assert (__NAME___HASH_BYTES == hash->bytesCount);
    BR__Name__Hash __symbol__;
    memcpy (__symbol__.bytes, hash->bytes, __NAME___HASH_BYTES);
    return __symbol__;
}

// MARK: -

private_extern const char **
__name__GetTransactionAttributeKeys (int asRequired,
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
