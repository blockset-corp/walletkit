//
//  test__Name__.c
//  WalletKitCore
//
//  Created by __USER__ on __DATE__.
//  Copyright © __YEAR__ Breadwinner AG. All rights reserved.
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

#include "__name__/BR__Name__.h"

// MARK: - Hash Test

static void
run__Name__HashTest (void) {
    printf("TST:    __Name__ Hash tests\n");

    return;
}

// MARK: - Address Test

static void
run__Name__AddressTest (void) {
    printf("TST:    __Name__ Address tests\n");
    return;
}

// MARK: - Account Test

static void
run__Name__AccountTest (void) {
    printf("TST:    __Name__ Account tests\n");
   return;
}

// MARK: - Fee Basis Test

static void
run__Name__FeeBasisTest (void) {
    printf("TST:    __Name__ FeeBasis tests\n");
    return;
}

// MARK: - Transaction Test

static void
run__Name__TransactionCreateTest (void) {
    printf("TST:        __Name__ Transaction Create tests\n");
    return;
}

static void
run__Name__TransactionSignTest (void) {
    printf("TST:        __Name__ Transaction Sign tests\n");
   return;
}

static void
run__Name__TransactionSerializeTest (void) {
    printf("TST:        __Name__ Transaction Serialize tests\n");
    return;
}

static void
run__Name__TransactionTest (void) {
    printf("TST:    __Name__ Transaction tests\n");
    run__Name__TransactionCreateTest ();
    run__Name__TransactionSignTest ();
    run__Name__TransactionSerializeTest ();
}

// MARK: - Wallet Test

static void
run__Name__WalletTest (void) {
    printf("TST:    __Name__ Wallet tests\n");
   return;
}

// MARK: - All Tests

extern void
run__Name__Test (void /* ... */) {
    printf("TST: __Name__ tests\n");

    run__Name__HashTest ();
    run__Name__AddressTest();
    run__Name__AccountTest();
    run__Name__FeeBasisTest ();
    run__Name__TransactionTest ();
    run__Name__WalletTest ();
}

