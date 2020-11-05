//
//  BRCryptoSupportXRP.c
//  Core
//
//  Created by Ehsan Rezaie on 2020-05-19.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "BRCryptoXRP.h"
#include "ripple/BRRippleBase.h"
#include "crypto/BRCryptoHashP.h"
#include "crypto/BRCryptoAmountP.h"
#include "ethereum/util/BRUtilMath.h"
#include "support/util/BRHex.h"

private_extern BRCryptoAmount
cryptoAmountCreateAsXRP (BRCryptoUnit unit,
                         BRCryptoBoolean isNegative,
                         BRRippleUnitDrops value) {
    return cryptoAmountCreate (unit, isNegative, uint256Create (value));
}

private_extern BRCryptoHash
cryptoHashCreateAsXRP (BRRippleTransactionHash hash) {
    uint32_t setValue = (uint32_t) ((UInt256 *) hash.bytes)->u32[0];
    return cryptoHashCreateInternal (setValue, 32, hash.bytes, CRYPTO_NETWORK_TYPE_XRP);
}

private_extern BRRippleTransactionHash
rippleHashCreateFromString (const char *string) {
    BRRippleTransactionHash hash;
    hexDecode (hash.bytes, sizeof (hash.bytes), string, strlen (string));
    return hash;
}

// MARK: -

private_extern int // 1 if equal, 0 if not.
rippleCompareFieldOption (const char *t1, const char *t2) {
    return 0 == strcasecmp (t1, t2);
}

static const char *knownDestinationTagRequiringAddresses[] = {
    "rLNaPoKeeBjZe2qs6x52yVPZpZ8td4dc6w",           // Coinbase(1)
    "rw2ciyaNshpHe7bCHo4bRWq6pqqynnWKQg",           // Coinbase(2)
    "rEb8TK3gBgk5auZkwc6sHnwrGVJH8DuaLh",           // Binance(1)
    "rJb5KsHsDHF1YS5B5DU6QCkH5NsPaKQTcy",           // Binance(2)
    "rEy8TFcrAPvhpKrwyrscNYyqBGUkE9hKaJ",           // Binance(3)
    "rXieaAC3nevTKgVu2SYoShjTCS2Tfczqx",            // Wirex(1)
    "r9HwsqBnAUN4nF6nDqxd4sgP8DrDnDcZP3",           // BitBay
    "rLbKbPyuvs4wc1h13BEPHgbFGsRXMeFGL6",           // BitBank(1)
    "rw7m3CtVHwGSdhFjV4MyJozmZJv3DYQnsA",           // BitBank(2)
    NULL
};

static int
rippleRequiresDestinationTag (BRRippleAddress address) {
    if (NULL == address) return 0;

    char *addressAsString = rippleAddressAsString(address);
    int isRequired = 0;

    for (size_t index = 0; NULL != knownDestinationTagRequiringAddresses[index]; index++)
        if (0 == strcasecmp (addressAsString, knownDestinationTagRequiringAddresses[index])) {
            isRequired = 1;
            break;
        }

    free (addressAsString);
    return isRequired;
}

private_extern const char **
rippleAddressGetTransactionAttributeKeys (BRRippleAddress address,
                                          int asRequired,
                                          size_t *count) {
    
    if (rippleRequiresDestinationTag (address)) {
        static size_t requiredCount = 1;
        static const char *requiredNames[] = {
            FIELD_OPTION_DESTINATION_TAG,
        };
        
        static size_t optionalCount = 1;
        static const char *optionalNames[] = {
            FIELD_OPTION_INVOICE_ID
        };
        
        if (asRequired) { *count = requiredCount; return requiredNames; }
        else {            *count = optionalCount; return optionalNames; }
    }
    
    else {
        static size_t requiredCount = 0;
        static const char **requiredNames = NULL;
        
        static size_t optionalCount = 2;
        static const char *optionalNames[] = {
            FIELD_OPTION_DESTINATION_TAG,
            FIELD_OPTION_INVOICE_ID
        };
        
        if (asRequired) { *count = requiredCount; return requiredNames; }
        else {            *count = optionalCount; return optionalNames; }
    }
}
