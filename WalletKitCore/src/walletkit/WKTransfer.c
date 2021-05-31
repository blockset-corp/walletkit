//
//  WKTransfer.c
//  WalletKitCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "support/BROSCompat.h"

#include "WKTransferP.h"

#include "WKBase.h"
#include "WKHashP.h"
#include "WKAddressP.h"
#include "WKAmountP.h"
#include "WKFeeBasisP.h"

#include "WKHandlersP.h"

/// MARK: - Transfer State Type

IMPLEMENT_WK_GIVE_TAKE (WKTransferState, wkTransferState)

extern const char *
wkTransferStateTypeString (WKTransferStateType type) {
    static const char *strings[] = {
        "WK_TRANSFER_STATE_CREATED",
        "WK_TRANSFER_STATE_SIGNED",
        "WK_TRANSFER_STATE_SUBMITTED",
        "WK_TRANSFER_STATE_INCLUDED",
        "WK_TRANSFER_STATE_ERRORED",
        "WK_TRANSFER_STATE_DELETED",
    };
    assert (WK_TRANSFER_EVENT_CREATED <= type && type <= WK_TRANSFER_STATE_DELETED);
    return strings[type];
}

static void
wkTransferStateRelease (WKTransferState state);

static WKTransferState
wkTransferStateCreate (WKTransferStateType type) {
    WKTransferState state = calloc (1, sizeof (struct WKTransferStateRecord));

    state->type  = type;
    state->ref = WK_REF_ASSIGN (wkTransferStateRelease);

    return state;
}

extern WKTransferStateType
wkTransferStateGetType (WKTransferState state) {
    return state->type;
}

extern WKTransferState
wkTransferStateInit (WKTransferStateType type) {
    assert (WK_TRANSFER_STATE_INCLUDED != type &&
            WK_TRANSFER_STATE_ERRORED  != type);

    return wkTransferStateCreate(type);
}

extern WKTransferState
wkTransferStateIncludedInit (uint64_t blockNumber,
                                 uint64_t transactionIndex,
                                 uint64_t blockTimestamp,
                                 OwnershipKept WKFeeBasis feeBasis,
                                 WKBoolean success,
                                 const char *error) {
    WKTransferState state = wkTransferStateCreate (WK_TRANSFER_STATE_INCLUDED);

    state->u.included.blockNumber = blockNumber;
    state->u.included.transactionIndex = transactionIndex;
    state->u.included.timestamp = blockTimestamp;
    state->u.included.feeBasis  = wkFeeBasisTake(feeBasis);
    state->u.included.success   = success;

    memset (state->u.included.error, 0, WK_TRANSFER_INCLUDED_ERROR_SIZE + 1);
    if (WK_FALSE == success)
        strlcpy (state->u.included.error,
                 (NULL == error ? "unknown error" : error),
                 WK_TRANSFER_INCLUDED_ERROR_SIZE + 1);

    return state;
}

extern WKTransferState
wkTransferStateErroredInit (WKTransferSubmitError error) {
    WKTransferState state = wkTransferStateCreate (WK_TRANSFER_STATE_ERRORED);

    state->u.errored.error = error;

    return state;
}

static void
wkTransferStateRelease (WKTransferState state) {
    switch (state->type) {
        case WK_TRANSFER_STATE_INCLUDED:
            wkFeeBasisGive (state->u.included.feeBasis);
            break;

        default:
            break;
    }

    memset (state, 0, sizeof(struct WKTransferStateRecord));
    free (state);
}

private_extern bool
wkTransferStateIsEqual (const WKTransferState s1,
                            const WKTransferState s2) {
    if (s1->type != s2->type) return false;

    switch (s1->type) {
        case WK_TRANSFER_STATE_INCLUDED:
            return (s1->u.included.blockNumber      == s2->u.included.blockNumber      &&
                    s1->u.included.transactionIndex == s2->u.included.transactionIndex &&
                    s1->u.included.timestamp        == s2->u.included.timestamp        &&
                    WK_TRUE == wkFeeBasisIsEqual (s1->u.included.feeBasis, s2->u.included.feeBasis) &&
                    s1->u.included.success          == s2->u.included.success);

        case WK_TRANSFER_STATE_ERRORED:
            return wkTransferSubmitErrorIsEqual (&s1->u.errored.error, &s2->u.errored.error);

        default:
            return true;
    }
}

