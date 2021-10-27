/*
 * WalletKitCoreTests.c
 *
 *  Created on: Jun. 18, 2021
 *      Author: bryan
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ftw.h>
#include <assert.h>

#include "test.h"

#include "WKNetwork.h"
#include "WKCurrency.h"
#include "WKUnit.h"
#include "WKAmount.h"

/**
 * Test & supporting defs
 */
#define DEF_ACCT_SPEC   (0)
#define QUICK           (1)
#define SLOW            (!QUICK)
#define TEST_DATA_DIR   ("test_data")
#define SMALL_BUF       (32)
#define NO_ASSERTION    (1)

/**
 * Unit test prototype similar to WalletKitCoreTests.swift
 * Some tests return an integer return code to indicate success
 */
typedef int (*UnitTestCrypto)(void);

/**
 * Definition of a test having description and callout
 *
 */
typedef struct test {
    int             quick;
    const char*     desc;
    UnitTestCrypto  func;
} TestDef;

/**
 * Definition of an account
 */
typedef struct acct {
    char* identifier;
    char* paperKey;
    char* timestamp;
    char* network;
} AccountSpec;

/**
 * Test configuration information
 */
typedef struct configurations {
    bool        isMainnet;
    uint64_t    blockHeight;
} TestConfig;

/**
 * Global settings may be overridden by cmd line args
 */
static int              isMainnet               = 1;
static const char*      uids                    = "5766b9fa-e9aa-4b6d-9b77-b5f1136e5e96";
static BRBitcoinChain   bitcoinChain            = BITCOIN_CHAIN_BTC;
static int              quickTestsOnly          = 0;
static int              specificTest            = -1;
static AccountSpec      *accountSpecifications  = NULL;
static int              numberOfAccounts        = 1;

/**
 * Forward test function declarations
 */
int testCrypto                          (void);
int testCryptoWithAccountAndNetworkBTC  (void);
int testCryptoWithAccountAndNetworkBCH  (void);
int testCryptoWithAccountAndNetworkETH  (void);
int testRLPETH                          (void);
int testUtilETH                         (void);
int testEventETH                        (void);
int testBaseETH                         (void);
int testBlockchainETH                   (void);
int testContractETH                     (void);
int testBasicsETH                       (void);
int testRipple                          (void);
int testHedera                          (void);
int testTezos                           (void);
int testBitcoinSupport                  (void);
int testBitcoin                         (void);
int testBitcoinSyncOne                  (void);


/* & support functions */
WKNetwork   createBitcoinNetwork        (bool, uint64_t);
WKNetwork   createBitcoinCashNetwork    (bool, uint64_t);
WKNetwork   createEthereumNetwork       (bool, uint64_t);
void        createStoragePath           (void);
void        storagePathClear            (void);
void        setup                       (void);
void        teardown                    (void);

static TestDef allTests[] = {
    // Crypto
    {QUICK, "testCrypto",       testCrypto                          },
    {SLOW,  "testCryptoBTC",    testCryptoWithAccountAndNetworkBTC  },
    {SLOW,  "testCryptoBCH",    testCryptoWithAccountAndNetworkBCH  },
    {SLOW,  "testCryptoETH",    testCryptoWithAccountAndNetworkETH  },

    // Ethereum
    {QUICK, "testRLP",          testRLPETH                          },
 /*CORE*/   {QUICK, "testUtil",         testUtilETH                         },
    {QUICK, "testEvent",        testEventETH                        },
 /*CORE*/   {QUICK, "testBase",         testBaseETH                         },
    {QUICK, "testBC",           testBlockchainETH                   },
    {QUICK, "testContracdt",    testContractETH                     },
    {QUICK, "testBasics",       testBasicsETH                       },

    // Ripple
    {QUICK, "testRipple",       testRipple                          },

    // Hedera
    {QUICK, "testHedera",       testHedera                          },

    // Tezos
    {QUICK, "testTezos",        testTezos                           },

    // Bitcoin
    {QUICK, "testSupportBTC",   testBitcoinSupport                  },
    {QUICK, "testBTC",          testBitcoin                         },
    {SLOW,  "testSyncOneBTC",   testBitcoinSyncOne                  }
};
#define NUM_TESTS   (sizeof(allTests) / sizeof(TestDef))

/* --------------- Tests ----------------- */
int testCrypto(void) {
    runWalletKitTests();
    return NO_ASSERTION;
}

