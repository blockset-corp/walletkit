//
//  BBREthereumTransaction.h
//  WalletKitCore Ethereum
//
//  Created by Ed Gamble on 2/21/2018.
//  Copyright © 2018-2019 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BR_Ethereum_Transaction_H
#define BR_Ethereum_Transaction_H

#include "ethereum/base/BREthereumBase.h"
#include "ethereum/contract/BREthereumToken.h"
#include "BREthereumNetwork.h"
#include "BREthereumTransactionStatus.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ETHEREUM_TRANSACTION_NONCE_IS_NOT_ASSIGNED   UINT64_MAX

/// If we get a gasEstimate we'll want the gasLimit to have a margin over the estimate
#define ETHEREUM_GAS_LIMIT_MARGIN_PERCENT        (20)

static inline BREthereumGas
gasApplyLimitMargin (BREthereumGas gas) {
    return ethGasCreate(((100 + ETHEREUM_GAS_LIMIT_MARGIN_PERCENT) * gas.amountOfGas) / 100);
}

/**
 * An Ethereum Transaction is a transaction on the Ethereum P2P network
 *
 * Per the Ethereum Specification: A transaction (formally, T) is a single cryptographically-signed
 * instruction constructed by an actor externally to the scope of Ethereum. While it is assumed
 * that the ultimate external actor will be human in nature, software tools will be used in its
 * construction and dissemination1. There are two types of transactions: those which result in
 * message calls and those which result in the creation of new accounts with associated code
 * (known informally as ‘contract creation’). Both types specify a number of common fields:
 * { nonce, gasPrice, gasLimit, to, value, {v,r,s}}.
 *
 * Additional filds are: {hash, chainId, data and status}.
 */
typedef struct BREthereumTransactionRecord *BREthereumTransaction;

extern BREthereumTransaction
ethTransactionCreate(BREthereumAddress sourceAddress,
                  BREthereumAddress targetAddress,
                  BREthereumEther amount,
                  BREthereumGasPrice gasPrice,
                  BREthereumGas gasLimit,
                  const char *data,
                  uint64_t nonce);

extern BREthereumTransaction
ethTransactionCopy (BREthereumTransaction transaction);

extern void
ethTransactionRelease (BREthereumTransaction transaction);

extern void
ethTransactionReleaseForSet (void *ignore, void *item);

extern BREthereumAddress
ethTransactionGetSourceAddress(BREthereumTransaction transaction);

extern BREthereumAddress
ethTransactionGetTargetAddress(BREthereumTransaction transaction);

extern BREthereumBoolean
ethTransactionHasAddress (BREthereumTransaction transaction,
                       BREthereumAddress address);
    
extern BREthereumEther
ethTransactionGetAmount(BREthereumTransaction transaction);

/**
 * Return the gasPrice
 */
extern BREthereumGasPrice
ethTransactionGetGasPrice (BREthereumTransaction transaction);

/**
 * Return the gasLimit
 */
extern BREthereumGas
ethTransactionGetGasLimit (BREthereumTransaction transaction);

/**
 * Return the gasUsed if the transaction is included.  In that case *isValid will be
 * ETHEREUM_BOOLEAN_TRUE.  If not included, then *isValid will be ETHEREUM_BOOLEAN_FALSE and the
 * gasUsed will be zero.
 */
extern BREthereumGas
ethTransactionGetGasUsed (BREthereumTransaction transaction,
                       BREthereumBoolean *isValid);

/**
 * Return the feeBasis for transaction based on the gasLimit.  The gasLimit is an upper bound
 * on the gasUsed; thus the returned feeBasis is but an estimate.
 */
extern BREthereumFeeBasis
ethTransactionGetFeeBasisEstimated (BREthereumTransaction transaction);

/**
 * Return the feeBasis for transaction if the transaction is included.  When included *isValid
 * will be ETHEREUM_BOOLEAN_TRUE.  If not included, then *isValid will be ETHEREUM_BOOLEAN_FALSE
 * and the returned feeBasis should not be referenced (it will be filled with zeros).
 */
