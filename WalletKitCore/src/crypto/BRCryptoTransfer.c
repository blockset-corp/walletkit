//
//  BRCryptoTransfer.c
//  BRCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "support/BROSCompat.h"

#include "BRCryptoTransferP.h"

#include "BRCryptoBase.h"
#include "BRCryptoHashP.h"
#include "BRCryptoAddressP.h"
#include "BRCryptoAmountP.h"
#include "BRCryptoFeeBasisP.h"

#include "BRCryptoHandlersP.h"

/// MARK: - Transfer State Type

IMPLEMENT_CRYPTO_GIVE_TAKE (BRCryptoTransferState, cryptoTransferState)

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

static void
cryptoTransferStateRelease (BRCryptoTransferState state);

static BRCryptoTransferState
cryptoTransferStateCreate (BRCryptoTransferStateType type) {
    BRCryptoTransferState state = calloc (1, sizeof (struct BRCryptoTransferStateRecord));

    state->type  = type;
    state->ref = CRYPTO_REF_ASSIGN (cryptoTransferStateRelease);

    return state;
}

extern BRCryptoTransferStateType
cryptoTransferStateGetType (BRCryptoTransferState state) {
    return state->type;
}

extern BRCryptoTransferState
cryptoTransferStateInit (BRCryptoTransferStateType type) {
    assert (CRYPTO_TRANSFER_STATE_INCLUDED != type &&
            CRYPTO_TRANSFER_STATE_ERRORED  != type);

    return cryptoTransferStateCreate(type);
}

extern BRCryptoTransferState
cryptoTransferStateIncludedInit (uint64_t blockNumber,
                                 uint64_t transactionIndex,
                                 uint64_t blockTimestamp,
                                 OwnershipKept BRCryptoFeeBasis feeBasis,
                                 BRCryptoBoolean success,
                                 const char *error) {
    BRCryptoTransferState state = cryptoTransferStateCreate (CRYPTO_TRANSFER_STATE_INCLUDED);

    state->u.included.blockNumber = blockNumber;
    state->u.included.transactionIndex = transactionIndex;
    state->u.included.timestamp = blockTimestamp;
    state->u.included.feeBasis  = cryptoFeeBasisTake(feeBasis);
    state->u.included.success   = success;

    memset (state->u.included.error, 0, CRYPTO_TRANSFER_INCLUDED_ERROR_SIZE + 1);
    if (CRYPTO_FALSE == success)
        strlcpy (state->u.included.error,
                 (NULL == error ? "unknown error" : error),
                 CRYPTO_TRANSFER_INCLUDED_ERROR_SIZE + 1);

    return state;
}

extern BRCryptoTransferState
cryptoTransferStateErroredInit (BRCryptoTransferSubmitError error) {
    BRCryptoTransferState state = cryptoTransferStateCreate (CRYPTO_TRANSFER_STATE_ERRORED);

    state->u.errored.error = error;

    return state;
}

static void
cryptoTransferStateRelease (BRCryptoTransferState state) {
    switch (state->type) {
        case CRYPTO_TRANSFER_STATE_INCLUDED:
            cryptoFeeBasisGive (state->u.included.feeBasis);
            break;

        default:
            break;
    }

    memset (state, 0, sizeof(struct BRCryptoTransferStateRecord));
    free (state);
}

private_extern bool
cryptoTransferStateIsEqual (const BRCryptoTransferState s1,
                            const BRCryptoTransferState s2) {
    if (s1->type != s2->type) return false;

    switch (s1->type) {
        case CRYPTO_TRANSFER_STATE_INCLUDED:
            return (s1->u.included.blockNumber      == s2->u.included.blockNumber      &&
                    s1->u.included.transactionIndex == s2->u.included.transactionIndex &&
                    s1->u.included.timestamp        == s2->u.included.timestamp        &&
                    CRYPTO_TRUE == cryptoFeeBasisIsEqual (s1->u.included.feeBasis, s2->u.included.feeBasis) &&
                    s1->u.included.success          == s2->u.included.success);

        case CRYPTO_TRANSFER_STATE_ERRORED:
            return cryptoTransferSubmitErrorIsEqual (&s1->u.errored.error, &s2->u.errored.error);

        default:
            return true;
    }
}