int testCryptoWithAccountAndNetworkBTC(void) {
    WKAccount   account;
    WKNetwork   network;
    int         success = 1;
    TestConfig  configurations[] = {
        {true,   500000 },
        {false, 1500000 }
    };

    account = wkAccountCreate (accountSpecifications[DEF_ACCT_SPEC].paperKey,
                               0,
                               uids);

    for (int config=0;
         config < (sizeof (configurations) / sizeof (struct configurations));
         config++) {

        storagePathClear();

        network = createBitcoinNetwork(configurations[config].isMainnet,
                                       configurations[config].blockHeight);

        success = success && runWalletKitTestsWithAccountAndNetwork (account,
                                                                  network,
                                                                  TEST_DATA_DIR);

        /*defer*/wkNetworkGive (network);
    }

    /*defer*/wkAccountGive (account);

    return success;
}

int testCryptoWithAccountAndNetworkBCH() {
    WKAccount   account;
    WKNetwork   network;
    int         success = 1;
    TestConfig  configurations[] = {
        {true,   500000 },
        {false, 1500000 }
    };

    account = wkAccountCreate (accountSpecifications[DEF_ACCT_SPEC].paperKey,
                               0,
                               uids);

    for (int config=0;
         config < (sizeof (configurations) / sizeof (struct configurations));
         config++) {

        storagePathClear();

        network = createBitcoinCashNetwork(configurations[config].isMainnet,
                                           configurations[config].blockHeight);

        success = success && runWalletKitTestsWithAccountAndNetwork (account,
                                                                  network,
                                                                  TEST_DATA_DIR);

        /*defer*/wkNetworkGive (network);
    }

    /*defer*/wkAccountGive (account);

    return success;
}

int testCryptoWithAccountAndNetworkETH(void) {
    WKAccount   account;
    WKNetwork   network;
    int         success = 1;
    TestConfig  configurations[] = {
        {true,  8000000  },
        {false, 4500000  }
    };

    account = wkAccountCreate (accountSpecifications[DEF_ACCT_SPEC].paperKey,
                               0,
                               uids);

    for (int config=0;
         config < (sizeof (configurations) / sizeof (struct configurations));
         config++) {

        storagePathClear();

        network = createEthereumNetwork(configurations[config].isMainnet,
                                        configurations[config].blockHeight);

        success = success && runWalletKitTestsWithAccountAndNetwork (account,
                                                                  network,
                                                                  TEST_DATA_DIR);

        /*defer*/wkNetworkGive (network);
    }

    /*defer*/wkAccountGive (account);

    return /*success*/NO_ASSERTION; // Parity with swift tests
}

int testRLPETH(void) {
    runRlpTests();
    return NO_ASSERTION;
}

int testUtilETH(void) {
    runUtilTests();
    return NO_ASSERTION;
}

int testEventETH(void) {
    runEventTests();
    return NO_ASSERTION;
}

int testBaseETH(void) {
    runBaseTests();
    return NO_ASSERTION;
}

int testBlockchainETH(void) {
    runBcTests();
    return NO_ASSERTION;
}

int testContractETH(void) {
    runContractTests();
    return NO_ASSERTION;
}

int testBasicsETH(void) {
    runTests(0);
    return NO_ASSERTION;
}

int testRipple(void) {
    runRippleTest();
    return NO_ASSERTION;
}

int testHedera(void) {
    runHederaTest();
    return NO_ASSERTION;
}

int testTezos(void) {
    runTezosTest();
    return NO_ASSERTION;
}

int testBitcoinSupport(void) {
    return BRRunSupTests();
    return NO_ASSERTION;
}

int testBitcoin(void) {
    return BRRunTests();
    return NO_ASSERTION;
}

int testBitcoinSyncOne() {
    BRRunTestsSync (accountSpecifications[DEF_ACCT_SPEC].paperKey,
                    bitcoinChain,
                    isMainnet);
    return NO_ASSERTION;
}

/* ---------- Support functions ------------- */

