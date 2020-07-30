//
//  BRTezosWallet.c
//  Core
//
//  Created by Ehsan Rezaie on 2020-06-17.
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "BRTezosWallet.h"

#include <stdlib.h>
#include <assert.h>
#include "support/BRArray.h"
#include "BRTezosAddress.h"
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>


struct BRTezosWalletRecord {
    BRTezosAccount account;
    BRTezosUnitMutez balance;
    BRTezosFeeBasis feeBasis;

    BRArrayOf(BRTezosTransfer) transfers;

    pthread_mutex_t lock;
};

extern BRTezosWallet
tezosWalletCreate (BRTezosAccount account) {
    assert(account);
    BRTezosWallet wallet = calloc (1, sizeof(struct BRTezosWalletRecord));
    wallet->account = account;
    wallet->balance = 0;

    // Putting a '1' here avoids a 'false positive' in the Xcode leak instrument.
    array_new(wallet->transfers, 1);

    wallet->feeBasis = tezosDefaultFeeBasis();
    
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
tezosWalletFree (BRTezosWallet wallet) {
    if (wallet) {
        pthread_mutex_lock (&wallet->lock);
        for (size_t index = 0; index < array_count(wallet->transfers); index++)
            tezosTransferFree (wallet->transfers[index]);
        array_free(wallet->transfers);
        pthread_mutex_unlock (&wallet->lock);
        pthread_mutex_destroy (&wallet->lock);
        free(wallet);
    }
}

extern BRTezosAddress
tezosWalletGetSourceAddress (BRTezosWallet wallet) {
    assert(wallet);
    assert(wallet->account);
    // In the Tezos system there is only a single manager address per account
    // No need to clone the address here since the account code will do that
    return tezosAccountGetAddress (wallet->account);
}

extern BRTezosAddress
tezosWalletGetTargetAddress (BRTezosWallet wallet) {
    assert(wallet);
    assert(wallet->account);
    // In the Tezos system there is only a single manager address per account
    // No need to clone the address here since the account code will do that
    return tezosAccountGetAddress (wallet->account);
}

extern int
tezosWalletHasAddress (BRTezosWallet wallet,
                        BRTezosAddress address) {
    return tezosAccountHasAddress (wallet->account, address);
}

extern void
tezosWalletSetBalance (BRTezosWallet wallet, BRTezosUnitMutez balance) {
    assert(wallet);
    wallet->balance = balance;
}

extern BRTezosUnitMutez
tezosWalletGetBalance (BRTezosWallet wallet) {
    assert(wallet);
    return (wallet->balance);
}

extern BRTezosUnitMutez
tezosWalletGetBalanceLimit (BRTezosWallet wallet,
                            int asMaximum,
                            int *hasLimit) {
    assert (NULL != hasLimit);
    *hasLimit = 0;
    return 0;
}

extern void
tezosWalletSetDefaultFeeBasis (BRTezosWallet wallet, BRTezosFeeBasis feeBasis)
{
    assert(wallet);
    wallet->feeBasis = feeBasis;
}

extern BRTezosFeeBasis
tezosWalletGetDefaultFeeBasis (BRTezosWallet wallet)
{
    assert(wallet);
    return wallet->feeBasis;
}

static bool tezosTransferEqual(BRTezosTransfer t1, BRTezosTransfer t2) {
    // Equal means the same transaction id, source, target, amount
    bool result = false;
    BRTezosTransactionHash hash1 = tezosTransferGetTransactionId(t1);
    BRTezosTransactionHash hash2 = tezosTransferGetTransactionId(t2);
    if (memcmp(hash1.bytes, hash2.bytes, sizeof(hash1.bytes)) == 0) {
        // Hash is the same - compare the source
        BRTezosAddress source1 = tezosTransferGetSource(t1);
        BRTezosAddress source2 = tezosTransferGetSource(t2);
        if (1 == tezosAddressEqual(source1, source2)) {
            // OK - compare the target
            BRTezosAddress target1 = tezosTransferGetTarget(t1);
            BRTezosAddress target2 = tezosTransferGetTarget(t2);
            if (1 == tezosAddressEqual(target1, target2)) {
                result = tezosTransferGetAmount(t1) == tezosTransferGetAmount(t2);
            }
            tezosAddressFree(target1);
            tezosAddressFree(target2);
        }
        tezosAddressFree (source1);
        tezosAddressFree (source2);
    }
    return result;
}

static bool
walletHasTransfer (BRTezosWallet wallet, BRTezosTransfer transfer) {
    bool r = false;
    for (size_t index = 0; index < array_count(wallet->transfers) && false == r; index++) {
        r = tezosTransferEqual (transfer, wallet->transfers[index]);
    }
    return r;
}

extern int
tezosWalletHasTransfer (BRTezosWallet wallet, BRTezosTransfer transfer) {
    pthread_mutex_lock (&wallet->lock);
    int result = walletHasTransfer (wallet, transfer);
    pthread_mutex_unlock (&wallet->lock);
    return result;
}

extern void
tezosWalletAddTransfer (BRTezosWallet wallet, BRTezosTransfer transfer)
{
    assert(wallet);
    assert(transfer);
    pthread_mutex_lock (&wallet->lock);
    if (!walletHasTransfer(wallet, transfer)) {
        transfer = tezosTransferClone (transfer);
        array_add(wallet->transfers, transfer);

        // Update the balance
        BRTezosUnitMutez amount = tezosTransferGetAmount(transfer);
        BRTezosUnitMutez fee    = tezosTransferGetFee(transfer);

        BRTezosAddress accountAddress = tezosAccountGetAddress(wallet->account);
        BRTezosAddress source = tezosTransferGetSource(transfer);
        BRTezosAddress target = tezosTransferGetTarget(transfer);

        int isSource = tezosAccountHasAddress (wallet->account, source);
        int isTarget = tezosAccountHasAddress (wallet->account, target);

        if (isSource && isTarget)
            wallet->balance -= fee;
        else if (isSource)
            wallet->balance -= (amount + fee);
        else if (isTarget)
            wallet->balance += amount;
        else {
            // something is seriously wrong
        }
        tezosAddressFree (source);
        tezosAddressFree (target);
        tezosAddressFree (accountAddress);
    }
    pthread_mutex_unlock (&wallet->lock);
}

extern void
tezosWalletRemTransfer (BRTezosWallet wallet,
                        OwnershipKept BRTezosTransfer transfer)
{
    assert(wallet);
    assert(transfer);
    pthread_mutex_lock (&wallet->lock);
    if (walletHasTransfer(wallet, transfer)) {
        for (size_t index = 0; index < array_count(wallet->transfers); index++)
            if (tezosTransferEqual (transfer, wallet->transfers[index])) {
                tezosTransferFree(wallet->transfers[index]);
                array_rm (wallet->transfers, index);
                break;
            }

        // Update the balance
        BRTezosUnitMutez amount = tezosTransferGetAmount(transfer);
        BRTezosUnitMutez fee    = tezosTransferGetFee(transfer);

        BRTezosAddress accountAddress = tezosAccountGetAddress(wallet->account);
        BRTezosAddress source = tezosTransferGetSource(transfer);
        BRTezosAddress target = tezosTransferGetTarget(transfer);

        int isSource = tezosAccountHasAddress (wallet->account, source);
        int isTarget = tezosAccountHasAddress (wallet->account, target);

        if (isSource && isTarget)
            wallet->balance += fee;
        else if (isSource)
            wallet->balance += (amount + fee);
        else if (isTarget)
            wallet->balance -= amount;
        else {
            // something is seriously wrong
        }
        // Cleanup
        tezosAddressFree (source);
        tezosAddressFree (target);
        tezosAddressFree (accountAddress);
    }
    pthread_mutex_unlock (&wallet->lock);
}