extern bool
cryptoTransferStateExtractIncluded (BRCryptoTransferState state,
                                    uint64_t *blockNumber,
                                    uint64_t *blockTimestamp,
                                    uint64_t *transactionIndex,
                                    BRCryptoFeeBasis *feeBasis,
                                    BRCryptoBoolean  *success,
                                    char **error) {
    if (CRYPTO_TRANSFER_STATE_INCLUDED != state->type) return false;

    if (NULL != blockNumber     ) *blockNumber      = state->u.included.blockNumber;
    if (NULL != blockTimestamp  ) *blockTimestamp   = state->u.included.timestamp;
    if (NULL != transactionIndex) *transactionIndex = state->u.included.transactionIndex;
    if (NULL != feeBasis        ) *feeBasis         = cryptoFeeBasisTake(state->u.included.feeBasis);
    if (NULL != success         ) *success          = state->u.included.success;
    if (NULL != error           ) *error            = (CRYPTO_TRUE == state->u.included.success
                                                       ? NULL
                                                       : strdup (state->u.included.error));

    return true;
}

extern bool
cryptoTransferStateExtractError (BRCryptoTransferState state,
                                 BRCryptoTransferSubmitError *error) {
    if (CRYPTO_TRANSFER_STATE_ERRORED != state->type) return false;

    if (NULL != error) *error = state->u.errored.error;

    return true;
}

/// MARK: Transfer

IMPLEMENT_CRYPTO_GIVE_TAKE (BRCryptoTransfer, cryptoTransfer)

extern BRCryptoTransfer // OwnershipKept, all arguments
cryptoTransferAllocAndInit (size_t sizeInBytes,
                            BRCryptoBlockChainType type,
                            BRCryptoTransferListener listener,
                            BRCryptoUnit unit,
                            BRCryptoUnit unitForFee,
                            BRCryptoFeeBasis feeBasisEstimated,
                            BRCryptoAmount amount,
                            BRCryptoTransferDirection direction,
                            BRCryptoAddress sourceAddress,
                            BRCryptoAddress targetAddress,
                            BRCryptoTransferState state,
                            BRCryptoTransferCreateContext  createContext,
                            BRCryptoTransferCreateCallback createCallback) {
    assert (sizeInBytes >= sizeof (struct BRCryptoTransferRecord));
    BRCryptoTransfer transfer = calloc (1, sizeInBytes);

    transfer->type  = type;
    transfer->handlers = cryptoHandlersLookup(type)->transfer;
    transfer->sizeInBytes = sizeInBytes;

    transfer->listener   = listener;
    transfer->unit       = cryptoUnitTake(unit);
    transfer->unitForFee = cryptoUnitTake(unitForFee);
    transfer->feeBasisEstimated = cryptoFeeBasisTake (feeBasisEstimated);
    
    transfer->amount = cryptoAmountTake (amount);
    transfer->direction = direction;
    
    transfer->sourceAddress = cryptoAddressTake (sourceAddress);
    transfer->targetAddress = cryptoAddressTake (targetAddress);
    transfer->state         = cryptoTransferStateTake (state);

    array_new (transfer->attributes, 1);

    transfer->ref = CRYPTO_REF_ASSIGN (cryptoTransferRelease);

    pthread_mutex_init_brd (&transfer->lock, PTHREAD_MUTEX_NORMAL);

    if (NULL != createContext) createCallback (createContext, transfer);
    
    cryptoTransferGenerateEvent (transfer, (BRCryptoTransferEvent) {
        CRYPTO_TRANSFER_EVENT_CREATED
    });

    return transfer;
}

