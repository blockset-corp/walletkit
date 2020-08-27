//
//  BRRippleWallet.c
//  Core
//
//  Created by Carl Cherry on 5/3/2019.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include <stdlib.h>
#include <pthread.h>
#include "BRRippleWallet.h"
#include "support/BRArray.h"
#include "BRRipplePrivateStructs.h"
#include "BRRippleFeeBasis.h"
#include "BRRippleAddress.h"
#include <stdio.h>

#define RIPPLE_WALLET_MINIMUM_IN_XRP            (20)

//
// Wallet
//
struct BRRippleWalletRecord
{
    BRRippleBalance  balance;  // XRP balance
    BRRippleFeeBasis feeBasis; // Base fee for transactions

    // Ripple account
    BRRippleAccount account;

    BRArrayOf(BRRippleTransfer) transfers;

    pthread_mutex_t lock;
};

extern BRRippleWallet
rippleWalletCreate (BRRippleAccount account)
{
    BRRippleWallet wallet = (BRRippleWallet) calloc (1, sizeof(struct BRRippleWalletRecord));

    // To void a Xcode 'Leaks' Instrument false positive; use '1'.
    array_new(wallet->transfers, 1);

    wallet->account = account;
    wallet->balance = 0;
    wallet->feeBasis = (BRRippleFeeBasis) {
        10, 1
    };

    {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);

        pthread_mutex_init(&wallet->lock, &attr);
        pthread_mutexattr_destroy(&attr);
    }

    return wallet;
}

extern void
rippleWalletFree (BRRippleWallet wallet)
{
    if (wallet) {
        pthread_mutex_lock (&wallet->lock);
        for (size_t index = 0; index < array_count(wallet->transfers); index++)
            rippleTransferFree (wallet->transfers[index]);
        array_free(wallet->transfers);
        pthread_mutex_unlock (&wallet->lock);

        pthread_mutex_destroy (&wallet->lock);
        free(wallet);
    }
}

extern BRRippleAddress
rippleWalletGetSourceAddress (BRRippleWallet wallet)
{
    assert(wallet);
    assert(wallet->account);
    // NOTE - the following call will create a copy of the address
    // so we don't need to here as well
    return rippleAccountGetPrimaryAddress(wallet->account);
}

extern BRRippleAddress
rippleWalletGetTargetAddress (BRRippleWallet wallet)
{
    assert(wallet);
    assert(wallet->account);
    // NOTE - the following call will create a copy of the address
    // so we don't need to here as well
    return rippleAccountGetPrimaryAddress(wallet->account);
}

extern int
rippleWalletHasAddress (BRRippleWallet wallet,
                        BRRippleAddress address) {
    return rippleAccountHasAddress (wallet->account, address);
}

extern BRRippleBalance
rippleWalletGetBalance (BRRippleWallet wallet)
{
    assert(wallet);
    return wallet->balance;
}

extern void
rippleWalletSetBalance (BRRippleWallet wallet, BRRippleBalance balance)
{
    assert(wallet);
    wallet->balance = balance;
}

static void
rippleWalletUpdateBalance (BRRippleWallet wallet) {
    BRRippleUnitDrops balance = 0;
    BRRippleAddress accountAddress = rippleAccountGetAddress(wallet->account);

    for (size_t index = 0; index < array_count(wallet->transfers); index++) {
        int negative = 0;
        BRRippleUnitDrops amountDirected = rippleTransferGetAmountDirected (wallet->transfers[index],
                                                                            accountAddress,
                                                                            &negative);

        if (negative) balance -= amountDirected;
        else          balance += amountDirected;
    }
    rippleAddressFree (accountAddress);

    rippleWalletSetBalance (wallet, balance);
}

extern BRRippleUnitDrops
rippleWalletGetBalanceLimit (BRRippleWallet wallet,
                             int asMaximum,
                             int *hasLimit) {
    assert (NULL != hasLimit);
    
    *hasLimit = !asMaximum;
    return (asMaximum ? 0 : RIPPLE_XRP_TO_DROPS (RIPPLE_WALLET_MINIMUM_IN_XRP));
}

extern void rippleWalletSetDefaultFeeBasis (BRRippleWallet wallet, BRRippleFeeBasis feeBasis)
{
    assert(wallet);
    wallet->feeBasis = feeBasis;
}

extern BRRippleFeeBasis rippleWalletGetDefaultFeeBasis (BRRippleWallet wallet)
{
    assert(wallet);
    return wallet->feeBasis;
}

static bool rippleTransferEqual(BRRippleTransfer t1, BRRippleTransfer t2) {
    // Equal means the same transaction id, source, target
    bool result = false;
    BRRippleTransactionHash hash1 = rippleTransferGetTransactionId(t1);
    BRRippleTransactionHash hash2 = rippleTransferGetTransactionId(t2);
    if (memcmp(hash1.bytes, hash2.bytes, sizeof(hash1.bytes)) == 0) {
        // Hash is the same - compare the source
        BRRippleAddress source1 = rippleTransferGetSource(t1);
        BRRippleAddress source2 = rippleTransferGetSource(t2);
        if (1 == rippleAddressEqual(source1, source2)) {
            // OK - compare the target
            BRRippleAddress target1 = rippleTransferGetTarget(t1);
            BRRippleAddress target2 = rippleTransferGetTarget(t2);
            if (1 == rippleAddressEqual(target1, target2)) {
                result = true;
            }
            rippleAddressFree(target1);
            rippleAddressFree(target2);
        }
        rippleAddressFree (source1);
        rippleAddressFree (source2);
    }
    return result;
}

