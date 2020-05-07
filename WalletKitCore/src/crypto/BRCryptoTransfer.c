//
//  BRCryptoTransfer.c
//  BRCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "BRCryptoTransferP.h"

#include "BRCryptoBase.h"
#include "BRCryptoHashP.h"
#include "BRCryptoAddressP.h"
#include "BRCryptoAmountP.h"
#include "BRCryptoFeeBasisP.h"
#include "BRCryptoGenericP.h"

/// MARK: - Transfer State Type

extern const char *
cryptoTransferStateTypeString (BRCryptoTransferStateType type) {
    static const char *strings[] = {
        "CRYPTO_TRANSFER_STATE_CREATED",
        "CRYPTO_TRANSFER_STATE_SIGNED",
        "CRYPTO_TRANSFER_STATE_SUBMITTED",
        "CRYPTO_TRANSFER_STATE_INCLUDED",
        "CRYPTO_TRANSFER_STATE_ERRORED",
        "CRYPTO_TRANSFER_STATE_DELETED",
    };
    assert (CRYPTO_TRANSFER_EVENT_CREATED <= type && type <= CRYPTO_TRANSFER_STATE_DELETED);
    return strings[type];
}

/// MARK: Transfer

IMPLEMENT_CRYPTO_GIVE_TAKE (BRCryptoTransfer, cryptoTransfer)

extern BRCryptoTransfer
cryptoTransferAllocAndInit (size_t sizeInBytes,
                            BRCryptoBlockChainType type,
                            BRCryptoUnit unit,
                            BRCryptoUnit unitForFee) {
    assert (sizeInBytes >= sizeof (struct BRCryptoTransferRecord));
    BRCryptoTransfer transfer = calloc (1, sizeInBytes);

    transfer->type  = type;
    transfer->handlers = cryptoGenericHandlersLookup(type)->transfer;
    transfer->sizeInBytes = sizeInBytes;

    transfer->state = (BRCryptoTransferState) { CRYPTO_TRANSFER_STATE_CREATED };
    transfer->unit       = cryptoUnitTake(unit);
    transfer->unitForFee = cryptoUnitTake(unitForFee);
    transfer->feeBasisEstimated = NULL;

    array_new (transfer->attributes, 1);

    transfer->ref = CRYPTO_REF_ASSIGN (cryptoTransferRelease);

    {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);

        pthread_mutex_init(&transfer->lock, &attr);
        pthread_mutexattr_destroy(&attr);
    }

    return transfer;
}

static void
cryptoTransferRelease (BRCryptoTransfer transfer) {
    if (NULL != transfer->sourceAddress) cryptoAddressGive (transfer->sourceAddress);
    if (NULL != transfer->targetAddress) cryptoAddressGive (transfer->targetAddress);
    cryptoUnitGive (transfer->unit);
    cryptoUnitGive (transfer->unitForFee);
    cryptoTransferStateRelease (&transfer->state);
    if (NULL != transfer->feeBasisEstimated) cryptoFeeBasisGive (transfer->feeBasisEstimated);

    array_free_all(transfer->attributes, cryptoTransferAttributeGive);

    transfer->handlers->release (transfer);
//    switch (transfer->type) {
//        case BLOCK_CHAIN_TYPE_BTC:
//            break;
//        case BLOCK_CHAIN_TYPE_ETH:
//            break;
//        case BLOCK_CHAIN_TYPE_GEN:
//            genTransferRelease(transfer->u.gen);
//            break;
//    }

    pthread_mutex_destroy (&transfer->lock);

    memset (transfer, 0, sizeof(*transfer));
    free (transfer);
}

private_extern BRCryptoBlockChainType
cryptoTransferGetType (BRCryptoTransfer transfer) {
    return transfer->type;
}

extern BRCryptoAddress
cryptoTransferGetSourceAddress (BRCryptoTransfer transfer) {
    return NULL == transfer->sourceAddress ? NULL : cryptoAddressTake (transfer->sourceAddress);
}

extern BRCryptoAddress
cryptoTransferGetTargetAddress (BRCryptoTransfer transfer) {
    return NULL == transfer->targetAddress ? NULL : cryptoAddressTake (transfer->targetAddress);
}

static BRCryptoAmount
cryptoTransferGetAmountAsSign (BRCryptoTransfer transfer, BRCryptoBoolean isNegative) {
    return transfer->handlers->getAmountAsSign (transfer, isNegative);
}


extern BRCryptoAmount
cryptoTransferGetAmount (BRCryptoTransfer transfer) {
    return cryptoTransferGetAmountAsSign (transfer, CRYPTO_FALSE);
}

