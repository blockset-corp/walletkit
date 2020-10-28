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

static BRGenericHash
genericTezosNetworkCreateHashFromString (const char *string) {
    BRTezosHash hash = tezosHashFromString (string);
    return genericHashCreate(TEZOS_HASH_BYTES, hash.bytes, GENERIC_HASH_ENCODING_BASE58);
}

static char *
genericTezosNetworkEncodeHash (BRGenericHash hash) {
    size_t len = BRBase58CheckEncode (NULL, 0, hash.bytes, TEZOS_HASH_BYTES);
    char * string = calloc (1, len);
    BRBase58CheckEncode (string, len, hash.bytes, TEZOS_HASH_BYTES);
    return string;
}

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
                                         BRGenericWalletRef wallet,
                                         BRGenericTransferRef transfer,
                                         UInt512 seed)
{
    BRTezosHash lastBlockHash = tezosWalletGetLastBlockHash ((BRTezosWallet) wallet);
    BRTezosTransaction transaction = tezosTransferGetTransaction ((BRTezosTransfer) transfer);
    assert(transaction);
    if (transaction) {
        bool needsReveal = (TEZOS_OP_TRANSACTION == tezosTransactionGetOperationKind(transaction)) && tezosWalletNeedsReveal((BRTezosWallet) wallet);
        tezosTransactionSerializeAndSign (transaction, (BRTezosAccount) account, seed, lastBlockHash, needsReveal);
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
    return uint256Create ((uint64_t) mutez);
}

static BRGenericFeeBasis
genericTezosTransferGetFeeBasis (BRGenericTransferRef transfer) {
    if (NULL == tezosTransferGetTransaction ((BRTezosTransfer) transfer)) {
        BRTezosUnitMutez tezosFee = tezosTransferGetFee ((BRTezosTransfer) transfer);
        return (BRGenericFeeBasis) {
            uint256Create ((uint64_t) tezosFee),
            1,
            0,
            0,
            0
        };
    } else {
        BRTezosFeeBasis feeBasis = tezosTransferGetFeeBasis ((BRTezosTransfer) transfer);
        assert(FEE_BASIS_ESTIMATE == feeBasis.type);
        return (BRGenericFeeBasis) {
            uint256Create ((uint64_t) feeBasis.u.estimate.mutezPerByte),
            (double) feeBasis.u.estimate.sizeInBytes,
            feeBasis.u.estimate.gasLimit,
            feeBasis.u.estimate.storageLimit,
            feeBasis.u.estimate.counter
        };
    }
}

static BRGenericHash
genericTezosTransferGetHash (BRGenericTransferRef transfer) {
    BRTezosHash hash = tezosTransferGetTransactionId ((BRTezosTransfer) transfer);
    return genericHashCreate (sizeof (hash.bytes), hash.bytes, GENERIC_HASH_ENCODING_BASE58);
}

static uint8_t *
genericTezosTransferGetSerialization (BRGenericTransferRef transfer, size_t *bytesCount)
{
    uint8_t * result = NULL;
    *bytesCount = 0;
    BRTezosTransaction transaction = tezosTransferGetTransaction ((BRTezosTransfer) transfer);
    if (transaction) {
        result = tezosTransactionGetSignedBytes (transaction, bytesCount);
    }
    return result;
}

static uint8_t *
genericTezosTransferGetSerializationForFeeEstimation (BRGenericTransferRef transfer,
                                                      BRGenericWalletRef wallet,
                                                      BRGenericAccountRef account,
                                                      size_t *bytesCount)
{
    BRTezosHash lastBlockHash = tezosWalletGetLastBlockHash ((BRTezosWallet) wallet);
    BRTezosTransaction transaction = tezosTransferGetTransaction ((BRTezosTransfer) transfer);
    assert(transaction);
    bool needsReveal = (TEZOS_OP_TRANSACTION == tezosTransactionGetOperationKind(transaction)) && tezosWalletNeedsReveal((BRTezosWallet) wallet);
    
    tezosTransactionSerializeForFeeEstimation (transaction,
                                               (BRTezosAccount) account,
                                               lastBlockHash,
                                               needsReveal);
    
    uint8_t * result = NULL;
    *bytesCount = 0;
    if (transaction) {
        result = tezosTransactionGetSignedBytes (transaction, bytesCount);
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
genericTezosWalletGetBalance (BRGenericWalletRef wallet, int *negative) {
    BRTezosUnitMutez balance = tezosWalletGetBalance ((BRTezosWallet) wallet);
    
    *negative = balance < 0;
    if (*negative) balance = -balance;

    return uint256Create ((uint64_t) balance);
}

static UInt256
genericTezosWalletGetBalanceLimit (BRGenericWalletRef wallet,
                                   int asMaximum,
                                   int *hasLimit) {
    return uint256Create ((uint64_t) tezosWalletGetBalanceLimit ((BRTezosWallet) wallet, asMaximum, hasLimit));
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

static void
genericTezosWalletUpdTransfer (BRGenericWalletRef wallet,
                               OwnershipKept BRGenericTransferRef transfer) {
    tezosWalletUpdateTransfer ((BRTezosWallet) wallet, (BRTezosTransfer) transfer);
}

#define FIELD_OPTION_DELEGATION_OP         "DelegationOp"

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
    BRTezosUnitMutez mutez = (BRTezosUnitMutez) amount.u64[0];
    int64_t counter = tezosWalletGetCounter ((BRTezosWallet) wallet);
    
    BRTezosFeeBasis feeBasis;
    if (0 == estimatedFeeBasis.counter) { // default fee for estimation
        feeBasis = tezosDefaultFeeBasis ((int64_t)estimatedFeeBasis.pricePerCostFactor.u64[0]);
    } else {
        feeBasis.type = FEE_BASIS_ESTIMATE;
        feeBasis.u.estimate.mutezPerByte = (int64_t)estimatedFeeBasis.pricePerCostFactor.u64[0];
        feeBasis.u.estimate.sizeInBytes = (size_t)estimatedFeeBasis.costFactor;
        feeBasis.u.estimate.gasLimit = estimatedFeeBasis.gasLimit;
        feeBasis.u.estimate.storageLimit = estimatedFeeBasis.storageLimit;
        feeBasis.u.estimate.counter = estimatedFeeBasis.counter;
    }
    
    bool delegationOp = false;
    
    for (size_t index = 0; index < attributesCount; index++) {
        BRGenericTransferAttribute attribute = attributes[index];
        if (NULL != genTransferAttributeGetVal(attribute)) {
            if (genericTezosCompareFieldOption (genTransferAttributeGetKey(attribute), FIELD_OPTION_DELEGATION_OP)) {
                uint op;
                sscanf (genTransferAttributeGetVal(attribute), "%u", &op);
                delegationOp = (op == 1);
            }
        }
    }
    
    BRTezosTransfer transfer = tezosTransferCreateNew (source,
                                                       (BRTezosAddress) target,
                                                       mutez,
                                                       feeBasis,
                                                       counter,
                                                       delegationOp);
    tezosAddressFree(source);
    
    return (BRGenericTransferRef) transfer;
}

static BRGenericFeeBasis
genericTezosWalletEstimateFeeBasis (BRGenericWalletRef wallet,
                                    BRGenericAddressRef address,
                                    UInt256 amount,
                                    UInt256 pricePerCostFactor) {
    BRTezosUnitMutez mutezPerByte = (BRTezosUnitMutez) pricePerCostFactor.u64[0];
    BRTezosFeeBasis defaultFeeBasis = tezosDefaultFeeBasis(mutezPerByte);
    BRTezosUnitMutez defaultFee = tezosFeeBasisGetFee (&defaultFeeBasis);
    return (BRGenericFeeBasis) {
        uint256Create ((uint64_t) defaultFee),
        1,
        0,
        0,
        0
    };
}

static const char **
genericTezosWalletGetTransactionAttributeKeys (BRGenericWalletRef wallet,
                                               BRGenericAddressRef address,
                                               int asRequired,
                                               size_t *count) {
    
    static size_t requiredCount = 0;
    static const char **requiredNames = NULL;
    
    static size_t optionalCount = 1;
    static const char *optionalNames[] = {
        FIELD_OPTION_DELEGATION_OP
    };
    
    if (asRequired) { *count = requiredCount; return requiredNames; }
    else {            *count = optionalCount; return optionalNames; }
}

static int
genericTezosWalletValidateTransactionAttribute (BRGenericWalletRef wallet,
                                                BRGenericTransferAttribute attribute) {
    const char *key = genTransferAttributeGetKey (attribute);
    const char *val = genTransferAttributeGetVal (attribute);
    
    // If attribute.value is NULL, we validate unless the attribute.value is required.
    if (NULL == val) return !genTransferAttributeIsRequired(attribute);
    
    if (genericTezosCompareFieldOption (key, FIELD_OPTION_DELEGATION_OP)) {
        // expect 0 or 1
        char *end = NULL;
        errno = 0;
        uintmax_t tag = strtoumax (val, &end, 10);
        return (ERANGE != errno && EINVAL != errno && '\0' == end[0] && tag >= 0 && tag <= 1);
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
    BRTezosHash txId = tezosHashFromString (hash);
    
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

static const char *
cwmLookupAttributeValueForKey (const char *key, size_t count, const char **keys, const char **vals) {
    for (size_t index = 0; index < count; index++)
        if (0 == strcasecmp (key, keys[index]))
            return vals[index];
    return NULL;
}

static uint64_t
cwmParseUInt64 (const char *string, bool *error) {
    if (!string) { *error = true; return 0; }
    return strtoull(string, NULL, 0);
}

static BRGenericFeeBasis
genericTezosWalletManagerRecoverFeeBasisFromEstimate (BRGenericManager gwm,
                                                      BRGenericFeeBasis initialFeeBasis,
                                                      size_t attributesCount,
                                                      OwnershipKept const char **attributeKeys,
                                                      OwnershipKept const char **attributeVals) {
    bool parseError;
    
    int64_t gasUsed = (int64_t) cwmParseUInt64 (cwmLookupAttributeValueForKey ("consumed_gas", attributesCount, attributeKeys, attributeVals), &parseError);
    int64_t storageUsed = (int64_t) cwmParseUInt64 (cwmLookupAttributeValueForKey ("storage_size", attributesCount, attributeKeys, attributeVals), &parseError);
    int64_t counter = (int64_t) cwmParseUInt64 (cwmLookupAttributeValueForKey ("counter", attributesCount, attributeKeys, attributeVals), &parseError);
    // increment counter
    counter += 1;
    // add 10% padding to gas/storage limits
    gasUsed = (int64_t)(gasUsed * 1.1);
    storageUsed = (int64_t)(storageUsed * 1.1);
    BRTezosUnitMutez mutezPerByte = (BRTezosUnitMutez) (initialFeeBasis.pricePerCostFactor.u64[0]) / 1000; // given as nanotez/byte
    // get the serialized txn size from the estimation payload
    size_t sizeInBytes = (size_t) initialFeeBasis.costFactor;
    
    return (BRGenericFeeBasis) {
        uint256Create((uint64_t) mutezPerByte),
        (double) sizeInBytes,
        gasUsed,
        storageUsed,
        counter
    };
}

// MARK: - Generic Handlers

struct BRGenericHandersRecord genericTezosHandlersRecord = {
    CRYPTO_NETWORK_TYPE_XTZ,
    { // Network
        genericTezosNetworkCreateHashFromString,
        genericTezosNetworkEncodeHash
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
        genericTezosTransferGetSerializationForFeeEstimation
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
        genericTezosWalletUpdTransfer,
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
        genericTezosWalletManagerRecoverFeeBasisFromEstimate
    },
};

const BRGenericHandlers genericTezosHandlers = &genericTezosHandlersRecord;
