//
//  WKTransfer.h
//  WalletKitCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKTransfer_h
#define WKTransfer_h

#include "WKHash.h"
#include "WKAddress.h"
#include "WKAmount.h"
#include "WKFeeBasis.h"
#include "WKNetwork.h"
#include "WKListener.h"

#ifdef __cplusplus
extern "C" {
#endif

/// MARK: - Transfer Direction

/**
 * An enumeration of transfer directions.
 */
typedef enum {
    WK_TRANSFER_SENT,
    WK_TRANSFER_RECEIVED,
    WK_TRANSFER_RECOVERED
} WKTransferDirection;

/// MARK: - Transfer Attribute

/**
 * @brief A Transfer Attribute is an arbitrary {key,value,isRequired} triple that extends a
 * transfer with, typically, network specific data.
 */
typedef struct WKTransferAttributeRecord *WKTransferAttribute;

/**
 * Get the transfer attribute's key
 */
extern const char *
wkTransferAttributeGetKey (WKTransferAttribute attribute);

/**
 * Get the transfer attribute's value
 */
extern const char * // nullable
wkTransferAttributeGetValue (WKTransferAttribute attribute);

/**
 * Set the transfer attribute's value.
 */
extern void
wkTransferAttributeSetValue (WKTransferAttribute attribute, const char *value);

/**
 * Check if the transfer attribute is required.
 */
extern WKBoolean
wkTransferAttributeIsRequired (WKTransferAttribute attribute);

/**
 * Copy the transfer attribute.
 */
extern WKTransferAttribute
wkTransferAttributeCopy (WKTransferAttribute attribute);

DECLARE_WK_GIVE_TAKE (WKTransferAttribute, wkTransferAttribute);

/**
 * An enumeration of transfer attribute validation errors.
 */
typedef enum {
    WK_TRANSFER_ATTRIBUTE_VALIDATION_ERROR_REQUIRED_BUT_NOT_PROVIDED,
    WK_TRANSFER_ATTRIBUTE_VALIDATION_ERROR_MISMATCHED_TYPE,
    WK_TRANSFER_ATTRIBUTE_VALIDATION_ERROR_RELATIONSHIP_INCONSISTENCY
} WKTransferAttributeValidationError;

/// MARK: - Transfer

/**
 * Returns the transfer's source address
 *
 * @param transfer the transfer
 *
 * @return the source address or NULL
 */
extern WKAddress
wkTransferGetSourceAddress (WKTransfer transfer);

/**
 * Returns the transfer's target address
 *
 * @param transfer the transfer
 *
 * @return the source address or NULL
 */
extern WKAddress
wkTransferGetTargetAddress (WKTransfer transfer);

/**
 * Returns the transfer's amount
 *
 * @param transfer the transfer
 *
 * @return the amount
 */
extern WKAmount
wkTransferGetAmount (WKTransfer transfer);

/**
 * Returns the transfer's amount after considering the direction
 *
 * If we received the transfer, the amount will be positive; if we sent the transfer, the
 * amount will be negative; if the transfer is 'self directed', the amount will be zero.
 *
 * @param transfer the transfer
 *
 * @return the amount
 */
extern WKAmount
wkTransferGetAmountDirected (WKTransfer transfer);

/**
 * Get the transfer's state type.
 */
extern WKTransferStateType
wkTransferGetStateType (WKTransfer transfer);

/**
 * Get the transfer's state.
 */
extern WKTransferState
wkTransferGetState (WKTransfer transfer);

/**
 * Check if transfer was sent.
 */
extern WKBoolean
wkTransferIsSent (WKTransfer transfer);

/**
 * Get the transfer's direction.
 */
extern WKTransferDirection
wkTransferGetDirection (WKTransfer transfer);

/**
 * Returns the unique identifier for a transaction.  The identifer is unique within the
 * scope of the Transfer's wallet.  In WalletKit it is possible for two Transfers to have
 * the same identifier if and only if the Transfers are in a different wallet.  This will
 * happen, for example, with send ERC-20 wallets whereby both the asset transfer and the fee
 * will have the same identifer.
 *
 * @Note In general, the identifier will not exist until the Transfer has been successfully
 * submitted to its Blockchain; before then the return value is `NULL`.  However, the actual
 * time of existence is blockchain specific.
 *
 * @param transfer the transfer
 * @return the identifier as a string.
 */
extern const char *
wkTransferGetIdentifier (WKTransfer transfer);

/**
 * Returns the transfer's hash.  The hash is determined by applying a blockchain-specific hash
 * function to a blockchain-specific byte-serialized representation of a Transfer.  A hash is
 * generally a unique identifier of Transfers in a wallet and also, but not always, on a
 * blockchain.
 *
 * This value may be NULL; notably before a Transfer is signed; and, in the case of HBAR,
 * before it is successfully submitted (the HBAR hash depends on the HBAR node handling the
 * Transfer).
 *
 * @note: One should expect all Transfers in a Wallet to have a unique hash.  However, in
 * WalletKit, a hash might be shared by two Transfers if and only if in different wallets.
 *
 * @param transfer the transfer
 *
 * @return the transfer's hash
 */
extern WKHash
wkTransferGetHash (WKTransfer transfer);

/**
 * Generally a Transfer's hash is derived by applying a Blockchain-specific hash function to a
 * Blockchain-specific byte representation of a Transfer.
 *
 * @Note: Some Blockchains, notablely Hedera, produce a hash the depends on the Hedera Node the
 * processes the Transfer.  Hence, we don't
 * know the hash until after
 */
extern WKBoolean
wkTransferSetHash (WKTransfer transfer,
                   OwnershipKept WKHash hash);

/**
 * Get the unit used for the transfer's amount.
 */
extern WKUnit
wkTransferGetUnitForAmount (WKTransfer transfer);

/**
 * Get the unit used for the transfer's fee.  This unit might be imcompatible (have a different
 * currency) from the unit for the transfer's amount.  For example, a transfer for an ERC20
 * token will have a fee in Ether but the amount will be the currency for the ERC20 token
 */
extern WKUnit
wkTransferGetUnitForFee (WKTransfer transfer);

/**
 * Returns the transfer's feeBasis.
 *
 * @param transfer the transfer
 *
 * @return the transfer's feeBasis
 */
extern WKFeeBasis
wkTransferGetEstimatedFeeBasis (WKTransfer transfer);

/**
 * Get the transfer's confirmed fee basis.  This can be NULL if the transfer has not been
 * confirmed.  The confirmed fee basis may differ from the estimated fee basis.  For example,
 * an Etheruem ERC20 transfer will have an estimated fee basis using the `gasLimit` whereas
 * the confirmed fee basis has `gasUsed`.  In general `gasUsed <= gasLimit`.
 */
extern WKFeeBasis
wkTransferGetConfirmedFeeBasis (WKTransfer transfer);

/**
 * Return the transfer's fee.  If the transfer's fee is paid in a different currency from the
 * transfer's amount, such as an ERC20 transfer being paid in ETHER, then NULL is returned.  If
 * the transfers is not SEND by our User, then NULL is returned.
 *
 * TODO: The Transfer's Fee should be independent of the direction
 *
 * @param transfer the transfer
 */
extern WKAmount
wkTransferGetFee (WKTransfer transfer);

/**
 * Get the count of the transfer's attributes
 */
extern size_t
wkTransferGetAttributeCount (WKTransfer transfer);

/**
 * Get the transfer's attribute at `index`.  The index must be `[0,count)`.
 */
extern WKTransferAttribute
wkTransferGetAttributeAt (WKTransfer transfer,
                          size_t index);

/**
 * Get a serialization of `transfer` suitable for submission to `network`.  The transfer must
 * apply to `network` and the transfer must be signed.  Fills `serializationCount` with the
 * number of bytes in the serialization.  The returned value is owned by the caller and must
 * be freed.
 */
extern OwnershipGiven uint8_t *
wkTransferSerializeForSubmission (WKTransfer transfer,
                                  WKNetwork  network,
                                  size_t *serializationCount);

/**
 * Get a serialization of `transfer` suitable for fee estimation on `network`.  The transfer
 * must apply to `network` and the transfer should not be signed.  Fills `serializationCount`
 * with the number of bytes in the serialization.  The returned value is owned by the caller and
 * must be freed.
 */
extern OwnershipGiven uint8_t *
wkTransferSerializeForFeeEstimation (WKTransfer transfer,
                                     WKNetwork  network,
                                     size_t *serializationCount);

/**
 * Check if two transfers are equal.
 */
extern WKBoolean
wkTransferEqual (WKTransfer transfer1, WKTransfer transfer2);

/**
 * Compares two transfers.
 *
 * The transfers are ordered according to the following algorithm:
 *   - IF neither transfer is in the INCLUDED state, they are ordered by pointer identity
 *   - ELSE IF one transfer is in the INCLUDED state, it is "lesser than" one that is NOT
 *   - ELSE both are in the INCLUDED state, order by timestamp, block number and transaction
 *     index (in that order), with those values being compared by magnitude
 *
 * In practice, this means that:
 *   - Transfer A (INCLUDED at time 0, block 0, index 0) is lesser than
 *   - Transfer B (INCLUDED at time 0, block 0, index 1) is lesser than
 *   - Transfer C (INCLUDED at time 0, block 1, index 0) is lesser than
 *   - Transfer D (INCLUDED at time 1, block 0, index 0) is lesser than
 *   - Transfer E (CREATED with pointer 0x10000000) is lesser than
 *   - Transfer F (SIGNED  with pointer 0x20000000) is lesser than
 *   - Transfer G (CREATED with pointer 0x30000000) is lesser than
 *   - Transfer H (DELETED with pointer 0x40000000)
 */
extern WKComparison
wkTransferCompare (WKTransfer transfer1, WKTransfer transfer2);

DECLARE_WK_GIVE_TAKE (WKTransfer, wkTransfer);

extern void
wkTransferExtractBlobAsBTC (WKTransfer transfer,
                            uint8_t **bytes,
                            size_t   *bytesCount,
                            uint32_t *blockHeight,
                            uint32_t *timestamp);

// MARK: - Transfer Output

/**
 * A TransferOutput is a pair of {target, amount} that is used to create a transfer w/
 * multiple outputs.  This is *only* used in the Crypto C interface and is never stored in
 * the C code; hence we get away w/o making `WKTransferOutput` a 'first-class object'
 * (that is, with a reference count).  The User is expected to maintain references to
 * `target` and `amount` during the scope of use.
 *
 * Preliminary interface.  Caution warranted.
 */
typedef struct {
    WKAddress target;
    WKAmount  amount;
    // TODO: This does not handle WKTransferAttribute; only BTC, BCH, BSV supported
} WKTransferOutput;

#ifdef __cplusplus
}
#endif

#endif /* WKTransfer_h */
