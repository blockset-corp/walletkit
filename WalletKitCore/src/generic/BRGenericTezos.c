//
//  BRGenericTezos.c
//  Core
//
//  Created by Ehsan Rezaie on 2020-06-17.
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include <errno.h>

#include "BRGenericTezos.h"
#include "tezos/BRTezosAccount.h"
#include "tezos/BRTezosWallet.h"
#include "tezos/BRTezosTransaction.h"
#include "tezos/BRTezosFeeBasis.h"
#include "support/BRSet.h"
#include "support/BRBase58.h"

// MARK: - Generic Network

// MARK: - Generic Account

static BRGenericAccountRef
genericTezosAccountCreate (BRCryptoNetworkCanonicalType type, UInt512 seed) {
    return (BRGenericAccountRef) tezosAccountCreateWithSeed (seed);
}

static BRGenericAccountRef
genericTezosAccountCreateWithSerialization (BRCryptoNetworkCanonicalType type, uint8_t *bytes, size_t bytesCount) {
    return (BRGenericAccountRef) tezosAccountCreateWithSerialization (bytes, bytesCount);
}

static void
genericTezosAccountFree (BRGenericAccountRef account) {
    tezosAccountFree ((BRTezosAccount) account);
}

static BRGenericAddressRef
genericTezosAccountGetAddress (BRGenericAccountRef account) {
    return (BRGenericAddressRef) tezosAccountGetAddress((BRTezosAccount) account);
}

static uint8_t *
genericTezosAccountGetInitializationData (BRGenericAccountRef account, size_t *bytesCount) {
    if (NULL != bytesCount) *bytesCount = 0;
    return NULL;
}

static void
genericTezosAccountInitialize (BRGenericAccountRef account, const uint8_t *bytes, size_t bytesCount) {
    return;
}

static int
genericTezosAccountIsInitialized (BRGenericAccountRef account) {
    return 1;
}

static uint8_t *
genericTezosAccountGetSerialization (BRGenericAccountRef account,
                                     size_t *bytesCount) {
    return tezosAccountGetSerialization ((BRTezosAccount) account, bytesCount);
}

static void
genericTezosAccountSignTransferWithSeed (BRGenericAccountRef account,
                                         BRGenericTransferRef transfer,
                                         UInt512 seed)
{
    BRKey publicKey = tezosAccountGetPublicKey ((BRTezosAccount)account);
    BRTezosTransaction transaction = tezosTransferGetTransaction ((BRTezosTransfer) transfer);
    if (transaction) {
        tezosTransactionSignTransaction (transaction, publicKey, seed);
    }
}

// MARK: - Generic Address

static BRGenericAddressRef
genericTezosAddressCreate (const char *string) {
    return (BRGenericAddressRef) tezosAddressCreateFromString (string, true);
}

static char *
genericTezosAddressAsString (BRGenericAddressRef address) {
    return tezosAddressAsString ((BRTezosAddress) address);
}

static int
genericTezosAddressEqual (BRGenericAddressRef address1,
                          BRGenericAddressRef address2) {
    return tezosAddressEqual ((BRTezosAddress) address1,
                              (BRTezosAddress) address2);
}

static void
genericTezosAddressFree (BRGenericAddressRef address) {
    tezosAddressFree ((BRTezosAddress) address);
}

// MARK: - Generic Transfer

static BRGenericTransferRef
genericTezosTransferCopy (BRGenericTransferRef transfer) {
    return (BRGenericTransferRef) tezosTransferClone ((BRTezosTransfer) transfer);
}

static void
genericTezosTransferFree (BRGenericTransferRef transfer) {
    tezosTransferFree ((BRTezosTransfer) transfer);
}

static BRGenericAddressRef
genericTezosTransferGetSourceAddress (BRGenericTransferRef transfer) {
    return (BRGenericAddressRef) tezosTransferGetSource ((BRTezosTransfer) transfer);
}

static BRGenericAddressRef
genericTezosTransferGetTargetAddress (BRGenericTransferRef transfer) {
    return (BRGenericAddressRef) tezosTransferGetTarget ((BRTezosTransfer) transfer);
}

static UInt256
genericTezosTransferGetAmount (BRGenericTransferRef transfer) {
    BRTezosUnitMutez mutez = tezosTransferGetAmount ((BRTezosTransfer) transfer);
    return uint256Create(mutez);
}

static BRGenericFeeBasis
genericTezosTransferGetFeeBasis (BRGenericTransferRef transfer) {
    BRTezosUnitMutez tezosFee = tezosTransferGetFee ((BRTezosTransfer) transfer);
    return (BRGenericFeeBasis) {
        uint256Create (tezosFee),
        1
    };
}

static BRGenericHash
genericTezosTransferGetHash (BRGenericTransferRef transfer) {
    BRTezosTransactionHash hash = tezosTransferGetTransactionId ((BRTezosTransfer) transfer);
    return genericHashCreate (sizeof (hash.bytes), hash.bytes);
}

