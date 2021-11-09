//
//  testBTCWalletManager.c
//  WalletKitCore Tests
//
//  Created by Bryan Goring on 11/08/21.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
#include <stdio.h>

#include "WKBase.h"
#include "WKAccount.h"
#include "walletkit/handlers/btc/WKBTC.h"
#include "walletkit/WKHandlersP.h"
#include "bitcoin/BRBitcoinWallet.h"
#include "WKCurrency.h"

/** Initialization data
 *
 */
static const char* testPaperKey = "journey baby slam jealous aware drama fall move tide vapor jelly ghost";
static const char* testAccountUids = "9f2328e3-4de1-436b-b856-82d4d6200351";

WKWalletManager createTestBTCManager(WKBoolean isMainnet) {

    // Create accounts object which is needed on Ethereum for signing
    const WKHandlers* btcHandlers = wkHandlersLookup(WK_NETWORK_TYPE_BTC);
    WKAccount accounts = wkAccountCreate(testPaperKey, 1514764800, testAccountUids, isMainnet);
    WKNetwork btcNetwork = wkNetworkFindBuiltin (isMainnet ? "bitcoin-mainnet" : "bitcoin-testnet",
                                                 false);
    
    // Create a minimal BTC wallet manager
    WKWalletManagerListener noMgrListener = {NULL, NULL};
    WKClient noClient = {0};
    WKWalletManager btcManager = btcHandlers->manager->create(noMgrListener,
                                                              noClient,
                                                              accounts,
                                                              btcNetwork,
                                                              WK_SYNC_MODE_P2P_ONLY,
                                                              WK_ADDRESS_SCHEME_NATIVE,
                                                              "./testData"    );
    assert (NULL != btcManager);

    return btcManager;
}

void releaseTestBTCManager(WKWalletManager mgr) {
    const WKHandlers* btcHandlers = wkHandlersLookup(WK_NETWORK_TYPE_BTC);
    btcHandlers->manager->release(mgr);
}

static void
runTestnetTransactionSigningTest() {
    
    WKWalletManager mgr = createTestBTCManager(false);
    WKWallet wallet = wkWalletManagerGetWallet (mgr);
    assert (NULL != wallet);
    
    WKWalletBTC walletBTC = wkWalletCoerceBTC(wallet);
    BRBitcoinWallet *wid = walletBTC->wid;
    
    bool isMainnet = false;
    UInt512 seed = wkAccountDeriveSeed(testPaperKey);
    
    WKCurrency btc = wkCurrencyCreate ("BitcoinUIDS",
                                       "Bitcoin",
                                       "BTC",
                                       "native",
                                       NULL);
        
    WKUnit sat = wkUnitCreateAsBase (btc,
                                     "SatoshiUIDS",
                                     "Satoshi",
                                     "SAT");
    assert (NULL != btc && NULL != sat);
    
    // Create a transfer from raw. As unsigned testnet transaction
    // captured from 'Java Demo App'.
    const char* testTransaction = "01000000011f4377da25e13b9d23430e"
                                  "57785bb865825bbfa5978e0388b9c9e1"
                                  "d9ad339fa4000000001600146e15535e"
                                  "2dbc3986983b5907b6f53fea16374617"
                                  "94c0000000000000ffffffff02004e00"
                                  "00000000001600149fc2c4cd715b139b"
                                  "a5e857157071e97da409c89eac710000"
                                  "0000000016001486121be30172b00b16"
                                  "8b3f24b9c39328680623dd00000000";

    // Create a transaction
    size_t rawSize;
    uint8_t *rawBytes = hexDecodeCreate(&rawSize, 
                                        testTransaction, 
                                        strlen (testTransaction));
    BRBitcoinTransaction *btcTransaction = btcTransactionParse (rawBytes, rawSize);
    assert (NULL != btcTransaction);
    
    btcTransaction->blockHeight = 2147483647;
    btcTransaction->timestamp = 0;
    
    // Create a transfer out of transaction
    WKTransferListener listener = { NULL };
    printf("  Create BTC transfer from transaction\n");
    WKTransfer btcTransfer = wkTransferCreateAsBTC (listener,
                                                    sat,
                                                    sat,
                                                    wid,
                                                    btcTransactionCopy(btcTransaction), // ownership given
                                                    WK_NETWORK_TYPE_BTC);
    assert (NULL != btcTransfer);
    
    const WKHandlers* btcHandlers = wkHandlersLookup(WK_NETWORK_TYPE_BTC);
    printf("  Sign transaction from seed using handler handler\n");
    WKBoolean res = btcHandlers->manager->signTransactionWithSeed(mgr,
                                                                  wallet,
                                                                  btcTransfer,
                                                                  seed);
    assert (WK_TRUE == res);
    releaseTestBTCManager(mgr);
}

extern void
runBTCWalletManagerTests (void) {
    
    printf("Testnet transaction signing test\n");
    runTestnetTransactionSigningTest();
    
    printf("BTCWalletManagerTests Done\n");
}