WKNetwork createBitcoinNetwork(
    bool        isMainnet,
    uint64_t    blockHeight ) {

    char            uids[SMALL_BUF];
    WKNetwork       network;
    WKCurrency      currency;
    WKUnit          satUnit;
    WKUnit          btcUnit;
    WKAmount        factor;
    WKNetworkFee    fee;

    snprintf(uids,
             SMALL_BUF,
             "bitcoin-%s",
             (isMainnet ? "mainnet" : "testnet"));

    network = wkNetworkFindBuiltin (uids, isMainnet);
    currency = wkCurrencyCreate ("bitcoin", "bitcoin", "btc", "native", NULL);
    satUnit = wkUnitCreateAsBase (currency, "sat", "satoshis", "SAT");
    btcUnit = wkUnitCreate (currency, "btc", "bitcoin", "B", satUnit, 8);
    factor = wkAmountCreateInteger (1000, satUnit);
    fee = wkNetworkFeeCreate (30000, factor, satUnit);

    wkNetworkSetHeight (network, blockHeight);
    wkNetworkAddCurrency (network, currency, satUnit, btcUnit);
    wkNetworkAddCurrencyUnit(network, currency, satUnit);
    wkNetworkAddCurrencyUnit(network, currency, btcUnit);
    wkNetworkAddNetworkFee(network, fee);

    network = wkNetworkTake (network);

    /*defer*/wkNetworkFeeGive (fee);
    /*defer*/wkAmountGive (factor);
    /*defer*/wkUnitGive (btcUnit);
    /*defer*/wkUnitGive (satUnit);
    /*defer*/wkCurrencyGive (currency);
    /*defer*/wkNetworkGive(network);

    return network;
}

WKNetwork createBitcoinCashNetwork(
    bool        isMainnet,
    uint64_t    blockHeight ) {

    char            uids[SMALL_BUF];
    WKNetwork       network;
    WKCurrency      currency;
    WKUnit          satUnit;
    WKUnit          btcUnit;
    WKAmount        factor;
    WKNetworkFee    fee;

    snprintf(uids,
             SMALL_BUF,
             "bitcoincash-%s",
             (isMainnet ? "mainnet" : "testnet"));

    network = wkNetworkFindBuiltin (uids, isMainnet);
    currency = wkCurrencyCreate ("bitcoin-cash", "bitcoin cash", "bch", "native", NULL);
    satUnit = wkUnitCreateAsBase (currency, "sat", "satoshis", "SAT");
    btcUnit = wkUnitCreate (currency, "btc", "bitcoin", "B", satUnit, 8);
    factor = wkAmountCreateInteger (1000, satUnit);
    fee = wkNetworkFeeCreate (30000, factor, satUnit);

    wkNetworkSetHeight (network, blockHeight);
    wkNetworkAddCurrency (network, currency, satUnit, btcUnit);
    wkNetworkAddCurrencyUnit(network, currency, satUnit);
    wkNetworkAddCurrencyUnit(network, currency, btcUnit);
    wkNetworkAddNetworkFee(network, fee);

    network = wkNetworkTake (network);

    /*defer*/wkNetworkFeeGive (fee);
    /*defer*/wkAmountGive (factor);
    /*defer*/wkUnitGive (btcUnit);
    /*defer*/wkUnitGive (satUnit);
    /*defer*/wkCurrencyGive (currency);
    /*defer*/wkNetworkGive(network);

    return network;
}

WKNetwork createEthereumNetwork(
    bool        isMainnet,
    uint64_t    blockHeight ) {

    char            uids[SMALL_BUF];
    WKNetwork       network;
    WKCurrency      currency;
    WKUnit          weiUnit;
    WKUnit          gweiUnit;
    WKUnit          etherUnit;
    WKAmount        factor;
    WKNetworkFee    fee;

    snprintf(uids,
             SMALL_BUF,
             "ethereum-%s",
             (isMainnet ? "mainnet" : "ropsten"));

    network = wkNetworkFindBuiltin (uids, isMainnet);
    currency = wkCurrencyCreate ("ethereum", "ethereum", "eth", "native", NULL);
    weiUnit = wkUnitCreateAsBase (currency, "wei", "wei", "wei");
    gweiUnit = wkUnitCreate (currency, "gwei", "gwei", "gwei", weiUnit, 9);
    etherUnit = wkUnitCreate (currency, "ether", "ether", "ether", weiUnit, 18);
    factor = wkAmountCreateDouble (2.0, gweiUnit);
    fee = wkNetworkFeeCreate (1000, factor, gweiUnit);

    wkNetworkSetHeight (network, blockHeight);
    wkNetworkAddCurrency (network, currency, weiUnit, etherUnit);
    wkNetworkAddCurrencyUnit(network, currency, weiUnit);
    wkNetworkAddCurrencyUnit(network, currency, gweiUnit);
    wkNetworkAddCurrencyUnit(network, currency, etherUnit);
    wkNetworkAddNetworkFee(network, fee);

    network = wkNetworkTake (network);

    /*defer*/wkNetworkFeeGive (fee);
    /*defer*/wkAmountGive (factor);
    /*defer*/wkUnitGive (etherUnit);
    /*defer*/wkUnitGive (gweiUnit);
    /*defer*/wkUnitGive (weiUnit);
    /*defer*/wkCurrencyGive (currency);
    /*defer*/wkNetworkGive(network);

    return network;
}

