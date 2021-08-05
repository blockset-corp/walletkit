//
//  BRBitcoinWallet.h
//
//  Created by Aaron Voisine on 9/1/15.
//  Copyright (c) 2015 breadwallet LLC
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

#ifndef BRWallet_h
#define BRWallet_h

#include "BRBitcoinTransaction.h"
#include "support/BRAddress.h"
#include "support/BRBIP32Sequence.h"
#include "support/BRInt.h"
#include <string.h>
#include <stdbool.h>  

#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_FEE_PER_KB (TX_FEE_PER_KB*10)                  // 10 satoshis-per-byte
#define MIN_FEE_PER_KB     TX_FEE_PER_KB                       // bitcoind 0.12 default min-relay fee
#define MAX_FEE_PER_KB     ((TX_FEE_PER_KB*1000100 + 190)/191) // slightly higher than a 10,000bit fee on a 191byte tx

typedef struct {
    UInt256 hash;
    uint32_t n;
} BRBitcoinUTXO;

inline static size_t btcUTXOHash(const void *utxo)
{
    // (hash xor n)*FNV_PRIME
    return (size_t)((((const BRBitcoinUTXO *)utxo)->hash.u32[0] ^ ((const BRBitcoinUTXO *)utxo)->n)*0x01000193);
}

inline static int btcUTXOEq(const void *utxo, const void *otherUtxo)
{
    return (utxo == otherUtxo ||
            (UInt256Eq(((const BRBitcoinUTXO *)utxo)->hash, ((const BRBitcoinUTXO *)otherUtxo)->hash) &&
             ((const BRBitcoinUTXO *)utxo)->n == ((const BRBitcoinUTXO *)otherUtxo)->n));
}

typedef struct BRBitcoinWalletStruct BRBitcoinWallet;

// allocates and populates a BRBitcoinWallet struct that must be freed by calling btcWalletFree()
BRBitcoinWallet *btcWalletNew(BRAddressParams addrParams, BRBitcoinTransaction *transactions[], size_t txCount,
                              BRMasterPubKey mpk);

// not thread-safe, set callbacks once after btcWalletNew(), before calling other BRBitcoinWallet functions
// info is a void pointer that will be passed along with each callback call
// void balanceChanged(void *, uint64_t) - called when the wallet balance changes
// void txAdded(void *, BRBitcoinTransaction *) - called when transaction is added to the wallet
// void txUpdated(void *, const UInt256[], size_t, uint32_t, uint32_t)
//   - called when the blockHeight or timestamp of previously added transactions are updated
// void txDeleted(void *, UInt256, int, int) - called when a previously added transaction is removed from the wallet
void btcWalletSetCallbacks(BRBitcoinWallet *wallet, void *info,
                          void (*balanceChanged)(void *info, uint64_t balance),
                          void (*txAdded)(void *info, BRBitcoinTransaction *tx),
                          void (*txUpdated)(void *info, const UInt256 txHashes[], size_t txCount, uint32_t blockHeight,
                                            uint32_t timestamp),
                          void (*txDeleted)(void *info, UInt256 txHash, int notifyUser, int recommendRescan));

// wallets are composed of chains of addresses
// each chain is traversed until a gap of a number of addresses is found that haven't been used in any transactions
// this function writes to addrs an array of <gapLimit> unused addresses following the last used address in the chain
// the internal chain is used for change addresses and the external chain for receive addresses
// addrs may be NULL to only generate addresses for btcWalletContainsAddress()
// returns the number addresses written to addrs
size_t btcWalletUnusedAddrs(BRBitcoinWallet *wallet, BRAddress addrs[], uint32_t gapLimit, uint32_t internal);

BRAddressParams btcWalletGetAddressParams (BRBitcoinWallet *wallet);

// returns the first unused external address (bech32 pay-to-witness-pubkey-hash)
BRAddress btcWalletReceiveAddress(BRBitcoinWallet *wallet);

// returns the first unused external address (legacy pay-to-pubkey-hash)
BRAddress btcWalletLegacyAddress(BRBitcoinWallet *wallet);

// return the legacy address for `addr`
BRAddress btcWalletAddressToLegacy (BRBitcoinWallet *wallet, BRAddress *addr);

// writes all addresses previously genereated with btcWalletUnusedAddrs() to addrs
// returns the number addresses written, or total number available if addrs is NULL
size_t btcWalletAllAddrs(BRBitcoinWallet *wallet, BRAddress addrs[], size_t addrsCount);

// true if the address was previously generated by btcWalletUnusedAddrs() (even if it's now used)
int btcWalletContainsAddress(BRBitcoinWallet *wallet, const char *addr);

// true if the address was previously used as an input or output in any wallet transaction
int btcWalletAddressIsUsed(BRBitcoinWallet *wallet, const char *addr);

