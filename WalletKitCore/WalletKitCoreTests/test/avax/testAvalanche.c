//
//  testAvalanche.c
//  WalletKitCore
//
//  Created by Amit Shah on 2021-08-04.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// Key WalletKit includes
#include "support/BRArray.h"
#include "support/BRInt.h"

#include "avalanche/BRAvalanche.h"

// MARK: - Hash Test

static void
runAvalancheHashTest (void) {
    printf("TST:    Avalanche Hash tests\n");

    return;
}

// MARK: - Address Test

static void
runAvalancheAddressTest (void) {
    printf("TST:    Avalanche Address tests\n");
    return;
}

// MARK: - Account Test

static void
runAvalancheAccountTest (void) {
    printf("TST:    Avalanche Account tests\n");
   return;
}

// MARK: - Fee Basis Test

static void
runAvalancheFeeBasisTest (void) {
    printf("TST:    Avalanche FeeBasis tests\n");
    return;
}

// MARK: - Transaction Test

static void
runAvalancheTransactionCreateTest (void) {
    printf("TST:        Avalanche Transaction Create tests\n");
    return;
}

static void
runAvalancheTransactionSignTest (void) {
    printf("TST:        Avalanche Transaction Sign tests\n");
   return;
}

static void
runAvalancheTransactionSerializeTest (void) {
    printf("TST:        Avalanche Transaction Serialize tests\n");
    return;
}

static void
runAvalancheTransactionTest (void) {
    printf("TST:    Avalanche Transaction tests\n");
    runAvalancheTransactionCreateTest ();
    runAvalancheTransactionSignTest ();
    runAvalancheTransactionSerializeTest ();
}

// MARK: - Wallet Test

static void
runAvalancheWalletTest (void) {
    printf("TST:    Avalanche Wallet tests\n");
   return;
}

// MARK: - All Tests

extern void
runAvalancheTest (void /* ... */) {
    printf("TST: Avalanche tests\n");

    runAvalancheHashTest ();
    runAvalancheAddressTest();
    runAvalancheAccountTest();
    runAvalancheFeeBasisTest ();
    runAvalancheTransactionTest ();
    runAvalancheWalletTest ();
}