extern BREthereumFeeBasis
ethTransactionGetFeeBasisConfirmed (BREthereumTransaction transaction,
                                 BREthereumBoolean *isValid);

/**
 * Return the feeBasis for transaction.  If the transaction is confirmed (aka blocked) then
 * the value returned is the actual feeBasis paid {gasUsed, gasPrice}; if the transaction is not
 * confirmed then an estimated feeBasis is returned {gasEstimate, gasPrice}.
 */
extern BREthereumFeeBasis
ethTransactionGetFeeBasis (BREthereumTransaction transaction);

/**
 * Return the fee (in Ether) for transaction based on `ethTransactionGetFeeBasis()`.  If the fee
 * computation overflows, then *overflow is set to ETHEREUM_BOOLEAN_TRUE.
 */
extern BREthereumEther
ethTransactionGetFee (BREthereumTransaction transaction,
                   BREthereumBoolean *overflow);

extern uint64_t
ethTransactionGetNonce (BREthereumTransaction transaction);

private_extern void
ethTransactionSetNonce (BREthereumTransaction transaction,
                     uint64_t nonce);

extern const BREthereumHash
ethTransactionGetHash (BREthereumTransaction transaction);

// Caution
extern void
ethTransactionSetHash (BREthereumTransaction transaction,
                    BREthereumHash hash);

extern const char * // no not modify the return value
ethTransactionGetData (BREthereumTransaction transaction);

// Support BRSet
extern size_t
ethTransactionHashValue (const void *h);

// Support BRSet
extern int
ethTransactionHashEqual (const void *h1, const void *h2);

//
// Transaction Signing
//
extern void
ethTransactionSign(BREthereumTransaction transaction,
                BREthereumSignature signature);

extern BREthereumBoolean
ethTransactionIsSigned (BREthereumTransaction transaction);

extern BREthereumSignature
ethTransactionGetSignature (BREthereumTransaction transaction);

/**
 * Extract the signer's address.  If not signed, an empty address is returned.
 */
extern BREthereumAddress
ethTransactionExtractAddress(BREthereumTransaction transaction,
                          BREthereumNetwork network,
                          BRRlpCoder coder);
//
// Transaction RLP Encoding
//
extern BREthereumTransaction
ethTransactionRlpDecode (BRRlpItem item,
                      BREthereumNetwork network,
                      BREthereumRlpType type,
                      BRRlpCoder coder);

/**
 * RLP encode transaction for the provided network with the specified type.  Different networks
 * have different RLP encodings - notably the network's chainId is part of the encoding.
 */

extern BRRlpItem
ethTransactionRlpEncode(BREthereumTransaction transaction,
                     BREthereumNetwork network,
                     BREthereumRlpType type,
                     BRRlpCoder coder);

extern BRRlpData
ethTransactionGetRlpData (BREthereumTransaction transaction,
                       BREthereumNetwork network,
                       BREthereumRlpType type);

extern char *
ethTransactionGetRlpHexEncoded (BREthereumTransaction transaction,
                             BREthereumNetwork network,
                             BREthereumRlpType type,
                             const char *prefix);
//
// Transaction Comparison
//
extern BREthereumComparison
ethTransactionCompare (BREthereumTransaction t1,
                    BREthereumTransaction t2);

extern BREthereumTransactionStatus
ethTransactionGetStatus (BREthereumTransaction transaction);

extern void
ethTransactionSetStatus (BREthereumTransaction transaction,
                      BREthereumTransactionStatus status);
    
extern BREthereumBoolean
ethTransactionIsConfirmed (BREthereumTransaction transaction);

// TODO: Rethink
extern BREthereumBoolean
ethTransactionIsSubmitted (BREthereumTransaction transaction);

extern BREthereumBoolean
ethTransactionIsErrored (BREthereumTransaction transaction);

extern void
ethTransactionShow (BREthereumTransaction transaction, const char *topic);

extern void
transactionsRelease (BRArrayOf(BREthereumTransaction) transactions);

//
// Transaction Result
//
typedef struct BREthereumTransactionResultRecord *BREthereumTransactionResult;

#ifdef __cplusplus
}
#endif

#endif /* BR_Ethereum_Transaction_H */
