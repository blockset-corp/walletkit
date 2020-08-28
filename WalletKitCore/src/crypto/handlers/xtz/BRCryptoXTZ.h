//
//  BRCryptoXTZ.h
//  Core
//
//  Created by Ehsan Rezaie on 2020-08-27.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef BRCryptoXTZ_h
#define BRCryptoXTZ_h

#include "../BRCryptoHandlersExport.h"

#include "tezos/BRTezos.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Address

typedef struct BRCryptoAddressXTZRecord {
    struct BRCryptoAddressRecord base;
    BRTezosAddress addr;
} *BRCryptoAddressXTZ;

extern BRCryptoAddress
cryptoAddressCreateAsXTZ (BRTezosAddress addr);

extern BRCryptoAddress
cryptoAddressCreateFromStringAsXTZ (const char *string);

private_extern BRTezosAddress
cryptoAddressAsXTZ (BRCryptoAddress address);

// MARK: - Network

typedef struct BRCryptoNetworkXTZRecord {
    struct BRCryptoNetworkRecord base;
    // Nothing more needed
} *BRCryptoNetworkXTZ;

// MARK: - Transfer

typedef struct BRCryptoTransferXTZRecord {
    struct BRCryptoTransferRecord base;

    BRTezosTransfer xtzTransfer;
} *BRCryptoTransferXTZ;

extern BRCryptoTransferXTZ
cryptoTransferCoerceXTZ (BRCryptoTransfer transfer);

extern BRCryptoTransfer
cryptoTransferCreateAsXTZ (BRCryptoTransferListener listener,
                           BRCryptoUnit unit,
                           BRCryptoUnit unitForFee,
                           BRTezosAccount xtzAccount,
                           BRTezosTransfer xtzTransfer);

// MARK: - Wallet

typedef struct BRCryptoWalletXTZRecord {
    struct BRCryptoWalletRecord base;
    BRTezosAccount xtzAccount;
    int64_t counter;
} *BRCryptoWalletXTZ;

extern BRCryptoWalletHandlers cryptoWalletHandlersXTZ;

private_extern BRCryptoWallet
cryptoWalletCreateAsXTZ (BRCryptoWalletListener listener,
                         BRCryptoUnit unit,
                         BRCryptoUnit unitForFee,
                         BRTezosAccount xtzAccount);

private_extern bool
cryptoWalletNeedsRevealXTZ (BRCryptoWallet wallet);

//TODO:TEZOS set the initial counter
private_extern void
cryptoWalletSetCounterXTZ (BRCryptoWallet wallet, int64_t counter);

//TODO:XTZ needed?
private_extern BRCryptoHash
cryptoHashCreateAsXTZ (BRTezosTransactionHash hash);

// MARK: - Wallet Manager

typedef struct BRCryptoWalletManagerXTZRecord {
    struct BRCryptoWalletManagerRecord base;
} *BRCryptoWalletManagerXTZ;

extern BRCryptoWalletManagerHandlers cryptoWalletManagerHandlersXTZ;

// MARK: - Events

// MARK: - Support

#define FIELD_OPTION_DELEGATION_OP         "DelegationOp"

private_extern BRCryptoAmount
cryptoAmountCreateAsXTZ (BRCryptoUnit unit,
                         BRCryptoBoolean isNegative,
                         BRTezosUnitMutez value);


private_extern const char **
tezosGetTransactionAttributeKeys (int asRequired,
                                  size_t *count);


#ifdef __cplusplus
}
#endif

#endif // BRCryptoXTZ_h