extern bool
wkTransferStateExtractIncluded (WKTransferState state,
                                    uint64_t *blockNumber,
                                    uint64_t *blockTimestamp,
                                    uint64_t *transactionIndex,
                                    WKFeeBasis *feeBasis,
                                    WKBoolean  *success,
                                    char **error) {
    if (WK_TRANSFER_STATE_INCLUDED != state->type) return false;

    if (NULL != blockNumber     ) *blockNumber      = state->u.included.blockNumber;
    if (NULL != blockTimestamp  ) *blockTimestamp   = state->u.included.timestamp;
    if (NULL != transactionIndex) *transactionIndex = state->u.included.transactionIndex;
    if (NULL != feeBasis        ) *feeBasis         = wkFeeBasisTake(state->u.included.feeBasis);
    if (NULL != success         ) *success          = state->u.included.success;
    if (NULL != error           ) *error            = (WK_TRUE == state->u.included.success
                                                       ? NULL
                                                       : strdup (state->u.included.error));

    return true;
}

extern bool
wkTransferStateExtractError (WKTransferState state,
                                 WKTransferSubmitError *error) {
    if (WK_TRANSFER_STATE_ERRORED != state->type) return false;

    if (NULL != error) *error = state->u.errored.error;

    return true;
}

/// MARK: Transfer

IMPLEMENT_WK_GIVE_TAKE (WKTransfer, wkTransfer)

extern WKTransfer // OwnershipKept, all arguments
wkTransferAllocAndInit (size_t sizeInBytes,
                            WKNetworkType type,
                            WKTransferListener listener,
                            WKUnit unit,
                            WKUnit unitForFee,
                            WKFeeBasis feeBasisEstimated,
                            WKAmount amount,
                            WKTransferDirection direction,
                            WKAddress sourceAddress,
                            WKAddress targetAddress,
                            WKTransferState state,
                            WKTransferCreateContext  createContext,
                            WKTransferCreateCallback createCallback) {
    assert (sizeInBytes >= sizeof (struct WKTransferRecord));
    WKTransfer transfer = calloc (1, sizeInBytes);

    transfer->type  = type;
    transfer->handlers = wkHandlersLookup(type)->transfer;
    transfer->sizeInBytes = sizeInBytes;

    transfer->listener   = listener;
    transfer->unit       = wkUnitTake(unit);
    transfer->unitForFee = wkUnitTake(unitForFee);
    transfer->feeBasisEstimated = wkFeeBasisTake (feeBasisEstimated);
    
    transfer->amount = wkAmountTake (amount);
    transfer->direction = direction;
    
    transfer->sourceAddress = wkAddressTake (sourceAddress);
    transfer->targetAddress = wkAddressTake (targetAddress);
    transfer->state         = wkTransferStateTake (state);

    array_new (transfer->attributes, 1);

    transfer->ref = WK_REF_ASSIGN (wkTransferRelease);

    pthread_mutex_init_brd (&transfer->lock, PTHREAD_MUTEX_NORMAL_BRD);

    if (NULL != createContext) createCallback (createContext, transfer);
    
    wkTransferGenerateEvent (transfer, (WKTransferEvent) {
        WK_TRANSFER_EVENT_CREATED
    });

    return transfer;
}

static void
wkTransferRelease (WKTransfer transfer) {
    pthread_mutex_lock (&transfer->lock);

    if (NULL != transfer->identifier) free (transfer->identifier);
    wkAddressGive (transfer->sourceAddress);
    wkAddressGive (transfer->targetAddress);
    wkUnitGive (transfer->unit);
    wkUnitGive (transfer->unitForFee);
    wkTransferStateGive (transfer->state);
    wkFeeBasisGive (transfer->feeBasisEstimated);
    wkAmountGive (transfer->amount);

    array_free_all(transfer->attributes, wkTransferAttributeGive);

    transfer->handlers->release (transfer);

    pthread_mutex_unlock  (&transfer->lock);
    pthread_mutex_destroy (&transfer->lock);

//    wkTransferGenerateEvent (transfer, (WKTransferEvent) {
//        WK_TRANSFER_EVENT_DELETED
//    });

    memset (transfer, 0, sizeof(*transfer));
    free (transfer);
}

private_extern WKNetworkType
wkTransferGetType (WKTransfer transfer) {
    return transfer->type;
}

extern WKAddress
wkTransferGetSourceAddress (WKTransfer transfer) {
    return wkAddressTake (transfer->sourceAddress);
}

extern WKAddress
wkTransferGetTargetAddress (WKTransfer transfer) {
    return wkAddressTake (transfer->targetAddress);
}