extern BRCryptoAmount
cryptoTransferGetAmountDirected (BRCryptoTransfer transfer) {
    BRCryptoAmount   amount;

    switch (cryptoTransferGetDirection(transfer)) {
        case CRYPTO_TRANSFER_RECOVERED: {
            amount = cryptoAmountCreate (transfer->unit,
                                         CRYPTO_FALSE,
                                         UINT256_ZERO);
            break;
        }

        case CRYPTO_TRANSFER_SENT: {
            amount = cryptoTransferGetAmountAsSign (transfer,
                                                    CRYPTO_TRUE);
            break;
        }

        case CRYPTO_TRANSFER_RECEIVED: {
            amount = cryptoTransferGetAmountAsSign (transfer,
                                                    CRYPTO_FALSE);
            break;
        }
        default: assert(0);
    }

    return amount;
}

extern BRCryptoAmount
cryptoTransferGetAmountDirectedNet (BRCryptoTransfer transfer) {
    BRCryptoAmount amount = cryptoTransferGetAmountDirected (transfer);

    // If the transfer->unit and transfer->unitForFee differ then there is no fee
    if (cryptoUnitIsIdentical (transfer->unit, transfer->unitForFee))
        return amount;

    BRCryptoFeeBasis feeBasis = cryptoTransferGetConfirmedFeeBasis(transfer);
    if (NULL == feeBasis)
        feeBasis = (NULL == transfer->feeBasisEstimated
                    ? NULL
                    : cryptoFeeBasisTake (transfer->feeBasisEstimated));

    // If there is no fee basis, then there is no fee
    if (NULL == feeBasis)
        return amount;

    BRCryptoAmount fee = cryptoFeeBasisGetFee (feeBasis);
    cryptoFeeBasisGive(feeBasis);

    // Simply subtract off the fee.
    BRCryptoAmount amountNet = cryptoAmountSub (amount, fee);

    cryptoAmountGive(fee);
    cryptoAmountGive(amount);

    return amountNet;
}

extern BRCryptoUnit
cryptoTransferGetUnitForAmount (BRCryptoTransfer transfer) {
    return cryptoUnitTake (transfer->unit);
}

extern BRCryptoUnit
cryptoTransferGetUnitForFee (BRCryptoTransfer transfer) {
    return cryptoUnitTake (transfer->unitForFee);
}

extern size_t
cryptoTransferGetAttributeCount (BRCryptoTransfer transfer) {
    pthread_mutex_lock (&transfer->lock);
    size_t count = array_count(transfer->attributes);
    pthread_mutex_unlock (&transfer->lock);
    return count;
}

extern BRCryptoTransferAttribute
cryptoTransferGetAttributeAt (BRCryptoTransfer transfer,
                              size_t index) {
    pthread_mutex_lock (&transfer->lock);
    BRCryptoTransferAttribute attribute = cryptoTransferAttributeTake (transfer->attributes[index]);
    pthread_mutex_unlock (&transfer->lock);
    return attribute;
}

private_extern void
cryptoTransferSetAttributes (BRCryptoTransfer transfer,
                             OwnershipKept BRArrayOf(BRCryptoTransferAttribute) attributes) {
    pthread_mutex_lock (&transfer->lock);

    // Give existing attributes and empty `transfer->attributes`
    for (size_t index = 0; index < array_count(transfer->attributes); index++)
        cryptoTransferAttributeGive (transfer->attributes[index]);
    array_clear(transfer->attributes);

    if (NULL != attributes)
        // Take new attributes.
        for (size_t index = 0; index < array_count(attributes); index++)
            array_add (transfer->attributes, cryptoTransferAttributeTake (attributes[index]));
    pthread_mutex_unlock (&transfer->lock);
}


extern BRCryptoTransferStateType
cryptoTransferGetStateType (BRCryptoTransfer transfer) {
    return transfer->state.type;
}

extern BRCryptoTransferState
cryptoTransferGetState (BRCryptoTransfer transfer) {
    pthread_mutex_lock (&transfer->lock);
    BRCryptoTransferState state = cryptoTransferStateCopy (&transfer->state);
    pthread_mutex_unlock (&transfer->lock);

    return state;
}

private_extern void
cryptoTransferSetState (BRCryptoTransfer transfer,
                        BRCryptoTransferState state) {
    BRCryptoTransferState newState = cryptoTransferStateCopy (&state);

    pthread_mutex_lock (&transfer->lock);
    BRCryptoTransferState oldState = transfer->state;
    transfer->state = newState;
    pthread_mutex_unlock (&transfer->lock);

    cryptoTransferStateRelease (&oldState);
}