void createStoragePath() {
    struct stat s = {0};
    if (stat(TEST_DATA_DIR, &s) == -1) {
        mkdir(TEST_DATA_DIR, 0777);
    }
}

void storagePathClear() {
    char rmSubfolders[SMALL_BUF];
    snprintf (rmSubfolders,
              SMALL_BUF,
              "rm -rf %s/*",
              TEST_DATA_DIR);
    assert (system (rmSubfolders) == 0);
}

void setup() {

    accountSpecifications = calloc (1, sizeof(AccountSpec));
    accountSpecifications[DEF_ACCT_SPEC].identifier = strdup("ginger");
    accountSpecifications[DEF_ACCT_SPEC].paperKey = strdup("ginger settle marine tissue robot crane night number ramp coast roast critic");;
    accountSpecifications[DEF_ACCT_SPEC].timestamp = strdup("2018-01-01");
    accountSpecifications[DEF_ACCT_SPEC].network = isMainnet ? strdup("mainnet") : strdup("testnet");

    createStoragePath ();
}

void teardown() {

    AccountSpec *a;
    for (int acctNo = 0; acctNo < numberOfAccounts; acctNo++) {
        a = &accountSpecifications[acctNo];

        free (a->identifier);
        free (a->paperKey);
        free (a->timestamp);
        free (a->network);
    }
}

void usage(const char* nm) {

    printf("Usage: %s [t|q|h|n]\n", nm);
    printf("\n  where:");
    printf("\n    -n <test_num>: Run a specific test");
    printf("\n    -t: Use testnet (default mainnet)");
    printf("\n    -q: Run quick tests only (default false, runs all)");
    printf("\n    -h: Print this help & exit");
    printf("\n\n");
    printf("\n  Tests:\n");
    for (size_t i=0;i < NUM_TESTS;i++) {
        printf("    (%2lu) - %s\n", i, allTests[i].desc);
    }
}

void args(int argc, char* argv[]) {
    int option;

    while ((option = getopt(argc, argv, "tqhn:")) != -1) {
        if (option == 'h') {
            usage (argv[0]);
            exit (0);
        } else if (option == 't') {
            isMainnet = 0;
        } else if (option == 'q') {
            quickTestsOnly = 1;
        } else if (option == 'n') {
            specificTest = atoi (optarg);
            if (specificTest < 0 || specificTest >= NUM_TESTS) {
                fprintf (stderr, "Test # %s is invalid\n", optarg);
                usage (argv[0]);
                exit (-1);
            }
        }
    }
}

int main(int argc, char* argv[]) {

    args(argc, argv);

    printf("Using %s\n", (isMainnet ? "mainnet" : "testnet"));

    size_t totalTests = NUM_TESTS;
    size_t pass = 0;
    size_t skipped = 0;

    setup ();

    if (specificTest != -1) {
        totalTests = 1;
        printf("%d) %s\n", specificTest, allTests[specificTest].desc);
        if (allTests[specificTest].func()) {
            pass++;
        }
    } else {
        for (size_t testNo = 0; testNo < totalTests; testNo++) {

            printf("%lu) %s\n", testNo, allTests[testNo].desc);
            if (quickTestsOnly && !allTests[testNo].quick) {
                printf(" -- SKIP\n");
                skipped++;
                continue;
            }

            if (allTests[testNo].func()) {
                pass++;
            }
        }
    }

    if ((pass+skipped) == totalTests) {
        printf("[OK] %lu of %lu tests passed (%lu skipped)\n",
               pass, totalTests, skipped);
    } else {
        printf("[FAIL] %lu of %lu tests failed\n",
               (totalTests - pass), totalTests);
    }

    teardown ();
}