static WKAmount
wkTransferGetAmountAsSign (WKTransfer transfer, WKBoolean isNegative) {
    return NULL == transfer->amount ? NULL : wkAmountCreate (wkAmountGetUnit(transfer->amount),
                                                                 isNegative,
                                                                 wkAmountGetValue(transfer->amount));
}

extern WKAmount
wkTransferGetAmount (WKTransfer transfer) {
    return wkAmountTake (transfer->amount);
}

private_extern WKAmount
wkTransferGetAmountDirectedInternal (WKTransfer transfer,
                                         WKBoolean  respectSuccess) {
    WKAmount   amount;

    // If the transfer is included but has an error, then the amountDirected is zero.
    WKBoolean success = WK_TRUE;
    if (WK_TRUE == respectSuccess &&
        wkTransferStateExtractIncluded (transfer->state, NULL, NULL, NULL, NULL, &success, NULL) &&
        WK_FALSE == success)
        return wkAmountCreateInteger(0, transfer->unit);

    switch (wkTransferGetDirection(transfer)) {
        case WK_TRANSFER_RECOVERED: {
            amount = wkAmountCreate (transfer->unit,
                                         WK_FALSE,
                                         UINT256_ZERO);
            break;
        }

        case WK_TRANSFER_SENT: {
            amount = wkTransferGetAmountAsSign (transfer,
                                                    WK_TRUE);
            break;
        }

        case WK_TRANSFER_RECEIVED: {
            amount = wkTransferGetAmountAsSign (transfer,
                                                    WK_FALSE);
            break;
        }
        default: assert(0);
    }

    return amount;
}

extern WKAmount
wkTransferGetAmountDirected (WKTransfer transfer) {
    return wkTransferGetAmountDirectedInternal (transfer, WK_TRUE);
}

extern WKUnit
wkTransferGetUnitForAmount (WKTransfer transfer) {
    return wkUnitTake (transfer->unit);
}

extern WKUnit
wkTransferGetUnitForFee (WKTransfer transfer) {
    return wkUnitTake (transfer->unitForFee);
}

extern size_t
wkTransferGetAttributeCount (WKTransfer transfer) {
    pthread_mutex_lock (&transfer->lock);
    size_t count = array_count(transfer->attributes);
    pthread_mutex_unlock (&transfer->lock);
    return count;
}

extern WKTransferAttribute
wkTransferGetAttributeAt (WKTransfer transfer,
                              size_t index) {
    pthread_mutex_lock (&transfer->lock);
    WKTransferAttribute attribute = wkTransferAttributeTake (transfer->attributes[index]);
    pthread_mutex_unlock (&transfer->lock);
    return attribute;
}

private_extern void
wkTransferSetAttributes (WKTransfer transfer,
                             size_t attributesCount,
                             OwnershipKept WKTransferAttribute *attributes) {
    pthread_mutex_lock (&transfer->lock);

    // Give existing attributes and empty `transfer->attributes`
    for (size_t index = 0; index < array_count(transfer->attributes); index++)
        wkTransferAttributeGive (transfer->attributes[index]);
    array_clear(transfer->attributes);

    if (NULL != attributes)
        // Take new attributes.
        for (size_t index = 0; index < attributesCount; index++)
            array_add (transfer->attributes, wkTransferAttributeTake (attributes[index]));
    pthread_mutex_unlock (&transfer->lock);
}


extern WKTransferStateType
wkTransferGetStateType (WKTransfer transfer) {
    pthread_mutex_lock (&transfer->lock);
    WKTransferStateType type = transfer->state->type;
    pthread_mutex_unlock (&transfer->lock);
    return type;
}

extern WKTransferState
wkTransferGetState (WKTransfer transfer) {
    pthread_mutex_lock (&transfer->lock);
    WKTransferState state = wkTransferStateTake (transfer->state);
    pthread_mutex_unlock (&transfer->lock);
    return state;
}

private_extern void
wkTransferSetStateForced (WKTransfer transfer,
                              WKTransferState state,
                              bool forced) {
    WKTransferState newState = wkTransferStateTake (state);

    pthread_mutex_lock (&transfer->lock);
    WKTransferState oldState = transfer->state;
    transfer->state = newState;
    pthread_mutex_unlock (&transfer->lock);

    if (forced || !wkTransferStateIsEqual (oldState, newState)) {
        // A Hack: Instead Wallet shouild listen for WK_TRANSFER_EVENT_CHANGED
        if (NULL != transfer->listener.transferChangedCallback)
            transfer->listener.transferChangedCallback (transfer->listener.wallet, transfer, newState);

        wkTransferGenerateEvent (transfer, (WKTransferEvent) {
            WK_TRANSFER_EVENT_CHANGED,
            { .state = {
                wkTransferStateTake (oldState),
                wkTransferStateTake (newState) }}
        });
    }
    
    wkTransferStateGive (oldState);
}