static bool
walletHasTransfer (BRRippleWallet wallet, BRRippleTransfer transfer) {
    bool r = false;
    for (size_t index = 0; index < array_count(wallet->transfers) && false == r; index++) {
        r = rippleTransferEqual (transfer, wallet->transfers[index]);
    }
    return r;
}

extern int rippleWalletHasTransfer (BRRippleWallet wallet, BRRippleTransfer transfer) {
    pthread_mutex_lock (&wallet->lock);
    int result = walletHasTransfer (wallet, transfer);
    pthread_mutex_unlock (&wallet->lock);
    return result;
}

static void rippleWalletUpdateSequence (BRRippleWallet wallet,
                                        OwnershipKept BRRippleAddress accountAddress) {
    // Now update the account's sequence id
    BRRippleSequence sequence = 0;
    // ... and the account's block-number-at-creation.
    uint64_t minBlockHeight = UINT64_MAX;

    for (size_t index = 0; index < array_count(wallet->transfers); index++) {
        BRRippleTransfer transfer = wallet->transfers[index];

        // If we are the source of the transfer then we might want to update our sequence number
        if (rippleTransferHasSource (wallet->transfers[index], accountAddress)) {
            // Update the sequence number if in a block OR successful
            if (rippleTransferIsInBlock(transfer) || !rippleTransferHasError(transfer))
                sequence += 1;
        } else if (!rippleTransferHasError(transfer) && rippleTransferHasTarget (transfer, accountAddress)) {
            // We are the target of the transfer - so we need to find the very first (successful) transfer where
            // our account received some XRP as this can affect our beginning sequence number. Ignore failed
            // transfers as Bockset could create a failed transfer for us before our account is created
            uint64_t blockHeight = rippleTransferGetBlockHeight(transfer);
            minBlockHeight = blockHeight < minBlockHeight ? blockHeight : minBlockHeight;
        }
    }

    rippleAccountSetBlockNumberAtCreation(wallet->account, minBlockHeight);
    rippleAccountSetSequence (wallet->account, sequence);
}

extern void rippleWalletAddTransfer (BRRippleWallet wallet,
                                     OwnershipKept BRRippleTransfer transfer)
{
    assert(wallet);
    assert(transfer);
    pthread_mutex_lock (&wallet->lock);
    if (!walletHasTransfer(wallet, transfer)) {
        // We'll add `transfer` to `wallet->transfers`; since we don't own `transfer` we must copy.
        transfer = rippleTransferClone(transfer);
        array_add(wallet->transfers, transfer);

        // Update the balance
        BRRippleAddress accountAddress = rippleAccountGetAddress(wallet->account);

        int negative = 0;
        BRRippleUnitDrops amountDirected = rippleTransferGetAmountDirected (transfer,
                                                                            accountAddress,
                                                                            &negative);

        if (negative) wallet->balance -= amountDirected;
        else          wallet->balance += amountDirected;

        rippleWalletUpdateSequence(wallet, accountAddress);
        rippleAddressFree (accountAddress);
    }
    pthread_mutex_unlock (&wallet->lock);
    // Now update the balance
}

extern void rippleWalletRemTransfer (BRRippleWallet wallet,
                                     OwnershipKept BRRippleTransfer transfer)
{
    assert(wallet);
    assert(transfer);
    pthread_mutex_lock (&wallet->lock);
    for (size_t index = 0; index < array_count(wallet->transfers); index++)
        if (rippleTransferEqual (transfer, wallet->transfers[index])) {
            // Save the transfer so we can free it, if needed.
            BRRippleTransfer walletTransfer = wallet->transfers[index];

            // Remove from the actual wallet.
            array_rm (wallet->transfers, index);

            // When removed, iterate over the remaining transfers to update balance and sequence.
            // This avoid issues with the removed transfer having an 'error' state and then uncertainty
            // on how to handle the transfer's amount.
            BRRippleAddress accountAddress = rippleAccountGetAddress(wallet->account);

            rippleWalletUpdateBalance  (wallet);
            rippleWalletUpdateSequence (wallet, accountAddress);

            rippleAddressFree (accountAddress);

            if (transfer != walletTransfer)
                rippleTransferFree(walletTransfer);

            break; // for
        }
    pthread_mutex_unlock (&wallet->lock);
}

extern void rippleWalletUpdateTransfer (BRRippleWallet wallet,
                                        OwnershipKept BRRippleTransfer transfer) {
    assert(wallet);
    assert(transfer);
    pthread_mutex_lock (&wallet->lock);
    if (walletHasTransfer (wallet, transfer)) {
        BRRippleAddress accountAddress = rippleAccountGetAddress(wallet->account);

        rippleWalletUpdateBalance  (wallet);
        rippleWalletUpdateSequence (wallet, accountAddress);

        rippleAddressFree (accountAddress);
    }
    pthread_mutex_unlock (&wallet->lock);
}