// writes transactions registered in the wallet, sorted by date, oldest first, to the given transactions array
// returns the number of transactions written, or total number available if transactions is NULL
size_t btcWalletTransactions(BRBitcoinWallet *wallet, BRBitcoinTransaction *transactions[], size_t txCount);

// writes transactions registered in the wallet, and that were unconfirmed before blockHeight, to the transactions array
// returns the number of transactions written, or total number available if transactions is NULL
size_t btcWalletTxUnconfirmedBefore(BRBitcoinWallet *wallet, BRBitcoinTransaction *transactions[], size_t txCount,
                                    uint32_t blockHeight);

// current wallet balance, not including transactions known to be invalid
uint64_t btcWalletBalance(BRBitcoinWallet *wallet);

void btcWalletUpdateBalance(BRBitcoinWallet *wallet);

// total amount spent from the wallet (exluding change)
uint64_t btcWalletTotalSent(BRBitcoinWallet *wallet);

// total amount received by the wallet (exluding change)
uint64_t btcWalletTotalReceived(BRBitcoinWallet *wallet);

// writes unspent outputs to utxos and returns the number of outputs written, or number available if utxos is NULL
size_t btcWalletUTXOs(BRBitcoinWallet *wallet, BRBitcoinUTXO utxos[], size_t utxosCount);
    
// fee-per-kb of transaction size to use when creating a transaction
// the wallet maintains a fee per kb that is associated with it
// this value can be set by calls to `BRWalletSetFeePerKb` but is also set in response to `feefilter` messages
// transactions can be created using functions that use the wallet's fee per kb or a passed in value (see
// BRWalletCreateTransaction and BRWalletCreateTransactionWithFeePerKb for example); the latter is to support
// cases where a specific feePerKb value is desired, so as to avoid having to get the wallet's feePerKb, set a
// new value, create a transaction and then restore the wallet's feePerKb (which potentially races against P2P updates)
uint64_t btcWalletFeePerKb(BRBitcoinWallet *wallet);
void btcWalletSetFeePerKb(BRBitcoinWallet *wallet, uint64_t feePerKb);

// returns an unsigned transaction that sends the specified amount from the wallet to the given address
// result must be freed using btcTransactionFree()
BRBitcoinTransaction *btcWalletCreateTransaction(BRBitcoinWallet *wallet, uint64_t amount, const char *addr);

// returns an unsigned transaction that sends the specified amount from the wallet to the given address
// result must be freed using btcTransactionFree()
// use feePerKb UINT64_MAX to indicate that the wallet feePerKb should be used
BRBitcoinTransaction *btcWalletCreateTransactionWithFeePerKb(BRBitcoinWallet *wallet, uint64_t feePerKb,
                                                             uint64_t amount, const char *addr);

// returns an unsigned transaction that satisifes the given transaction outputs
// result must be freed using btcTransactionFree()
BRBitcoinTransaction *btcWalletCreateTxForOutputs(BRBitcoinWallet *wallet, const BRBitcoinTxOutput outputs[],
                                                  size_t outCount);

// returns an unsigned transaction that satisifes the given transaction outputs
// result must be freed using btcTransactionFree()
// use feePerKb UINT64_MAX to indicate that the wallet feePerKb should be used
BRBitcoinTransaction *btcWalletCreateTxForOutputsWithFeePerKb(BRBitcoinWallet *wallet, uint64_t feePerKb,
                                                              const BRBitcoinTxOutput outputs[], size_t outCount);

// signs any inputs in tx that can be signed using private keys from the wallet
// forkId is 0 for bitcoin, 0x40 for b-cash
// seed is the master private key (wallet seed) corresponding to the master public key given when the wallet was created
// returns true if all inputs were signed, or false if there was an error or not all inputs were able to be signed
int btcWalletSignTransaction(BRBitcoinWallet *wallet, BRBitcoinTransaction *tx, uint8_t forkId,
                             int depth, const uint32_t child[], const void *seed, size_t seedLen);

// true if the given transaction is associated with the wallet (even if it hasn't been registered)
int btcWalletContainsTransaction(BRBitcoinWallet *wallet, const BRBitcoinTransaction *tx);

// adds a transaction to the wallet, or returns false if it isn't associated with the wallet
int btcWalletRegisterTransaction(BRBitcoinWallet *wallet, BRBitcoinTransaction *tx, bool needUpdateBalance);

// removes a tx from the wallet, along with any tx that depend on its outputs
void btcWalletRemoveTransaction(BRBitcoinWallet *wallet, UInt256 txHash);

// returns the transaction with the given hash if it's been registered in the wallet
BRBitcoinTransaction *btcWalletTransactionForHash(BRBitcoinWallet *wallet, UInt256 txHash);