extern WKTransferDirection
wkTransferGetDirection (WKTransfer transfer) {
    return transfer->direction;
}

extern const char *
wkTransferGetIdentifier (WKTransfer transfer) {
    // Lazy compute the `identifier`
    if (NULL == transfer->identifier) {

        // If there is a transfer specific `updateIdentifer`, then invoke it.  Think 'HBAR'
        if (NULL != transfer->handlers->updateIdentifier)
            transfer->handlers->updateIdentifier (transfer);

        // Otherwise, base the identifier on the string representation of the hash; which will
        // exist once the transfer is signed, generally.  In certain cases, think 'HBAR', the
        // hash won't necessarily exist.
        //
        // Note that BTC segwit transaction have a `txHash` and a `wtxHash`; BTC nodes
        // call the `txHash` the `hash` and the `wtxHash` the `identifier`.  WE DO NOT
        // adopt that definition of `identifer`.  In WalletKit a BTC Transfer's `hash`
        // and `identifer` are both the string representation of the `txHash`.  See
        // BRTransaction.{hc}
        //
        else {
            WKHash hash = wkTransferGetHash (transfer);

            if (NULL != hash) {
                pthread_mutex_lock (&transfer->lock);
                transfer->identifier = wkHashEncodeString(hash);
                pthread_mutex_unlock (&transfer->lock);
            }

            wkHashGive(hash);
        }
    }

    return transfer->identifier;
}

extern WKHash
wkTransferGetHash (WKTransfer transfer) {
    pthread_mutex_lock (&transfer->lock);
    WKHash hash = transfer->handlers->getHash (transfer);
    pthread_mutex_unlock (&transfer->lock);

    return hash;
}

extern WKBoolean
wkTransferSetHash (WKTransfer transfer,
                       OwnershipKept WKHash hash) {
    pthread_mutex_lock (&transfer->lock);
    bool changed = (NULL != transfer->handlers->setHash && transfer->handlers->setHash (transfer, hash));
    pthread_mutex_unlock (&transfer->lock);

    return AS_WK_BOOLEAN(changed);
}

extern WKFeeBasis
wkTransferGetEstimatedFeeBasis (WKTransfer transfer) {
    return wkFeeBasisTake (transfer->feeBasisEstimated);
}

private_extern WKAmount
wkTransferGetEstimatedFee (WKTransfer transfer) {
   return (NULL != transfer->feeBasisEstimated
            ? wkFeeBasisGetFee (transfer->feeBasisEstimated)
            : NULL);
}

extern WKFeeBasis
wkTransferGetConfirmedFeeBasis (WKTransfer transfer) {
    pthread_mutex_lock (&transfer->lock);
    WKFeeBasis feeBasisConfirmed = (WK_TRANSFER_STATE_INCLUDED == transfer->state->type
                                          ? wkFeeBasisTake (transfer->state->u.included.feeBasis)
                                          : NULL);
    pthread_mutex_unlock (&transfer->lock);

    return feeBasisConfirmed;
}

private_extern WKAmount
wkTransferGetConfirmedFee (WKTransfer transfer) {
    pthread_mutex_lock (&transfer->lock);
    WKAmount amount = ((WK_TRANSFER_STATE_INCLUDED == transfer->state->type &&
                              NULL != transfer->state->u.included.feeBasis)
                             ? wkFeeBasisGetFee (transfer->state->u.included.feeBasis)
                             : NULL);
    pthread_mutex_unlock (&transfer->lock);

    return amount;
}

private_extern WKFeeBasis
wkTransferGetFeeBasis (WKTransfer transfer) {
    pthread_mutex_lock (&transfer->lock);
    WKFeeBasis feeBasis = wkFeeBasisTake (WK_TRANSFER_STATE_INCLUDED == transfer->state->type
                                                    ? transfer->state->u.included.feeBasis
                                                    : transfer->feeBasisEstimated);
    pthread_mutex_unlock (&transfer->lock);

    return feeBasis;
}

