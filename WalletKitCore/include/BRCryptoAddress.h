//
//  BRCryptoAddress.h
//  WalletKitCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoAddress_h
#define BRCryptoAddress_h

#include "BRCryptoBase.h"

#ifdef __cplusplus
extern "C" {
#endif

    /// MARK: - Address

    /**
     * @brief An address represents an account address on a specific network.
     *
     * @discussion Different networks have different addresses and have different string
     * representations of addresses.  For example, an Ethereum address is 20 bytes that is 'printed'
     * as a '0x'-prefixed hex string with or without capitalization as a checksum.  Bitcoin has
     * 'legacy' and 'segwit' addresses.
     */
    typedef struct BRCryptoAddressRecord *BRCryptoAddress;

    ///
    /// Returns the address' string representation which is suitable for display.  Note that an
    /// address representing BCH will have a prefix included, typically one of 'bitcoincash' or
    ///'bchtest'.  And, there is not the reverse function of `cryptoAddressCreateFromString()`
    /// whereby the type (BTC, BCH, ETH, ...) is derived from the string - one must know
    /// beforehand in order to process the string.
    ///
    /// @param address the address
    ///
    /// @return A string representation which is newly allocated and must be freed.
    ///
    extern char *
    cryptoAddressAsString (BRCryptoAddress address);

    ///
    /// Compare two address for identity.
    ///
    /// @param a1
    /// @param a2
    ///
    /// @return CRYPTO_TRUE if identical, CRYPTO_FALSE otherwise
    ///
    extern BRCryptoBoolean
    cryptoAddressIsIdentical (BRCryptoAddress a1,
                              BRCryptoAddress a2);

    DECLARE_CRYPTO_GIVE_TAKE (BRCryptoAddress, cryptoAddress);

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoAddress_h */
