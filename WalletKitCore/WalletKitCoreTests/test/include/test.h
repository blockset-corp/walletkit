//
//  test.h
//  CoreTests
//
//  Created by Ed Gamble on 2/14/19.
//  Copyright © 2019 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BR_Ethereum_Test_H
#define BR_Ethereum_Test_H

#include "ethereum/blockchain/BREthereumNetwork.h"
#include "ethereum/ewm/BREthereumAccount.h"
#include "BRCryptoSync.h"
#include "BRCryptoAccount.h"
#include "BRCryptoNetwork.h"

#ifdef __cplusplus
extern "C" {
#endif

// Util
extern void runUtilTests (void);

// RLP
extern void runRlpTests (void);

// Event
extern void runEventTests (void);

// Base
extern void runBaseTests (void);

// Block Chain
extern void runBcTests (void);

// Contract
extern void runContractTests (void);

// LES
extern void runLESTests(const char *paperKey);

extern void
runNodeTests (void);

// EWM
extern void
runEWMTests (const char *paperKey,
             const char *storagePath);
    
extern void
runSyncTest (BREthereumNetwork network,
             BREthereumAccount account,
             BRCryptoSyncMode mode,
             BREthereumTimestamp accountTimestamp,
             unsigned int durationInSeconds,
             const char *storagePath);

//
extern void
installTokensForTest (void);

extern void
runTests (int reallySend);

extern void
runPerfTestsCoder (int repeat, int many);

// Bitcoin
extern int BRRunSupTests (void);

extern int BRRunTests();

extern int BRRunTestsSync (const char *paperKey,
                           int isBTC,
                           int isMainnet);

extern int BRRunTestWalletManagerSync (const char *paperKey,
                                       const char *storagePath,
                                       int isBTC,
                                       int isMainnet);

extern int BRRunTestWalletManagerSyncStress(const char *paperKey,
                                            const char *storagePath,
                                            uint32_t earliestKeyTime,
                                            uint64_t blockHeight,
                                            int isBTC,
                                            int isMainnet);

extern int BRRunTestsBWM (const char *paperKey,
                          const char *storagePath,
                          int isBTC,
                          int isMainnet);

extern void BRRandInit (void);

// testCrypto.c
extern void runCryptoTests (void);

extern BRCryptoBoolean
runCryptoTestsWithAccountAndNetwork (BRCryptoAccount account,
                                     BRCryptoNetwork network,
                                     const char *storagePath);

// Ripple
extern void
runRippleTest (void /* ... */);

// Hedera
extern void
runHederaTest (void);

// Tezos
extern void
runTezosTest (void);

#ifdef __cplusplus
}
#endif

#endif /* BR_Ethereum_Test_H */
