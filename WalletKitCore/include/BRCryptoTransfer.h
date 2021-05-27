//
//  BRCryptoTransfer.h
//  BRCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoTransfer_h
#define BRCryptoTransfer_h

#include "BRCryptoHash.h"
#include "BRCryptoAddress.h"
#include "BRCryptoAmount.h"
#include "BRCryptoFeeBasis.h"
#include "BRCryptoNetwork.h"
#include "BRCryptoListener.h"

#ifdef __cplusplus
extern "C" {
#endif


    /// MARK: - Transfer Direction

    typedef enum {
        CRYPTO_TRANSFER_SENT,
        CRYPTO_TRANSFER_RECEIVED,
        CRYPTO_TRANSFER_RECOVERED
    } BRCryptoTransferDirection;

    /// MARK: - Transfer Attribute

    typedef struct BRCryptoTransferAttributeRecord *BRCryptoTransferAttribute;

    extern const char *
    cryptoTransferAttributeGetKey (BRCryptoTransferAttribute attribute);

    extern const char * // nullable
    cryptoTransferAttributeGetValue (BRCryptoTransferAttribute attribute);

    extern void
    cryptoTransferAttributeSetValue (BRCryptoTransferAttribute attribute, const char *value);

    extern BRCryptoBoolean
    cryptoTransferAttributeIsRequired (BRCryptoTransferAttribute attribute);

    extern BRCryptoTransferAttribute
    cryptoTransferAttributeCopy (BRCryptoTransferAttribute attribute);

    private_extern BRCryptoTransferAttribute
    cryptoTransferAttributeCreate (const char *key,
                                   const char *val, // nullable
                                   BRCryptoBoolean isRequired);

    DECLARE_CRYPTO_GIVE_TAKE (BRCryptoTransferAttribute, cryptoTransferAttribute);

    typedef enum {
        CRYPTO_TRANSFER_ATTRIBUTE_VALIDATION_ERROR_REQUIRED_BUT_NOT_PROVIDED,
        CRYPTO_TRANSFER_ATTRIBUTE_VALIDATION_ERROR_MISMATCHED_TYPE,
        CRYPTO_TRANSFER_ATTRIBUTE_VALIDATION_ERROR_RELATIONSHIP_INCONSISTENCY
    } BRCryptoTransferAttributeValidationError;

    /// MARK: - Transfer

    /**
     * Returns the transfer's source address
     *
     * @param transfer the transfer
     *
     * @return the source address or NULL
     */
    extern BRCryptoAddress
    cryptoTransferGetSourceAddress (BRCryptoTransfer transfer);

    /**
     * Returns the transfer's target address
     *
     * @param transfer the transfer
     *
     * @return the source address or NULL
     */
   extern BRCryptoAddress
    cryptoTransferGetTargetAddress (BRCryptoTransfer transfer);

    /**
     * Returns the transfer's amount
     *
     * @param transfer the transfer
     *
     * @return the amount
     */
    extern BRCryptoAmount
    cryptoTransferGetAmount (BRCryptoTransfer transfer);

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
    extern BRCryptoAmount
    cryptoTransferGetAmountDirected (BRCryptoTransfer transfer);

    /**
     * Returns the transfer's fee.  Note that the `fee` and the `amount` may be in different
     * currencies.
     *
     * @param transfer the transfer
     *
     * @return the fee
     */
//    extern BRCryptoAmount
//    cryptoTransferGetFee (BRCryptoTransfer transfer);

//    extern BRCryptoBoolean
//    cryptoTransferExtractConfirmation (BRCryptoTransfer transfer,
//                                       uint64_t *blockNumber,
//                                       uint64_t *transactionIndex,
//                                       uint64_t *timestamp,
//                                       BRCryptoAmount *fee);

    extern BRCryptoTransferStateType
    cryptoTransferGetStateType (BRCryptoTransfer transfer);

    extern BRCryptoTransferState
    cryptoTransferGetState (BRCryptoTransfer transfer);

    extern BRCryptoBoolean
    cryptoTransferIsSent (BRCryptoTransfer transfer);

    extern BRCryptoTransferDirection
    cryptoTransferGetDirection (BRCryptoTransfer transfer);

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
    cryptoTransferGetIdentifier (BRCryptoTransfer transfer);

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
    extern BRCryptoHash
    cryptoTransferGetHash (BRCryptoTransfer transfer);

    /**
     * Generally a Transfer's hash is derived by applying a Blockchain-specific hash function to a
     * Blockchain-specific byte representation of a Transfer.
     *
     * @Note: Some Blockchains, notablely Hedera, produce a hash the depends on the Hedera Node the
     * processes the Transfer.  Hence, we don't
     * know the hash until after
     */
    extern BRCryptoBoolean
    cryptoTransferSetHash (BRCryptoTransfer transfer,
                           OwnershipKept BRCryptoHash hash);

    extern BRCryptoUnit
    cryptoTransferGetUnitForAmount (BRCryptoTransfer transfer);

    extern BRCryptoUnit
    cryptoTransferGetUnitForFee (BRCryptoTransfer transfer);

    /**
     * Returns the transfer's feeBasis.
     *
     * @param transfer the transfer
     *
     * @return the transfer's feeBasis
     */
    extern BRCryptoFeeBasis
    cryptoTransferGetEstimatedFeeBasis (BRCryptoTransfer transfer);

    extern BRCryptoFeeBasis
    cryptoTransferGetConfirmedFeeBasis (BRCryptoTransfer transfer);

    ///
    /// Return the transfer's fee.  If the transfer's fee is paid in a different currency from the
    /// transfer's amount, such as an ERC20 transfer being paid in ETHER, then NULL is returned.  If
    /// the transfers is not SEND by our User, then NULL is returned.
    ///
    /// TODO: The Transfer's Fee should be independent of the direction
    ///
    /// @param transfer the transfer
    ///
    extern BRCryptoAmount
    cryptoTransferGetFee (BRCryptoTransfer transfer);

    extern size_t
    cryptoTransferGetAttributeCount (BRCryptoTransfer transfer);

    extern BRCryptoTransferAttribute
    cryptoTransferGetAttributeAt (BRCryptoTransfer transfer,
                                  size_t index);

    extern uint8_t *
    cryptoTransferSerializeForSubmission (BRCryptoTransfer transfer,
                                          BRCryptoNetwork  network,
                                          size_t *serializationCount);

    extern uint8_t *
    cryptoTransferSerializeForFeeEstimation (BRCryptoTransfer transfer,
                                             BRCryptoNetwork  network,
                                             size_t *serializationCount);

    extern BRCryptoBoolean
    cryptoTransferEqual (BRCryptoTransfer transfer1, BRCryptoTransfer transfer2);

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
    extern BRCryptoComparison
    cryptoTransferCompare (BRCryptoTransfer transfer1, BRCryptoTransfer transfer2);

    DECLARE_CRYPTO_GIVE_TAKE (BRCryptoTransfer, cryptoTransfer);

    extern void
    cryptoTransferExtractBlobAsBTC (BRCryptoTransfer transfer,
                                    uint8_t **bytes,
                                    size_t   *bytesCount,
                                    uint32_t *blockHeight,
                                    uint32_t *timestamp);

    // MARK: - Transfer Output

    /**
     * A TransferOutput is a pair of {target, amount} that is used to create a transfer w/
     * multiple outputs.  This is *only* used in the Crypto C interface and is never stored in
     * the C code; hence we get away w/o making `BRCryptoTransferOutput` a 'first-class object'
     * (that is, with a reference count).  The User is expected to maintain references to
     * `target` and `amount` during the scope of use.
     *
     * Preliminary interface.  Caution warranted.
     */
    typedef struct {
        BRCryptoAddress target;
        BRCryptoAmount  amount;
    // TODO: This does not handle BRCryptoTransferAttribute; only BTC, BCH, BSV supported
    } BRCryptoTransferOutput;

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoTransfer_h */