static void
cryptoTransferRelease (BRCryptoTransfer transfer) {
    pthread_mutex_lock (&transfer->lock);

    if (NULL != transfer->identifier) free (transfer->identifier);
    cryptoAddressGive (transfer->sourceAddress);
    cryptoAddressGive (transfer->targetAddress);
    cryptoUnitGive (transfer->unit);
    cryptoUnitGive (transfer->unitForFee);
    cryptoTransferStateGive (transfer->state);
    cryptoFeeBasisGive (transfer->feeBasisEstimated);
    cryptoAmountGive (transfer->amount);

    array_free_all(transfer->attributes, cryptoTransferAttributeGive);

    transfer->handlers->release (transfer);

    pthread_mutex_unlock  (&transfer->lock);
    pthread_mutex_destroy (&transfer->lock);

//    cryptoTransferGenerateEvent (transfer, (BRCryptoTransferEvent) {
//        CRYPTO_TRANSFER_EVENT_DELETED
//    });

    memset (transfer, 0, sizeof(*transfer));
    free (transfer);
}

private_extern BRCryptoBlockChainType
cryptoTransferGetType (BRCryptoTransfer transfer) {
    return transfer->type;
}

extern BRCryptoAddress
cryptoTransferGetSourceAddress (BRCryptoTransfer transfer) {
    return cryptoAddressTake (transfer->sourceAddress);
}

extern BRCryptoAddress
cryptoTransferGetTargetAddress (BRCryptoTransfer transfer) {
    return cryptoAddressTake (transfer->targetAddress);
}

static BRCryptoAmount
cryptoTransferGetAmountAsSign (BRCryptoTransfer transfer, BRCryptoBoolean isNegative) {
    return NULL == transfer->amount ? NULL : cryptoAmountCreate (cryptoAmountGetUnit(transfer->amount),
                                                                 isNegative,
                                                                 cryptoAmountGetValue(transfer->amount));
}

extern BRCryptoAmount
cryptoTransferGetAmount (BRCryptoTransfer transfer) {
    return cryptoAmountTake (transfer->amount);
}