static uint8_t *
genericTezosTransferGetSerialization (BRGenericTransferRef transfer, size_t *bytesCount)
{
    uint8_t * result = NULL;
    *bytesCount = 0;
    BRTezosTransaction transaction = tezosTransferGetTransaction ((BRTezosTransfer) transfer);
    if (transaction) {
        result = tezosTransactionSerialize(transaction, bytesCount);
    }
    return result;
}

// MARK: Generic Wallet

static BRGenericWalletRef
genericTezosWalletCreate (BRGenericAccountRef account) {
    return (BRGenericWalletRef) tezosWalletCreate ((BRTezosAccount) account);
}

static void
genericTezosWalletFree (BRGenericWalletRef wallet) {
    tezosWalletFree ((BRTezosWallet) wallet);
}

static UInt256
genericTezosWalletGetBalance (BRGenericWalletRef wallet) {
    return uint256Create (tezosWalletGetBalance ((BRTezosWallet) wallet));
}

static UInt256
genericTezosWalletGetBalanceLimit (BRGenericWalletRef wallet,
                                   int asMaximum,
                                   int *hasLimit) {
    return uint256Create (tezosWalletGetBalanceLimit ((BRTezosWallet) wallet, asMaximum, hasLimit));
}

static BRGenericAddressRef
genericTezosGetAddress (BRGenericWalletRef wallet, int asSource) {
    return (BRGenericAddressRef) (asSource
                                  ? tezosWalletGetSourceAddress ((BRTezosWallet) wallet)
                                  : tezosWalletGetTargetAddress ((BRTezosWallet) wallet));
}

static int
genericTezosWalletHasAddress (BRGenericWalletRef wallet,
                              BRGenericAddressRef address) {
    return tezosWalletHasAddress ((BRTezosWallet) wallet,
                                  (BRTezosAddress) address);
}

static int
genericTezosWalletHasTransfer (BRGenericWalletRef wallet,
                               BRGenericTransferRef transfer) {
    return tezosWalletHasTransfer ((BRTezosWallet) wallet, (BRTezosTransfer) transfer);
}

static void
genericTezosWalletAddTransfer (BRGenericWalletRef wallet,
                               OwnershipKept BRGenericTransferRef transfer) {
    tezosWalletAddTransfer ((BRTezosWallet) wallet, (BRTezosTransfer) transfer);
}

static void
genericTezosWalletRemTransfer (BRGenericWalletRef wallet,
                               OwnershipKept BRGenericTransferRef transfer) {
    tezosWalletRemTransfer ((BRTezosWallet) wallet, (BRTezosTransfer) transfer);
}

#define FIELD_OPTION_DESTINATION_TAG        "DestinationTag"
#define FIELD_OPTION_INVOICE_ID             "InvoiceId"

static int // 1 if equal, 0 if not.
genericTezosCompareFieldOption (const char *t1, const char *t2) {
    return 0 == strcasecmp (t1, t2);
}

static BRGenericTransferRef
genericTezosWalletCreateTransfer (BRGenericWalletRef wallet,
                                  BRGenericAddressRef target,
                                  UInt256 amount,
                                  BRGenericFeeBasis estimatedFeeBasis,
                                  size_t attributesCount,
                                  BRGenericTransferAttribute *attributes) {
    BRTezosAddress source  = tezosWalletGetSourceAddress ((BRTezosWallet) wallet);
    BRTezosUnitMutez mutez = amount.u64[0];
    
    BRTezosTransfer transfer = tezosTransferCreateNew (source,
                                                       (BRTezosAddress) target,
                                                       mutez);
    
    BRTezosTransaction transaction = tezosTransferGetTransaction(transfer);
    
    for (size_t index = 0; index < attributesCount; index++) {
        BRGenericTransferAttribute attribute = attributes[index];
        if (NULL != genTransferAttributeGetVal(attribute)) {
            //TODO:TEZOS attributes?
        }
    }
    
    tezosAddressFree(source);
    
    return (BRGenericTransferRef) transfer;
}