// returns a copy of the transaction with the given hash if it's been registered in the wallet
BRBitcoinTransaction *btcWalletTransactionCopyForHash(BRBitcoinWallet *wallet, UInt256 txHash);

// true if no previous wallet transaction spends any of the given transaction's inputs, and no inputs are invalid
int btcWalletTransactionIsValid(BRBitcoinWallet *wallet, const BRBitcoinTransaction *tx);

// true if transaction cannot be immediately spent (i.e. if it or an input tx can be replaced-by-fee)
int btcWalletTransactionIsPending(BRBitcoinWallet *wallet, const BRBitcoinTransaction *tx);

// true if tx is considered 0-conf safe (valid and not pending, timestamp is greater than 0, and no unverified inputs)
int btcWalletTransactionIsVerified(BRBitcoinWallet *wallet, const BRBitcoinTransaction *tx);

// true if all tx inputs that are contained in wallet have txs in wallet AND if tx itself is signed.
int btcWalletTransactionIsResolved (BRBitcoinWallet *wallet, const BRBitcoinTransaction *tx);

// set the block heights and timestamps for the given transactions
// use height TX_UNCONFIRMED and timestamp 0 to indicate a tx should remain marked as unverified (not 0-conf safe)
void btcWalletUpdateTransactions(BRBitcoinWallet *wallet, const UInt256 txHashes[], size_t txCount,
                                 uint32_t blockHeight, uint32_t timestamp);
    
// marks all transactions confirmed after blockHeight as unconfirmed (useful for chain re-orgs)
void btcWalletSetTxUnconfirmedAfter(BRBitcoinWallet *wallet, uint32_t blockHeight);

// returns the amount received by the wallet from the transaction (total outputs to change and/or receive addresses)
uint64_t btcWalletAmountReceivedFromTx(BRBitcoinWallet *wallet, const BRBitcoinTransaction *tx);

// returns the amount sent from the wallet by the trasaction (total wallet outputs consumed, change and fee included)
uint64_t btcWalletAmountSentByTx(BRBitcoinWallet *wallet, const BRBitcoinTransaction *tx);

// returns the fee for the given transaction if all its inputs are from wallet transactions, UINT64_MAX otherwise
uint64_t btcWalletFeeForTx(BRBitcoinWallet *wallet, const BRBitcoinTransaction *tx);

// historical wallet balance after the given transaction, or current balance if transaction is not registered in wallet
uint64_t btcWalletBalanceAfterTx(BRBitcoinWallet *wallet, const BRBitcoinTransaction *tx);

// fee that will be added for a transaction of the given size in bytes
uint64_t btcWalletFeeForTxSize(BRBitcoinWallet *wallet, size_t size);

// fee that will be added for a transaction of the given amount
uint64_t btcWalletFeeForTxAmount(BRBitcoinWallet *wallet, uint64_t amount);

// fee that will be added for a transaction of the given amount
// use feePerKb UINT64_MAX to indicate that the wallet feePerKb should be used
uint64_t btcWalletFeeForTxAmountWithFeePerKb(BRBitcoinWallet *wallet, uint64_t feePerKb, uint64_t amount);

// outputs below this amount are uneconomical due to fees (TX_MIN_OUTPUT_AMOUNT is the absolute minimum output amount)
uint64_t btcWalletMinOutputAmount(BRBitcoinWallet *wallet);

// outputs below this amount are uneconomical due to fees (TX_MIN_OUTPUT_AMOUNT is the absolute minimum output amount)
// use feePerKb UINT64_MAX to indicate that the wallet feePerKb should be used
uint64_t btcWalletMinOutputAmountWithFeePerKb(BRBitcoinWallet *wallet, uint64_t feePerKb);

// maximum amount that can be sent from the wallet to a single address after fees
uint64_t btcWalletMaxOutputAmount(BRBitcoinWallet *wallet);

// maximum amount that can be sent from the wallet to a single address after fees
// use feePerKb UINT64_MAX to indicate that the wallet feePerKb should be used
uint64_t btcWalletMaxOutputAmountWithFeePerKb(BRBitcoinWallet *wallet, uint64_t feePerKb);

// frees memory allocated for wallet, and calls btcTransactionFree() for all registered transactions
void btcWalletFree(BRBitcoinWallet *wallet);

// returns the given amount (in satoshis) in local currency units (i.e. pennies, pence)
// price is local currency units per bitcoin
int64_t btcLocalAmount(int64_t amount, double price);

// returns the given local currency amount in satoshis
// price is local currency units (i.e. pennies, pence) per bitcoin
int64_t btcBitcoinAmount(int64_t localAmount, double price);

#ifdef __cplusplus
}
#endif

#endif // BRWallet_h