private_extern BRCryptoAmount
cryptoTransferGetAmountDirectedInternal (BRCryptoTransfer transfer,
                                         BRCryptoBoolean  respectSuccess) {
    BRCryptoAmount   amount;

    // If the transfer is included but has an error, then the amountDirected is zero.
    BRCryptoBoolean success = CRYPTO_TRUE;
    if (CRYPTO_TRUE == respectSuccess &&
        cryptoTransferStateExtractIncluded (transfer->state, NULL, NULL, NULL, NULL, &success, NULL) &&
        CRYPTO_FALSE == success)
        return cryptoAmountCreateInteger(0, transfer->unit);

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
cryptoTransferGetAmountDirected (BRCryptoTransfer transfer) {
    return cryptoTransferGetAmountDirectedInternal (transfer, CRYPTO_TRUE);
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
                             size_t attributesCount,
                             OwnershipKept BRCryptoTransferAttribute *attributes) {
    pthread_mutex_lock (&transfer->lock);

    // Give existing attributes and empty `transfer->attributes`
    for (size_t index = 0; index < array_count(transfer->attributes); index++)
        cryptoTransferAttributeGive (transfer->attributes[index]);
    array_clear(transfer->attributes);

    if (NULL != attributes)
        // Take new attributes.
        for (size_t index = 0; index < attributesCount; index++)
            array_add (transfer->attributes, cryptoTransferAttributeTake (attributes[index]));
    pthread_mutex_unlock (&transfer->lock);
}


extern BRCryptoTransferStateType
cryptoTransferGetStateType (BRCryptoTransfer transfer) {
    pthread_mutex_lock (&transfer->lock);
    BRCryptoTransferStateType type = transfer->state->type;
    pthread_mutex_unlock (&transfer->lock);
    return type;
}

extern BRCryptoTransferState
cryptoTransferGetState (BRCryptoTransfer transfer) {
    pthread_mutex_lock (&transfer->lock);
    BRCryptoTransferState state = cryptoTransferStateTake (transfer->state);
    pthread_mutex_unlock (&transfer->lock);
    return state;
}

private_extern void
cryptoTransferSetState (BRCryptoTransfer transfer,
                        BRCryptoTransferState state) {
    BRCryptoTransferState newState = cryptoTransferStateTake (state);

    pthread_mutex_lock (&transfer->lock);
    BRCryptoTransferState oldState = transfer->state;
    transfer->state = newState;
    pthread_mutex_unlock (&transfer->lock);

    if (!cryptoTransferStateIsEqual (oldState, newState)) {
        // A Hack: Instead Wallet shouild listen for CRYPTO_TRANSFER_EVENT_CHANGED
        if (NULL != transfer->listener.transferChangedCallback)
            transfer->listener.transferChangedCallback (transfer->listener.wallet, transfer, newState);

        cryptoTransferGenerateEvent (transfer, (BRCryptoTransferEvent) {
            CRYPTO_TRANSFER_EVENT_CHANGED,
            { .state = {
                cryptoTransferStateTake (oldState),
                cryptoTransferStateTake (newState) }}
        });
    }
    
    cryptoTransferStateGive (oldState);
}

extern BRCryptoTransferDirection
cryptoTransferGetDirection (BRCryptoTransfer transfer) {
    return transfer->direction;
}

extern const char *
cryptoTransferGetIdentifier (BRCryptoTransfer transfer) {
    // Lazy compute the `identifier`
    if (NULL == transfer->identifier) {

        // If there is a transfer specific `updateIdentifer`, then invoke it.  Think 'HBAR'
        if (NULL != transfer->handlers->updateIdentifier)
            transfer->handlers->updateIdentifier (transfer);
        
        // Otherwise, base the identifier on the string representation of the hash; which will
        // exist once the transfer is signed.  Except in certain cases; think 'HBAR'.
        else {
            switch (transfer->state->type) {
                case CRYPTO_TRANSFER_STATE_CREATED:
                    break;

                case CRYPTO_TRANSFER_STATE_SIGNED:
                case CRYPTO_TRANSFER_STATE_SUBMITTED:
                case CRYPTO_TRANSFER_STATE_INCLUDED:
                case CRYPTO_TRANSFER_STATE_ERRORED: {
                    //
                    // Note that BTC segwit transaction have a `txHash` and a `wtxHash`; BTC nodes
                    // call the `txHash` the `hash` and the `wtxHash` the `identifier`.  WE DO NOT
                    // adopt that definition of `identifer`.  In WalletKit a BTC Transfer's `hash`
                    // and `identifer` are both the string representation of the `txHash`.  See
                    // BRTransaction.{hc}
                    //
                    BRCryptoHash hash = cryptoTransferGetHash (transfer);
                    pthread_mutex_lock (&transfer->lock);
                    transfer->identifier = cryptoHashEncodeString(hash);
                    pthread_mutex_unlock (&transfer->lock);
                    cryptoHashGive(hash);
                    break;
                }

                case CRYPTO_TRANSFER_STATE_DELETED:
                    break;
            }
        }
    }

    return transfer->identifier;
}

extern BRCryptoHash
cryptoTransferGetHash (BRCryptoTransfer transfer) {
    pthread_mutex_lock (&transfer->lock);
    BRCryptoHash hash = transfer->handlers->getHash (transfer);
    pthread_mutex_unlock (&transfer->lock);

    return hash;
}

extern BRCryptoBoolean
cryptoTransferSetHash (BRCryptoTransfer transfer,
                       OwnershipKept BRCryptoHash hash) {
    pthread_mutex_lock (&transfer->lock);
    bool changed = (NULL != transfer->handlers->setHash && transfer->handlers->setHash (transfer, hash));
    pthread_mutex_unlock (&transfer->lock);

    return AS_CRYPTO_BOOLEAN(changed);
}

extern BRCryptoFeeBasis
cryptoTransferGetEstimatedFeeBasis (BRCryptoTransfer transfer) {
    return cryptoFeeBasisTake (transfer->feeBasisEstimated);
}

private_extern BRCryptoAmount
cryptoTransferGetEstimatedFee (BRCryptoTransfer transfer) {
   return (NULL != transfer->feeBasisEstimated
            ? cryptoFeeBasisGetFee (transfer->feeBasisEstimated)
            : NULL);
}

extern BRCryptoFeeBasis
cryptoTransferGetConfirmedFeeBasis (BRCryptoTransfer transfer) {
    pthread_mutex_lock (&transfer->lock);
    BRCryptoFeeBasis feeBasisConfirmed = (CRYPTO_TRANSFER_STATE_INCLUDED == transfer->state->type
                                          ? cryptoFeeBasisTake (transfer->state->u.included.feeBasis)
                                          : NULL);
    pthread_mutex_unlock (&transfer->lock);

    return feeBasisConfirmed;
}

private_extern BRCryptoAmount
cryptoTransferGetConfirmedFee (BRCryptoTransfer transfer) {
    pthread_mutex_lock (&transfer->lock);
    BRCryptoAmount amount = ((CRYPTO_TRANSFER_STATE_INCLUDED == transfer->state->type &&
                              NULL != transfer->state->u.included.feeBasis)
                             ? cryptoFeeBasisGetFee (transfer->state->u.included.feeBasis)
                             : NULL);
    pthread_mutex_unlock (&transfer->lock);

    return amount;
}

private_extern BRCryptoFeeBasis
cryptoTransferGetFeeBasis (BRCryptoTransfer transfer) {
    pthread_mutex_lock (&transfer->lock);
    BRCryptoFeeBasis feeBasis = cryptoFeeBasisTake (CRYPTO_TRANSFER_STATE_INCLUDED == transfer->state->type
                                                    ? transfer->state->u.included.feeBasis
                                                    : transfer->feeBasisEstimated);
    pthread_mutex_unlock (&transfer->lock);

    return feeBasis;
}

extern BRCryptoAmount
cryptoTransferGetFee (BRCryptoTransfer transfer) {
    pthread_mutex_lock (&transfer->lock);
    BRCryptoFeeBasis feeBasis = (CRYPTO_TRANSFER_STATE_INCLUDED == transfer->state->type
                                 ? transfer->state->u.included.feeBasis
                                 : transfer->feeBasisEstimated);

    BRCryptoAmount amount = (NULL == feeBasis ? NULL : cryptoFeeBasisGetFee (feeBasis));
    pthread_mutex_unlock (&transfer->lock);

    return amount;
}

extern uint8_t *
cryptoTransferSerializeForSubmission (BRCryptoTransfer transfer,
                                      BRCryptoNetwork  network,
                                      size_t *serializationCount) {
    assert (NULL != serializationCount);
    return transfer->handlers->serialize (transfer, network, CRYPTO_TRUE, serializationCount);
}

extern uint8_t *
cryptoTransferSerializeForFeeEstimation (BRCryptoTransfer transfer,
                                         BRCryptoNetwork  network,
                                         size_t *bytesCount) {
    assert (NULL != bytesCount);
    return (NULL != transfer->handlers->getBytesForFeeEstimate
            ? transfer->handlers->getBytesForFeeEstimate (transfer, network, bytesCount)
            : transfer->handlers->serialize (transfer, network, CRYPTO_FALSE, bytesCount));
}

extern BRCryptoBoolean
cryptoTransferEqual (BRCryptoTransfer t1, BRCryptoTransfer t2) {
    if (t1 == t2) return CRYPTO_TRUE;
    if (t1->type != t2->type) return CRYPTO_FALSE;

    pthread_mutex_lock (&t1->lock);
    const char *t1ID = t1->identifier;
    pthread_mutex_unlock (&t1->lock);

    pthread_mutex_lock (&t2->lock);
    const char *t2ID = t2->identifier;
    pthread_mutex_unlock (&t2->lock);

    if (NULL != t1ID && NULL != t2ID && 0 == strcmp (t1ID, t2ID)) return CRYPTO_TRUE;

    return AS_CRYPTO_BOOLEAN (t1->handlers->isEqual (t1, t2));
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
    if (state1->type != CRYPTO_TRANSFER_STATE_INCLUDED &&
        state2->type != CRYPTO_TRANSFER_STATE_INCLUDED) {
        // we don't have anything to sort on other than identity
        compareValue = (uintptr_t) transfer1 > (uintptr_t) transfer2 ?
            CRYPTO_COMPARE_GT : CRYPTO_COMPARE_LT;

    // transfer1 is NOT included (and transfer2 is)
    } else if (state1->type != CRYPTO_TRANSFER_STATE_INCLUDED) {
        // return "greater than" for transfer1
        compareValue = CRYPTO_COMPARE_GT;

    // transfer2 is NOT included (and transfer1 is)
    } else if (state2->type != CRYPTO_TRANSFER_STATE_INCLUDED) {
        // return "lesser than" for transfer1
        compareValue = CRYPTO_COMPARE_LT;

    // both are included, check if the timestamp differs
    } else if (state1->u.included.timestamp != state2->u.included.timestamp) {
        // return based on the greater timestamp
        compareValue = state1->u.included.timestamp > state2->u.included.timestamp ?
            CRYPTO_COMPARE_GT : CRYPTO_COMPARE_LT;

    // both are included and have the same timestamp, check if the block differs
    } else if (state1->u.included.blockNumber != state2->u.included.blockNumber) {
        // return based on the greater block number
        compareValue = state1->u.included.blockNumber > state2->u.included.blockNumber ?
            CRYPTO_COMPARE_GT : CRYPTO_COMPARE_LT;

    // both are included and have the same timestamp and block, check if the index differs
    } else if (state1->u.included.transactionIndex != state2->u.included.transactionIndex) {
        // return based on the greater index
        compareValue = state1->u.included.transactionIndex > state2->u.included.transactionIndex ?
            CRYPTO_COMPARE_GT : CRYPTO_COMPARE_LT;

    // both are included and have the same timestamp, block and index
    } else {
        // we are out of differentiators, return "equal"
        compareValue = CRYPTO_COMPARE_EQ;
    }

    // clean up on the way out
    cryptoTransferStateGive (state1);
    cryptoTransferStateGive (state2);

    return compareValue;
}

extern void
cryptoTransferExtractBlobAsBTC (BRCryptoTransfer transfer,
                                uint8_t **bytes,
                                size_t   *bytesCount,
                                uint32_t *blockHeight,
                                uint32_t *timestamp) {
    #ifdef REFACTOR
    assert (NULL != bytes && NULL != bytesCount);

    BRTransaction *tx = cryptoTransferAsBTC (transfer);

    *bytesCount = BRTransactionSerialize (tx, NULL, 0);
    *bytes = malloc (*bytesCount);
    BRTransactionSerialize (tx, *bytes, *bytesCount);

    if (NULL != blockHeight) *blockHeight = tx->blockHeight;
    if (NULL != timestamp)   *timestamp   = tx->timestamp;
    #endif
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

extern bool
cryptoTransferSubmitErrorIsEqual (const BRCryptoTransferSubmitError *e1,
                                  const BRCryptoTransferSubmitError *e2) {
    return (e1->type == e2->type &&
            (e1->type != CRYPTO_TRANSFER_SUBMIT_ERROR_POSIX ||
             e1->u.posix.errnum == e2->u.posix.errnum));
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

private_extern void
cryptoTransferAttributeArrayRelease (BRArrayOf(BRCryptoTransferAttribute) attributes) {
    if (NULL == attributes) return;
    array_free_all (attributes, cryptoTransferAttributeGive);
}

DECLARE_CRYPTO_GIVE_TAKE (BRCryptoTransferAttribute, cryptoTransferAttribute);