extern BRCryptoTransferDirection
cryptoTransferGetDirection (BRCryptoTransfer transfer) {
    return transfer->handlers->getDirection (transfer);
}


extern BRCryptoHash
cryptoTransferGetHash (BRCryptoTransfer transfer) {
    return transfer->handlers->getHash (transfer);
}

extern BRCryptoFeeBasis
cryptoTransferGetEstimatedFeeBasis (BRCryptoTransfer transfer) {
    return (NULL == transfer->feeBasisEstimated ? NULL : cryptoFeeBasisTake (transfer->feeBasisEstimated));
}

extern BRCryptoFeeBasis
cryptoTransferGetConfirmedFeeBasis (BRCryptoTransfer transfer) {
    pthread_mutex_lock (&transfer->lock);
    BRCryptoFeeBasis feeBasisConfirmed = (CRYPTO_TRANSFER_STATE_INCLUDED == transfer->state.type
                                          ? cryptoFeeBasisTake (transfer->state.u.included.feeBasis)
                                          : NULL);
    pthread_mutex_unlock (&transfer->lock);

    return feeBasisConfirmed;
}

extern uint8_t *
cryptoTransferSerializeForSubmission (BRCryptoTransfer transfer,
                                      size_t *serializationCount) {
    assert (NULL != serializationCount);
    return transfer->handlers->serializeForSubmission (transfer, serializationCount);
}

extern BRCryptoBoolean
cryptoTransferEqual (BRCryptoTransfer t1, BRCryptoTransfer t2) {
    return AS_CRYPTO_BOOLEAN (t1 == t2 ||
            (t1->type == t2->type &&
             t1->handlers->isEqual (t1, t2)));
}

extern BRCryptoComparison
cryptoTransferCompare (BRCryptoTransfer transfer1, BRCryptoTransfer transfer2) {
    // early bail when comparing the same transfer
    if (CRYPTO_TRUE == cryptoTransferEqual (transfer1, transfer2)) {
        return CRYPTO_COMPARE_EQ;
    }

    // The algorithm below is captured in the cryptoTransferCompare declaration
    // comments; any changes to this routine must be reflected in that comment
    // and vice versa).
    //
    // The algorithm includes timestamp as a differentiator despite the fact that
    // timestamp is likely derived from the block. Thus, an occurrence where timestamp
    // is different while block value is the same is unlikely. Regardless, this check
    // is included to handle cases where that assumption does not hold.
    //
    // Another reason to include timestamp is if this function were used to order
    // transfers across different wallets. While not anticipated to be a common use
    // case, there is not enough information available in the transfer object to
    // preclude it from happening. Checking on the `type` field is insufficient
    // given that GEN will handle multiple cases. While block number and transaction
    // index are meaningless comparables between wallets, ordering by timestamp
    // does provide some value.

    BRCryptoComparison compareValue;
    BRCryptoTransferState state1 = cryptoTransferGetState (transfer1);
    BRCryptoTransferState state2 = cryptoTransferGetState (transfer2);

    // neither transfer is included
    if (state1.type != CRYPTO_TRANSFER_STATE_INCLUDED &&
        state2.type != CRYPTO_TRANSFER_STATE_INCLUDED) {
        // we don't have anything to sort on other than identity
        compareValue = (uintptr_t) transfer1 > (uintptr_t) transfer2 ?
            CRYPTO_COMPARE_GT : CRYPTO_COMPARE_LT;

    // transfer1 is NOT included (and transfer2 is)
    } else if (state1.type != CRYPTO_TRANSFER_STATE_INCLUDED) {
        // return "greater than" for transfer1
        compareValue = CRYPTO_COMPARE_GT;

    // transfer2 is NOT included (and transfer1 is)
    } else if (state2.type != CRYPTO_TRANSFER_STATE_INCLUDED) {
        // return "lesser than" for transfer1
        compareValue = CRYPTO_COMPARE_LT;

    // both are included, check if the timestamp differs
    } else if (state1.u.included.timestamp != state2.u.included.timestamp) {
        // return based on the greater timestamp
        compareValue = state1.u.included.timestamp > state2.u.included.timestamp ?
            CRYPTO_COMPARE_GT : CRYPTO_COMPARE_LT;

    // both are included and have the same timestamp, check if the block differs
    } else if (state1.u.included.blockNumber != state2.u.included.blockNumber) {
        // return based on the greater block number
        compareValue = state1.u.included.blockNumber > state2.u.included.blockNumber ?
            CRYPTO_COMPARE_GT : CRYPTO_COMPARE_LT;

    // both are included and have the same timestamp and block, check if the index differs
    } else if (state1.u.included.transactionIndex != state2.u.included.transactionIndex) {
        // return based on the greater index
        compareValue = state1.u.included.transactionIndex > state2.u.included.transactionIndex ?
            CRYPTO_COMPARE_GT : CRYPTO_COMPARE_LT;

    // both are included and have the same timestamp, block and index
    } else {
        // we are out of differentiators, return "equal"
        compareValue = CRYPTO_COMPARE_EQ;
    }

    // clean up on the way out
    cryptoTransferStateRelease (&state1);
    cryptoTransferStateRelease (&state2);
    return compareValue;
}