extern WKAmount
wkTransferGetFee (WKTransfer transfer) {
    pthread_mutex_lock (&transfer->lock);
    WKFeeBasis feeBasis = (WK_TRANSFER_STATE_INCLUDED == transfer->state->type
                                 ? transfer->state->u.included.feeBasis
                                 : transfer->feeBasisEstimated);

    WKAmount amount = (NULL == feeBasis ? NULL : wkFeeBasisGetFee (feeBasis));
    pthread_mutex_unlock (&transfer->lock);

    return amount;
}

extern uint8_t *
wkTransferSerializeForSubmission (WKTransfer transfer,
                                      WKNetwork  network,
                                      size_t *serializationCount) {
    assert (NULL != serializationCount);
    return transfer->handlers->serialize (transfer, network, WK_TRUE, serializationCount);
}

extern uint8_t *
wkTransferSerializeForFeeEstimation (WKTransfer transfer,
                                         WKNetwork  network,
                                         size_t *bytesCount) {
    assert (NULL != bytesCount);
    return (NULL != transfer->handlers->getBytesForFeeEstimate
            ? transfer->handlers->getBytesForFeeEstimate (transfer, network, bytesCount)
            : transfer->handlers->serialize (transfer, network, WK_FALSE, bytesCount));
}

extern WKBoolean
wkTransferEqual (WKTransfer t1, WKTransfer t2) {
    if (t1 == t2) return WK_TRUE;
    if (t1->type != t2->type) return WK_FALSE;

    pthread_mutex_lock (&t1->lock);
    const char *t1ID = t1->identifier;
    pthread_mutex_unlock (&t1->lock);

    pthread_mutex_lock (&t2->lock);
    const char *t2ID = t2->identifier;
    pthread_mutex_unlock (&t2->lock);

    if (NULL != t1ID && NULL != t2ID && 0 == strcmp (t1ID, t2ID)) return WK_TRUE;

    return AS_WK_BOOLEAN (t1->handlers->isEqual (t1, t2));
}

extern WKComparison
wkTransferCompare (WKTransfer transfer1, WKTransfer transfer2) {
    // early bail when comparing the same transfer
    if (WK_TRUE == wkTransferEqual (transfer1, transfer2)) {
        return WK_COMPARE_EQ;
    }

    // The algorithm below is captured in the wkTransferCompare declaration
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

    WKComparison compareValue;
    WKTransferState state1 = wkTransferGetState (transfer1);
    WKTransferState state2 = wkTransferGetState (transfer2);

    // neither transfer is included
    if (state1->type != WK_TRANSFER_STATE_INCLUDED &&
        state2->type != WK_TRANSFER_STATE_INCLUDED) {
        // we don't have anything to sort on other than identity
        compareValue = (uintptr_t) transfer1 > (uintptr_t) transfer2 ?
            WK_COMPARE_GT : WK_COMPARE_LT;

    // transfer1 is NOT included (and transfer2 is)
    } else if (state1->type != WK_TRANSFER_STATE_INCLUDED) {
        // return "greater than" for transfer1
        compareValue = WK_COMPARE_GT;

    // transfer2 is NOT included (and transfer1 is)
    } else if (state2->type != WK_TRANSFER_STATE_INCLUDED) {
        // return "lesser than" for transfer1
        compareValue = WK_COMPARE_LT;

    // both are included, check if the timestamp differs
    } else if (state1->u.included.timestamp != state2->u.included.timestamp) {
        // return based on the greater timestamp
        compareValue = state1->u.included.timestamp > state2->u.included.timestamp ?
            WK_COMPARE_GT : WK_COMPARE_LT;

    // both are included and have the same timestamp, check if the block differs
    } else if (state1->u.included.blockNumber != state2->u.included.blockNumber) {
        // return based on the greater block number
        compareValue = state1->u.included.blockNumber > state2->u.included.blockNumber ?
            WK_COMPARE_GT : WK_COMPARE_LT;

    // both are included and have the same timestamp and block, check if the index differs
    } else if (state1->u.included.transactionIndex != state2->u.included.transactionIndex) {
        // return based on the greater index
        compareValue = state1->u.included.transactionIndex > state2->u.included.transactionIndex ?
            WK_COMPARE_GT : WK_COMPARE_LT;

    // both are included and have the same timestamp, block and index
    } else {
        // we are out of differentiators, return "equal"
        compareValue = WK_COMPARE_EQ;
    }

    // clean up on the way out
    wkTransferStateGive (state1);
    wkTransferStateGive (state2);

    return compareValue;
}

