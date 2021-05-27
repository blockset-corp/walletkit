//
//  BRCryptoSupportHBAR.c
//  Core
//
//  Created by Ehsan Rezaie on 2020-05-19.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "BRCryptoHBAR.h"
#include "hedera/BRHederaBase.h"
#include "crypto/BRCryptoAmountP.h"
#include "crypto/BRCryptoHashP.h"
#include "ethereum/util/BRUtilMath.h"
#include "support/util/BRHex.h"

static uint64_t
hederaTinyBarCoerceToUInt64 (BRHederaUnitTinyBar bars) {
    assert (bars >= 0);
    return (uint64_t) bars;
}

private_extern BRCryptoAmount
cryptoAmountCreateAsHBAR (BRCryptoUnit unit,
                          BRCryptoBoolean isNegative,
                          BRHederaUnitTinyBar value) {
    return cryptoAmountCreate (unit, isNegative, uint256Create (hederaTinyBarCoerceToUInt64 (value)));
}

private_extern BRCryptoHash
cryptoHashCreateAsHBAR (BRHederaTransactionHash hash) {
    uint32_t setValue = (uint32_t) ((UInt256 *) hash.bytes)->u32[0];
    return cryptoHashCreateInternal (setValue, 48, hash.bytes, CRYPTO_NETWORK_TYPE_HBAR);
}

private_extern BRHederaTransactionHash
hederaHashCreateFromString (const char *string) {
    BRHederaTransactionHash hash;
    memset (hash.bytes, 0x00, sizeof (hash.bytes));
    assert (96 == strlen (string));
    hexDecode (hash.bytes, sizeof (hash.bytes), string, strlen (string));
    return hash;
}

private_extern BRHederaTransactionHash
cryptoHashAsHBAR (BRCryptoHash hash) {
    assert (48 == hash->bytesCount);
    BRHederaTransactionHash hbarHash;
    memcpy (hbarHash.bytes, hash->bytes, hash->bytesCount);
    return hbarHash;
}

// MARK: -

static const char *knownMemoRequiringAddresses[] = {
    "0.0.16952",                // Binance
    NULL
};

private_extern int // 1 if equal, 0 if not.
hederaCompareAttribute (const char *t1, const char *t2) {
    return 0 == strcasecmp (t1, t2);
}

static int
hederaRequiresMemo (BRHederaAddress address) {
    if (NULL == address) return 0;

    char *addressAsString = hederaAddressAsString(address);
    int isRequired = 0;

    for (size_t index = 0; NULL != knownMemoRequiringAddresses[index]; index++)
        if (0 == strcasecmp (addressAsString, knownMemoRequiringAddresses[index])) {
            isRequired = 1;
            break;
        }

    free (addressAsString);
    return isRequired;
}

private_extern const char **
hederaWalletGetTransactionAttributeKeys (BRHederaAddress address,
                                         int asRequired,
                                         size_t *count) {
    
    if (hederaRequiresMemo (address)) {
        static size_t requiredCount = 1;
        static const char *requiredNames[] = {
            TRANSFER_ATTRIBUTE_MEMO_TAG,
        };
        
        static size_t optionalCount = 0;
        static const char **optionalNames = NULL;

        if (asRequired) { *count = requiredCount; return requiredNames; }
        else {            *count = optionalCount; return optionalNames; }
    }
    
    else {
        static size_t requiredCount = 0;
        static const char **requiredNames = NULL;

        static size_t optionalCount = 1;
        static const char *optionalNames[] = {
            TRANSFER_ATTRIBUTE_MEMO_TAG
        };

        if (asRequired) { *count = requiredCount; return requiredNames; }
        else {            *count = optionalCount; return optionalNames; }
    }
}