#ifdef REFACTOR
extern void
cryptoTransferExtractBlobAsBTC (BRCryptoTransfer transfer,
                                uint8_t **bytes,
                                size_t   *bytesCount,
                                uint32_t *blockHeight,
                                uint32_t *timestamp) {
    assert (NULL != bytes && NULL != bytesCount);

    BRTransaction *tx = cryptoTransferAsBTC (transfer);

    *bytesCount = BRTransactionSerialize (tx, NULL, 0);
    *bytes = malloc (*bytesCount);
    BRTransactionSerialize (tx, *bytes, *bytesCount);

    if (NULL != blockHeight) *blockHeight = tx->blockHeight;
    if (NULL != timestamp)   *timestamp   = tx->timestamp;
}
#endif

extern BRCryptoTransferState
cryptoTransferStateInit (BRCryptoTransferStateType type) {
    switch (type) {
        case CRYPTO_TRANSFER_STATE_CREATED:
        case CRYPTO_TRANSFER_STATE_DELETED:
        case CRYPTO_TRANSFER_STATE_SIGNED:
        case CRYPTO_TRANSFER_STATE_SUBMITTED: {
            return (BRCryptoTransferState) {
                type
            };
        }
        case CRYPTO_TRANSFER_STATE_INCLUDED:
            assert (0); // if you are hitting this, use cryptoTransferStateIncludedInit!
            return (BRCryptoTransferState) {
                CRYPTO_TRANSFER_STATE_INCLUDED,
                { .included = { 0, 0, 0, NULL }}
            };
        case CRYPTO_TRANSFER_STATE_ERRORED: {
            assert (0); // if you are hitting this, use cryptoTransferStateErroredInit!
            return (BRCryptoTransferState) {
                CRYPTO_TRANSFER_STATE_ERRORED,
                { .errored = { cryptoTransferSubmitErrorUnknown() }}
            };
        }
    }
}

extern BRCryptoTransferState
cryptoTransferStateIncludedInit (uint64_t blockNumber,
                                 uint64_t transactionIndex,
                                 uint64_t timestamp,
                                 BRCryptoFeeBasis feeBasis,
                                 BRCryptoBoolean success,
                                 const char *error) {
    BRCryptoTransferState result = (BRCryptoTransferState) {
        CRYPTO_TRANSFER_STATE_INCLUDED,
        { .included = {
            blockNumber,
            transactionIndex,
            timestamp,
            cryptoFeeBasisTake(feeBasis),
            success
        }}
    };

    memset (result.u.included.error, 0, CRYPTO_TRANSFER_INCLUDED_ERROR_SIZE + 1);
    if (CRYPTO_FALSE == success) {
        strlcpy (result.u.included.error,
                 (NULL == error ? "unknown error" : error),
                 CRYPTO_TRANSFER_INCLUDED_ERROR_SIZE + 1);
    }

    return result;
}

extern BRCryptoTransferState
cryptoTransferStateErroredInit (BRCryptoTransferSubmitError error) {
    return (BRCryptoTransferState) {
        CRYPTO_TRANSFER_STATE_ERRORED,
        { .errored = { error }}
    };
}

extern BRCryptoTransferState
cryptoTransferStateCopy (BRCryptoTransferState *state) {
    BRCryptoTransferState newState = *state;
    switch (state->type) {
        case CRYPTO_TRANSFER_STATE_INCLUDED: {
            if (NULL != newState.u.included.feeBasis) {
                cryptoFeeBasisTake (newState.u.included.feeBasis);
            }
            break;
        }
        case CRYPTO_TRANSFER_STATE_ERRORED:
        case CRYPTO_TRANSFER_STATE_CREATED:
        case CRYPTO_TRANSFER_STATE_DELETED:
        case CRYPTO_TRANSFER_STATE_SIGNED:
        case CRYPTO_TRANSFER_STATE_SUBMITTED:
        default: {
            break;
        }
    }
    return newState;
}