extern void
wkTransferExtractBlobAsBTC (WKTransfer transfer,
                                uint8_t **bytes,
                                size_t   *bytesCount,
                                uint32_t *blockHeight,
                                uint32_t *timestamp) {
    #ifdef REFACTOR
    assert (NULL != bytes && NULL != bytesCount);

    BRBitcoinTransaction *tx = wkTransferAsBTC (transfer);

    *bytesCount = btcTransactionSerialize (tx, NULL, 0);
    *bytes = malloc (*bytesCount);
    btcTransactionSerialize (tx, *bytes, *bytesCount);

    if (NULL != blockHeight) *blockHeight = tx->blockHeight;
    if (NULL != timestamp)   *timestamp   = tx->timestamp;
    #endif
}


extern const char *
wkTransferEventTypeString (WKTransferEventType t) {
    switch (t) {
        case WK_TRANSFER_EVENT_CREATED:
        return "WK_TRANSFER_EVENT_CREATED";

        case WK_TRANSFER_EVENT_CHANGED:
        return "WK_TRANSFER_EVENT_CHANGED";

        case WK_TRANSFER_EVENT_DELETED:
        return "WK_TRANSFER_EVENT_DELETED";
    }
    return "<WK_TRANSFER_EVENT_TYPE_UNKNOWN>";
}


/// MARK: Transaction Submission Error

// TODO(fix): This should be moved to a more appropriate file (BRTransfer.c/h?)

extern WKTransferSubmitError
wkTransferSubmitErrorUnknown(void) {
    return (WKTransferSubmitError) {
        WK_TRANSFER_SUBMIT_ERROR_UNKNOWN
    };
}

extern WKTransferSubmitError
wkTransferSubmitErrorPosix(int errnum) {
    return (WKTransferSubmitError) {
        WK_TRANSFER_SUBMIT_ERROR_POSIX,
        { .posix = { errnum } }
    };
}

extern bool
wkTransferSubmitErrorIsEqual (const WKTransferSubmitError *e1,
                                  const WKTransferSubmitError *e2) {
    return (e1->type == e2->type &&
            (e1->type != WK_TRANSFER_SUBMIT_ERROR_POSIX ||
             e1->u.posix.errnum == e2->u.posix.errnum));
}

extern char *
wkTransferSubmitErrorGetMessage (WKTransferSubmitError *e) {
    char *message = NULL;

    switch (e->type) {
        case WK_TRANSFER_SUBMIT_ERROR_POSIX: {
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

struct WKTransferAttributeRecord {
    char *key;
    char *value;
    WKBoolean isRequired;
    WKRef ref;
};

IMPLEMENT_WK_GIVE_TAKE (WKTransferAttribute, wkTransferAttribute)

private_extern WKTransferAttribute
wkTransferAttributeCreate (const char *key,
                               const char *val,
                               WKBoolean isRequired) {
    WKTransferAttribute attribute = calloc (1, sizeof (struct WKTransferAttributeRecord));

    attribute->key   = strdup (key);
    attribute->value = (NULL == val ? NULL : strdup (val));
    attribute->isRequired = isRequired;

    attribute->ref = WK_REF_ASSIGN (wkTransferAttributeRelease);

    return attribute;
}

extern WKTransferAttribute
wkTransferAttributeCopy (WKTransferAttribute attribute) {
    return wkTransferAttributeCreate (attribute->key,
                                          attribute->value,
                                          attribute->isRequired);
}

static void
wkTransferAttributeRelease (WKTransferAttribute attribute) {
    free (attribute->key);
    if (NULL != attribute->value) free (attribute->value);
    memset (attribute, 0, sizeof (struct WKTransferAttributeRecord));
    free (attribute);
}

extern const char *
wkTransferAttributeGetKey (WKTransferAttribute attribute) {
    return attribute->key;
}

extern const char * // nullable
wkTransferAttributeGetValue (WKTransferAttribute attribute) {
    return attribute->value;
}
extern void
wkTransferAttributeSetValue (WKTransferAttribute attribute, const char *value) {
    if (NULL != attribute->value) free (attribute->value);
    attribute->value = (NULL == value ? NULL : strdup (value));
}

extern WKBoolean
wkTransferAttributeIsRequired (WKTransferAttribute attribute) {
    return attribute->isRequired;
}

private_extern void
wkTransferAttributeArrayRelease (BRArrayOf(WKTransferAttribute) attributes) {
    if (NULL == attributes) return;
    array_free_all (attributes, wkTransferAttributeGive);
}

DECLARE_WK_GIVE_TAKE (WKTransferAttribute, wkTransferAttribute);