static BRGenericFeeBasis
genericTezosWalletEstimateFeeBasis (BRGenericWalletRef wallet,
                                    BRGenericAddressRef address,
                                    UInt256 amount,
                                    UInt256 pricePerCostFactor) {
    return (BRGenericFeeBasis) {
        pricePerCostFactor,
        1
    };
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
genericTezosRequiresDestinationTag (BRTezosAddress address) {
    if (NULL == address) return 0;
    
    char *addressAsString = tezosAddressAsString(address);
    int isRequired = 0;
    
    for (size_t index = 0; NULL != knownDestinationTagRequiringAddresses[index]; index++)
        if (0 == strcasecmp (addressAsString, knownDestinationTagRequiringAddresses[index])) {
            isRequired = 1;
            break;
        }
    
    free (addressAsString);
    return isRequired;
}

static const char **
genericTezosWalletGetTransactionAttributeKeys (BRGenericWalletRef wallet,
                                               BRGenericAddressRef address,
                                               int asRequired,
                                               size_t *count) {
    
    if (genericTezosRequiresDestinationTag ((BRTezosAddress) address)) {
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

static int
genericTezosWalletValidateTransactionAttribute (BRGenericWalletRef wallet,
                                                BRGenericTransferAttribute attribute) {
    const char *key = genTransferAttributeGetKey (attribute);
    const char *val = genTransferAttributeGetVal (attribute);
    
    // If attribute.value is NULL, we validate unless the attribute.value is required.
    if (NULL == val) return !genTransferAttributeIsRequired(attribute);
    
    if (genericTezosCompareFieldOption (key, FIELD_OPTION_DESTINATION_TAG)) {
        char *end = NULL;
        errno = 0;
        
        uintmax_t tag = strtoumax (val, &end, 10);
        return (ERANGE != errno && EINVAL != errno && '\0' == end[0] && tag <= UINT32_MAX);
    }
    else if (genericTezosCompareFieldOption (key, FIELD_OPTION_INVOICE_ID)) {
        BRCoreParseStatus status;
        uint256CreateParse(val, 10, &status);
        return CORE_PARSE_OK == status;
    }
    else return 0;
}

static int
genericTezosWalletValidateTransactionAttributes (BRGenericWalletRef wallet,
                                                 size_t attributesCount,
                                                 BRGenericTransferAttribute *attributes) {
    // Validate one-by-one
    for (size_t index = 0; index < attributesCount; index++)
        if (0 == genericTezosWalletValidateTransactionAttribute (wallet, attributes[index]))
            return 0;
    return 1;
}

// MARK: - Generic Manager

static BRGenericTransferRef
genericTezosWalletManagerRecoverTransfer (const char *hash,
                                          const char *from,
                                          const char *to,
                                          const char *amount,
                                          const char *currency,
                                          const char *fee,
                                          uint64_t timestamp,
                                          uint64_t blockHeight,
                                          int error) {
    BRTezosUnitMutez amountMutez, feeMutez = 0;
    sscanf(amount, "%" PRIu64, &amountMutez);
    if (NULL != fee) sscanf(fee, "%" PRIu64, &feeMutez);
    BRTezosAddress toAddress   = tezosAddressCreateFromString (to,   false);
    BRTezosAddress fromAddress = tezosAddressCreateFromString (from, false);
    // Convert the hash string to bytes
    BRTezosTransactionHash txId;
    BRBase58Decode(txId.bytes, sizeof(txId.bytes), hash);
    
    BRTezosTransfer transfer = tezosTransferCreate(fromAddress, toAddress, amountMutez, feeMutez, txId, timestamp, blockHeight, error);
    
    tezosAddressFree (toAddress);
    tezosAddressFree (fromAddress);
    
    return (BRGenericTransferRef) transfer;
}

static BRArrayOf(BRGenericTransferRef)
genericTezosWalletManagerRecoverTransfersFromRawTransaction (uint8_t *bytes,
                                                             size_t   bytesCount) {
    return NULL;
}

static BRGenericAPISyncType
genericTezosWalletManagerGetAPISyncType (void) {
    return GENERIC_SYNC_TYPE_TRANSFER;
}

// MARK: - Generic Handlers

struct BRGenericHandersRecord genericTezosHandlersRecord = {
    CRYPTO_NETWORK_TYPE_XTZ,
    { // Network
    },
    
    {    // Account
        genericTezosAccountCreate,
        NULL,//AccountCreateWithPublicKey,
        genericTezosAccountCreateWithSerialization,
        genericTezosAccountFree,
        genericTezosAccountGetAddress,
        genericTezosAccountGetInitializationData,
        genericTezosAccountInitialize,
        genericTezosAccountIsInitialized,
        genericTezosAccountGetSerialization,
        genericTezosAccountSignTransferWithSeed,
        NULL,//AccountSignTransferWithKey,
    },
    
    {    // Address
        genericTezosAddressCreate,
        genericTezosAddressAsString,
        genericTezosAddressEqual,
        genericTezosAddressFree
    },
    
    {    // Transfer
        NULL,//TransferCreate,
        genericTezosTransferCopy,
        genericTezosTransferFree,
        genericTezosTransferGetSourceAddress,
        genericTezosTransferGetTargetAddress,
        genericTezosTransferGetAmount,
        genericTezosTransferGetFeeBasis,
        genericTezosTransferGetHash,
        genericTezosTransferGetSerialization,
    },
    
    {   // Wallet
        genericTezosWalletCreate,
        genericTezosWalletFree,
        genericTezosWalletGetBalance,
        /* set balance */
        genericTezosWalletGetBalanceLimit,
        genericTezosGetAddress,
        genericTezosWalletHasAddress,
        genericTezosWalletHasTransfer,
        genericTezosWalletAddTransfer,
        genericTezosWalletRemTransfer,
        genericTezosWalletCreateTransfer,
        genericTezosWalletEstimateFeeBasis,
        
        genericTezosWalletGetTransactionAttributeKeys,
        genericTezosWalletValidateTransactionAttribute,
        genericTezosWalletValidateTransactionAttributes
    },
    
    { // Wallet Manager
        genericTezosWalletManagerRecoverTransfer,
        genericTezosWalletManagerRecoverTransfersFromRawTransaction,
        genericTezosWalletManagerGetAPISyncType,
    },
};

const BRGenericHandlers genericTezosHandlers = &genericTezosHandlersRecord;