extern void
cryptoTransferStateRelease (BRCryptoTransferState *state) {
    switch (state->type) {
        case CRYPTO_TRANSFER_STATE_INCLUDED: {
            if (NULL != state->u.included.feeBasis) {
                cryptoFeeBasisGive (state->u.included.feeBasis);
            }
            break;
        }
        case CRYPTO_TRANSFER_STATE_ERRORED:
        case CRYPTO_TRANSFER_STATE_CREATED:
        case CRYPTO_TRANSFER_STATE_DELETED:
        case CRYPTO_TRANSFER_STATE_SIGNED:
        case CRYPTO_TRANSFER_STATE_SUBMITTED:
        default: {
            break;
        }
    }

    memset (state, 0, sizeof(*state));
}

extern const char *
cryptoTransferEventTypeString (BRCryptoTransferEventType t) {
    switch (t) {
        case CRYPTO_TRANSFER_EVENT_CREATED:
        return "CRYPTO_TRANSFER_EVENT_CREATED";

        case CRYPTO_TRANSFER_EVENT_CHANGED:
        return "CRYPTO_TRANSFER_EVENT_CHANGED";

        case CRYPTO_TRANSFER_EVENT_DELETED:
        return "CRYPTO_TRANSFER_EVENT_DELETED";
    }
    return "<CRYPTO_TRANSFER_EVENT_TYPE_UNKNOWN>";
}


/// MARK: Transaction Submission Error

// TODO(fix): This should be moved to a more appropriate file (BRTransfer.c/h?)

extern BRCryptoTransferSubmitError
cryptoTransferSubmitErrorUnknown(void) {
    return (BRCryptoTransferSubmitError) {
        CRYPTO_TRANSFER_SUBMIT_ERROR_UNKNOWN
    };
}

extern BRCryptoTransferSubmitError
cryptoTransferSubmitErrorPosix(int errnum) {
    return (BRCryptoTransferSubmitError) {
        CRYPTO_TRANSFER_SUBMIT_ERROR_POSIX,
        { .posix = { errnum } }
    };
}

extern char *
cryptoTransferSubmitErrorGetMessage (BRCryptoTransferSubmitError *e) {
    char *message = NULL;

    switch (e->type) {
        case CRYPTO_TRANSFER_SUBMIT_ERROR_POSIX: {
            if (NULL != (message = strerror (e->u.posix.errnum))) {
                message = strdup (message);
            }
            break;
        }
        default: {
            break;
        }
    }

    return message;
}


/// MARK: - Transfer Attribute

struct BRCryptoTransferAttributeRecord {
    char *key;
    char *value;
    BRCryptoBoolean isRequired;
    BRCryptoRef ref;
};

IMPLEMENT_CRYPTO_GIVE_TAKE (BRCryptoTransferAttribute, cryptoTransferAttribute)

private_extern BRCryptoTransferAttribute
cryptoTransferAttributeCreate (const char *key,
                               const char *val,
                               BRCryptoBoolean isRequired) {
    BRCryptoTransferAttribute attribute = calloc (1, sizeof (struct BRCryptoTransferAttributeRecord));

    attribute->key   = strdup (key);
    attribute->value = (NULL == val ? NULL : strdup (val));
    attribute->isRequired = isRequired;

    attribute->ref = CRYPTO_REF_ASSIGN (cryptoTransferAttributeRelease);

    return attribute;
}

extern BRCryptoTransferAttribute
cryptoTransferAttributeCopy (BRCryptoTransferAttribute attribute) {
    return cryptoTransferAttributeCreate (attribute->key,
                                          attribute->value,
                                          attribute->isRequired);
}

static void
cryptoTransferAttributeRelease (BRCryptoTransferAttribute attribute) {
    free (attribute->key);
    if (NULL != attribute->value) free (attribute->value);
    memset (attribute, 0, sizeof (struct BRCryptoTransferAttributeRecord));
    free (attribute);
}

extern const char *
cryptoTransferAttributeGetKey (BRCryptoTransferAttribute attribute) {
    return attribute->key;
}

extern const char * // nullable
cryptoTransferAttributeGetValue (BRCryptoTransferAttribute attribute) {
    return attribute->value;
}
extern void
cryptoTransferAttributeSetValue (BRCryptoTransferAttribute attribute, const char *value) {
    if (NULL != attribute->value) free (attribute->value);
    attribute->value = (NULL == value ? NULL : strdup (value));
}

extern BRCryptoBoolean
cryptoTransferAttributeIsRequired (BRCryptoTransferAttribute attribute) {
    return attribute->isRequired;
}

DECLARE_CRYPTO_GIVE_TAKE (BRCryptoTransferAttribute, cryptoTransferAttribute);
