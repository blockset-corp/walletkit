//
//  testCrypto.c
//  CoreTests
//
//  Created by Ed Gamble on 3/28/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "WKAmount.h"
#include "WKWallet.h"
#include "walletkit/WKNetworkP.h"
#include "walletkit/WKTransferP.h"
#include "walletkit/WKWalletP.h"
#include "walletkit/WKWalletManagerP.h"
#include "walletkit/WKSystemP.h"

#include "support/BRBIP32Sequence.h"
#include "support/BRBIP39Mnemonic.h"
#include "support/util/BRHex.h"
#include "bitcoin/BRBitcoinChainParams.h"
#include "bitcoin/BRBitcoinWallet.h"

#include "walletkit/handlers/btc/WKBTC.h"

#ifdef __ANDROID__
#include <android/log.h>
#define fprintf(...) __android_log_print(ANDROID_LOG_ERROR, "testCrypto", _va_rest(__VA_ARGS__, NULL))
#define printf(...) __android_log_print(ANDROID_LOG_INFO, "testCrypto", __VA_ARGS__)
#define _va_first(first, ...) first
#define _va_rest(first, ...) __VA_ARGS__
#endif

///
/// Mark: WKAmount Tests
///

static void
runCryptoAmountTests (void) {
    WKBoolean overflow;

    WKCurrency currency =
        wkCurrencyCreate ("Cuids",
                              "Cname",
                              "Ccode",
                              "Ctype",
                              NULL);

    WKUnit unitBase =
        wkUnitCreateAsBase (currency,
                                "UuidsBase",
                                "UnameBase",
                                "UsymbBase");

    WKUnit unitDef =
        wkUnitCreate (currency,
                          "UuidsDef",
                          "UnameDef",
                          "UsymbDef",
                          unitBase,
                          18);

    double value = 25.25434525155732538797258871;

    WKAmount amountInBase = wkAmountCreateDouble (value, unitBase);
    assert (NULL != amountInBase);

    double valueFromBase = wkAmountGetDouble (amountInBase, unitBase, &overflow);
    assert (WK_FALSE == overflow);
    assert (valueFromBase == 25.0);  // In base truncated fraction
    wkAmountGive(amountInBase);

    WKAmount amountInDef  = wkAmountCreateDouble (value, unitDef);
    assert (NULL != amountInDef);

    double valueFromDef = wkAmountGetDouble (amountInDef, unitDef, &overflow);
    assert (WK_FALSE == overflow);
    assert (fabs (valueFromDef - value) / value < 1e-10);
    wkAmountGive(amountInDef);

    value = 1e50;
    amountInBase = wkAmountCreateDouble (value, unitBase);
    assert (NULL != amountInBase);
    valueFromBase = wkAmountGetDouble (amountInBase, unitBase, &overflow);
    assert (WK_FALSE == overflow);
    assert (fabs (valueFromBase - value) / value < 1e-10);
    wkAmountGive(amountInBase);

    value = 1e100;
    amountInBase = wkAmountCreateDouble (value, unitBase);
    assert (NULL == amountInBase);

    wkUnitGive(unitDef);
    wkUnitGive(unitBase);
    wkCurrencyGive(currency);
}

///
/// Mark: WKTransfer Tests
///
typedef struct {
    const char *hash;  // reversed
    const char *rawChars;
    const char *input;
    const char *output;
    const char *change;
    uint32_t blockHeight;
    uint32_t timestamp;
} WKTransferTest;

static WKTransferTest transferTests[] = {
    {
        //    fee : UINT64_MAX (=> 0)
        //    send: 0
        //    recv: 200000000
        "0eb359c765e0feee7ffdb8b101f4dd0d4d42270c205fd7d476c49a692fa3745f",
        "01000000000101c4e3cb5f65d651d4c4c80c5ebdf0d8fa6360e9637f4ac8f624cbf56a1f32b5f10100000017160014bc755823b44e38d765020cd944e668c8992e86feffffffff0200c2eb0b000000001976a9143d533b77b6c288b41c7d94859401e201dcb188b488ac433838220b00000017a91486619a6825cbb20976e75b3563f4795cf2ceff53870247304402203ff43de94394e3ceb7227da8517e98d1364b4711eccda773ba1379faef36ccb00220586c62ef88b7603c74a5a061cb1019523e0b4d1b0fcd65a4cc909bea65ab914a0121023ceb81082ba53a11ab5ab5591f103f43c518fb10770a0876666a4aa569e9254000000000",
        "2N8P6KqChGTw6Nspx5mcgqz2V8LGSoPmJtr",
        "mm7DDqVkFd35XcWecFipfTYM5dByBzn7nq",
        "2N5VmTaJsha4a3mbbq1PDtwMmFbEiEMY4VL",
        1284270,
        1519245775
    },

    {
        //    fee : UINT64_MAX (=> 0)
        //    send: 0
        //    recv: 100000000
        "eed055d3a9f9de04d9722819846431e115e4ab9234146662d11af748b057026d",
        "01000000000101b52458f98187f71e5056660ae74a255242d95b08ce305dd66c8ef39e464adc2501000000171600149c89b47eef6454e350a8da516e4b78f0156ed94fffffffff0200e1f505000000001976a9143d533b77b6c288b41c7d94859401e201dcb188b488accc09457b0a00000017a9149e720b9c90893dd69e23957294501e756b47a2d78702483045022100f355621b5203ebe40b80a0f5050fa6f225b5c8c7d5e00cb2530444a40d13da47022041bcb9e865beb6d8b54ac0a2fa0e0334b61eaf43d4dab8fb32670c701dd84d0f012103c2ed9a20ee302c26674211f9dbf775cc17cacbdb1f8625a5f14930cc5c1ee96700000000",
        "2N2QZZaAU87oBQYL647L8MAbNgWD37NEJPL",
        "mm7DDqVkFd35XcWecFipfTYM5dByBzn7nq",
        "2N7h1NbxXsJUDZYT4LXsWKzVChFMgcVMxS7",
        1284273,
        1519247461
    },

    // This test fails, transfer is marked as RECEIVED (send == 0) but the sole input
    // address is our address.  Should be RECOVERED:
    {
        //    fee : 4800
        //    send: 200000000
        //    recv: 199395200
        "16ab9cbef30c836f409fadf960ff15da98a30066d75a7a93fe307650e55481ea",
        "01000000015f74a32f699ac476d4d75f200c27424d0dddf401b1b8fd7feefee065c759b30e000000006a47304402203eb5187c9e2463faa8bcf55fa461116c18c75cf2556205ba096fc482dde8e55d02203666c48b47abf7a244f40b6eaf0a80d9eb7e52d451234f37cb8c1fc45c7ae60a012102919c3832438df35734c714f76e7dc4a8c1b2f81812c3a08c99ef14cac4c14394ffffffff028087e20b000000001976a91403562150956f194d2dba88a271f2feabecc2102b88acc02709000000000017a914a9974100aeee974a20cda9a2f545704a0ab54fdc8700000000",
        "mm7DDqVkFd35XcWecFipfTYM5dByBzn7nq",
        "2N8hwP1WmJrFF5QWABn38y63uYLhnJYJYTF",
        "mfpbW4DXp3T7JBAKFWijHX96cktfWPR9z3",
        1284282,
        1519252723
    },

    {
        "ec54d3c24bcfe886408b8a250c1bf72466beef4efa1deb63f3a09b125055a06b",
        "01000000026d0257b048f71ad16266143492abe415e1316484192872d904def9a9d355d0ee000000006b4830450221009766181ecbc32fb9b5b08d7fe48f16067d8171695ba8fd15dc4eba310e00f54e02204bef6b60dc9e3c9b4c39a5b5751db6c50c348fd14ac39e97b4aba730314d5ffc012102919c3832438df35734c714f76e7dc4a8c1b2f81812c3a08c99ef14cac4c14394ffffffffea8154e5507630fe937a5ad76600a398da15ff60f9ad9f406f830cf3be9cab16000000006a4730440220765a9e2374b39b92b8da8b3c634622241483e5cece3815c92400d2f4fbfd9c1402200aa22d2435d5ff7e5404f96377100f26d6630e9404fb2c2c8e8ff3bd5c594b9d012102b173d5f2f39cdb935ba149d464e9d659726674df92430d82c56648fed56fce33ffffffff02a878e20b000000001976a914ac6b9e72cd4b52483241d1ca4dc796af619206fb88ac00e1f505000000001976a914a5bbef25bb37f8a87322a915225b7b3b1e6e6bd788ac00000000",
        "mm7DDqVkFd35XcWecFipfTYM5dByBzn7nq", //"mfpbW4DXp3T7JBAKFWijHX96cktfWPR9z3"
        "mvdGvbpCxedu5sHrFr1n515aQxfRRRy6xo",
        "mwEdSYVjXMCAkGZG2DRaaha1JedTh3s2u8",
        1284294,
        1519259668
    },

    // b590b72f94cbf22494e8d98dbb80b8b3fde81ca5304d1cbabaab83c7ecf60ecc
    // 7ea37279c35390218cda27aa1d954aaaaf931bcbc11d12bd26a3349ec111370b
    // 9c783522236d8ec7a00badedcea44c58b4f70269c4514177de53e35fae61e4ac
    // 92676f59615abab15d94e81b8c1847306a9eaa705f79984756ae807db3e2dc6e
#if 0
    {
        "78088f86996eefd8356c39240bb8c602855fe78f3340a444f52be24bf2dd2a39",
        "01000000000101bd35a8ce4fc3dfd23c739648a331d508505b7e056c05bb6d01d226f324c4a7fe0100000000ffffffff025c2b0000000000001976a914d3dfc8fe08d5057530ddcf3b2c9553f4f2d8b69d88accb0e8c000000000016001462c7ab74ba7058ef8b4977fcf815e8b32c7dd8f8024830450221009fd9d8bbb959114636fed6cb6c90ccba6c2f2dc9614e9b543eedc88f5429e954022073a4a5e46e94551fe508844d00ab08434eb9f63ca56266e708e0e480382f9461012103733084959349ecdf8a2439450a175b5384430cd98cc4e7d481f476cda4e1fe7000000000",
        "tb1qjm3y6vkqxg06xk4mukdm0xuz0zhzwz6mkyrln5",
        "mzqExPmf6WxCevri1Ahk675R1DutFCDeJY",
        "tb1qvtr6ka96wpvwlz6fwl70s90gkvk8mk8c42y5e7",
        1584449,
        1572444630
    },

    {
        "deed607db0f1ed401b74a4d4ea9274ce0dd87f01f93365e991ddd4ac484947c6",
        "010000000001018ce281d670b64fa8f3d67efb31c7b87d20614ed6980ebc334ab92aca45e01f740100000000ffffffff025c2b0000000000001976a9141963f033c786e862f078edb7e16baa920bf22b0388ac7b5cec0500000000160014afce19fce1d59a0e2ff765b08ae16326192d931602483045022100ed56bc226a22df292c3f6ac4d3a51dc4995262ceff04819c7923ff67f6f4cbb2022053d1323d9b75db6d6cd5a0a5e05ecfb67f93c9f01eb84f495907e8c119b0c11d01210264a76eb46cd4c6d47168ffb208d84e6dcf6508cb8b418e07f82523d259e92f8900000000",
        "tb1qlme4970h6xjsjw4l7s2k6fyrrs9phq9gdpgatq",
        "mhqCugcvqBXZs1Xo7zssaBe86oKDNkXSns",
        "tb1q4l8pnl8p6kdqutlhvkcg4ctrycvjmyckz309r7",
        1584447,
        1572443743
    }
#endif
};
static size_t numberOfTransferTests = sizeof (transferTests) / sizeof (WKTransferTest);

static BRMasterPubKey
transferTestsGetMPK (void) {
    const char *paperKey = "ginger settle marine tissue robot crane night number ramp coast roast critic";

    UInt512 seed = UINT512_ZERO;
    BRBIP39DeriveKey (seed.u8, paperKey, NULL);
    return BRBIP32MasterPubKey(&seed, sizeof (seed));
}

static void
transferTestsBalance (void) {
    BRMasterPubKey mpk = transferTestsGetMPK();

    BRBitcoinTransaction *transactions[numberOfTransferTests];
    for (size_t index = 0; index < numberOfTransferTests; index++) {
        WKTransferTest *test = &transferTests[index];

        size_t   testRawSize;
        uint8_t *testRawBytes = hexDecodeCreate(&testRawSize, test->rawChars, strlen (test->rawChars));

        transactions[index] = btcTransactionParse (testRawBytes, testRawSize);
        transactions[index]->blockHeight = test->blockHeight;
        transactions[index]->timestamp   = test->timestamp;
    }

    // Initialize wallet w/ all transactions
    BRBitcoinWallet *wid1 = btcWalletNew (btcTestNetParams->addrParams, transactions, numberOfTransferTests, mpk);
    uint64_t balance1 = btcWalletBalance(wid1);

    // Initialize wallet w/ each transaction, one by one
    BRBitcoinWallet *wid2 = btcWalletNew (btcTestNetParams->addrParams, NULL, 0, mpk);
    for (size_t index = 0; index < numberOfTransferTests; index++) {
        btcWalletRegisterTransaction (wid2, btcTransactionCopy (transactions[index]));
    }
    uint64_t balance2 = btcWalletBalance(wid2);

    // Initialize wallet w/ each transaction in reverse order
     BRBitcoinWallet *wid3 = btcWalletNew (btcTestNetParams->addrParams, NULL, 0, mpk);
    for (size_t index = 0; index < numberOfTransferTests; index++) {
        btcWalletRegisterTransaction (wid3, btcTransactionCopy(transactions[numberOfTransferTests - 1 - index]));
    }
    uint64_t balance3 = btcWalletBalance(wid3);

    assert (balance1 == balance2);
    assert (balance1 == balance3);

    btcWalletFree(wid3);
    btcWalletFree(wid2);
    btcWalletFree(wid1);
}


static void
transferTestsAddress (void) {
    WKCurrency btc =
    wkCurrencyCreate ("BitcoinUIDS",
                          "Bitcoin",
                          "BTC",
                          "native",
                          NULL);

    WKUnit sat =
    wkUnitCreateAsBase (btc,
                            "SatoshiUIDS",
                            "Satoshi",
                            "SAT");

    BRMasterPubKey mpk = transferTestsGetMPK();
    BRBitcoinWallet *wid = btcWalletNew (btcTestNetParams->addrParams, NULL, 0, mpk);
    btcWalletSetCallbacks (wid, NULL, NULL, NULL, NULL, NULL);

    for (size_t index = 0; index < numberOfTransferTests; index++) {
        WKTransferTest *test = &transferTests[index];

        size_t   testRawSize;
        uint8_t *testRawBytes = hexDecodeCreate(&testRawSize, test->rawChars, strlen (test->rawChars));

        BRBitcoinTransaction *tid = btcTransactionParse (testRawBytes, testRawSize);
        tid->blockHeight = test->blockHeight;
        tid->timestamp   = test->timestamp;
        btcWalletRegisterTransaction (wid, tid); // ownership given

        WKTransferListener listener = { NULL };
        WKTransfer transfer = wkTransferCreateAsBTC (listener,
                                                               sat,
                                                               sat,
                                                               wid,
                                                               btcTransactionCopy(tid), // ownership given
                                                               WK_NETWORK_TYPE_BTC);

        WKAddress sourceAddress = wkTransferGetSourceAddress(transfer);
        WKAddress targetAddress = wkTransferGetTargetAddress(transfer);
        char *source = wkAddressAsString (sourceAddress);
        char *target = wkAddressAsString (targetAddress);

        assert (0 == strcmp (test->input,  source));
        assert (0 == strcmp (test->output, target));

        free (testRawBytes);
        free (source); free (target);
        wkAddressGive(sourceAddress); wkAddressGive(targetAddress);
        wkTransferGive(transfer);
    }
    btcWalletFree(wid);
}

static void
runCryptoTransferTests (void) {
    transferTestsBalance();
    transferTestsAddress();
}

///
/// Mark: WKWalletManager Tests
///

// WKWalletManager Abuse Thread Routines

typedef struct {
    uint8_t kill;
    WKWalletManager manager;
    WKSyncMode primaryMode;
    WKSyncMode secondaryMode;
} CWMAbuseThreadState;

static void *
_CWMAbuseConnectThread (void *context) {
    CWMAbuseThreadState *state = (CWMAbuseThreadState *) context;
    while (!state->kill) {
        wkWalletManagerConnect (state->manager, NULL);
    }
    return NULL;
}

static void *
_CWMAbuseDisconnectThread (void *context) {
    CWMAbuseThreadState *state = (CWMAbuseThreadState *) context;
    while (!state->kill) {
        wkWalletManagerDisconnect (state->manager);
    }
    return NULL;
}

static void *
_CWMAbuseSyncThread (void *context) {
    CWMAbuseThreadState *state = (CWMAbuseThreadState *) context;
    while (!state->kill) {
        wkWalletManagerSync (state->manager);
    }
    return NULL;
}

static void *
_CWMAbuseSwapThread (void *context) {
    CWMAbuseThreadState *state = (CWMAbuseThreadState *) context;
    while (!state->kill) {
        if (state->primaryMode == wkWalletManagerGetMode (state->manager)) {
            wkWalletManagerSetMode (state->manager, state->secondaryMode);
        } else {
            wkWalletManagerSetMode (state->manager, state->primaryMode);
        }
    }
    return NULL;
}

// WKCWMClient NOP Callbacks

// TODO(fix): The below callbacks leak state

static void
_CWMNopGetBlockNumberCallback (WKClientContext context,
                               OwnershipGiven WKWalletManager manager,
                               OwnershipGiven WKClientCallbackState callbackState) {
    wkWalletManagerGive (manager);
}

static void
_CWMNopGetTransactionsCallback (WKClientContext context,
                                OwnershipGiven WKWalletManager manager,
                                OwnershipGiven WKClientCallbackState callbackState,
                                OwnershipKept const char **addresses,
                                size_t addressCount,
                                uint64_t begBlockNumber,
                                uint64_t endBlockNumber) {
    wkWalletManagerGive (manager);
}

static void
_CWMNopGetTransfersCallback (WKClientContext context,
                             OwnershipGiven WKWalletManager manager,
                             OwnershipGiven WKClientCallbackState callbackState,
                             OwnershipKept const char **addresses,
                             size_t addressCount,
                             uint64_t begBlockNumber,
                             uint64_t endBlockNumber) {
    wkWalletManagerGive (manager);
}

static void
_CWMNopSubmitTransactionCallback (WKClientContext context,
                                  OwnershipGiven WKWalletManager manager,
                                  OwnershipGiven WKClientCallbackState callbackState,
                                  OwnershipKept const char    *identifier,
                                  OwnershipKept const uint8_t *transaction,
                                  size_t transactionLength) {
    wkWalletManagerGive (manager);
}

static void
_CWMNopEstimateTransactionFeeCallback (WKClientContext context,
                                       OwnershipGiven WKWalletManager manager,
                                       OwnershipGiven WKClientCallbackState callbackState,
                                       OwnershipKept const uint8_t *transaction,
                                       size_t transactionLength,
                                       OwnershipKept const char *hashAsHex) {
    wkWalletManagerGive (manager);

}

// WKListener Event Wrapper

typedef enum {
    SYNC_EVENT_WALLET_MANAGER_TYPE,
    SYNC_EVENT_WALLET_TYPE,
    SYNC_EVENT_TXN_TYPE,
} CWMEventType;

typedef struct CWMEventRecord {
    CWMEventType type;
    union {
        struct {
            WKWalletManager manager;
            WKWalletManagerEvent event;
        } m;
        struct {
            WKWalletManager manager;
            WKWallet wallet;
            WKWalletEvent event;
        } w;
        struct {
            WKWalletManager manager;
            WKWallet wallet;
            WKTransfer transfer;
            WKTransferEvent event;
        } t;
    } u;
} CWMEvent;

const char *
CWMEventTypeString (CWMEventType type) {
    switch (type) {
        case SYNC_EVENT_WALLET_MANAGER_TYPE:
        return "SYNC_EVENT_WALLET_MANAGER_TYPE";
        case SYNC_EVENT_WALLET_TYPE:
        return "SYNC_EVENT_WALLET_TYPE";
        case SYNC_EVENT_TXN_TYPE:
        return "SYNC_EVENT_TXN_TYPE";
    }
}

static CWMEvent
CWMEventForWalletManagerType(WKWalletManagerEventType type) {
    return (CWMEvent) {
        SYNC_EVENT_WALLET_MANAGER_TYPE,
        {
            .m = {
                NULL,
                (WKWalletManagerEvent) {
                    type
                }
            }
        }
    };
}

static CWMEvent
CWMEventForWalletManagerStateType(WKWalletManagerEventType type,
                                  WKWalletManagerState oldState,
                                  WKWalletManagerState newState) {
    return (CWMEvent) {
        SYNC_EVENT_WALLET_MANAGER_TYPE,
        {
            .m = {
                NULL,
                (WKWalletManagerEvent) {
                    type,
                    {
                        .state = { oldState, newState }
                    }
                }
            }
        }
    };
}

static CWMEvent
CWMEventForWalletManagerWalletType(WKWalletManagerEventType type,
                                   WKWallet wallet) {
    return (CWMEvent) {
        SYNC_EVENT_WALLET_MANAGER_TYPE,
        {
            .m = {
                NULL,
                (WKWalletManagerEvent) {
                    type,
                    {
                        .wallet = wallet
                    }
                }
            }
        }
    };
}

static CWMEvent
CWMEventForWalletType(WKWalletEventType type) {
    return (CWMEvent) {
        SYNC_EVENT_WALLET_TYPE,
        {
            .w = {
                NULL,
                NULL,
                wkWalletEventCreate (type)
            }
        }
    };
}

static int
CWMEventEqual (CWMEvent *e1, CWMEvent *e2) {
    int success = 1;

    if (e1->type != e2->type) {
        success = 0;
    }

    if (success) {
        switch (e1->type) {
            case SYNC_EVENT_WALLET_MANAGER_TYPE: {
                if (e1->u.m.event.type != e2->u.m.event.type) {
                    success = 0;
                }

                switch (e1->u.m.event.type) {
                    case WK_WALLET_MANAGER_EVENT_CHANGED:
                        // Do we want to check iff the disconnect reason matched?
                        success = (e1->u.m.event.u.state.old.type == e2->u.m.event.u.state.old.type &&
                                   e1->u.m.event.u.state.new.type == e2->u.m.event.u.state.new.type);
                        break;
                    case WK_WALLET_MANAGER_EVENT_WALLET_ADDED:
                    case WK_WALLET_MANAGER_EVENT_WALLET_CHANGED:
                    case WK_WALLET_MANAGER_EVENT_WALLET_DELETED:
                        success = WK_TRUE == wkWalletEqual (e1->u.m.event.u.wallet, e2->u.m.event.u.wallet);
                        break;
                    case WK_WALLET_MANAGER_EVENT_SYNC_CONTINUES:
                    case WK_WALLET_MANAGER_EVENT_SYNC_RECOMMENDED:
                    case WK_WALLET_MANAGER_EVENT_BLOCK_HEIGHT_UPDATED:
                        // Do we want to check for this?
                    default:
                        break;
                }
                break;
            }
            case SYNC_EVENT_WALLET_TYPE: {
                success = WK_TRUE == wkWalletEventIsEqual (e1->u.w.event, e2->u.w.event);
                break;
            }
            case SYNC_EVENT_TXN_TYPE: {
                if (e1->u.t.event.type != e2->u.t.event.type) {
                    success = 0;
                }
                break;
            }
        }
    }

    return success;
}

static char *
CWMEventString (CWMEvent *e) {
    const char * subtypeString = NULL;
    const char * typeString = CWMEventTypeString (e->type);

    switch (e->type) {
        case SYNC_EVENT_WALLET_MANAGER_TYPE:
        subtypeString = wkWalletManagerEventTypeString (e->u.m.event.type);
        break;
        case SYNC_EVENT_WALLET_TYPE:
        subtypeString = wkWalletEventTypeString (wkWalletEventGetType (e->u.w.event));
        break;
        case SYNC_EVENT_TXN_TYPE:
        subtypeString = wkTransferEventTypeString (e->u.t.event.type);
        break;
    }

    const char * fmtString = "CWMEventString(%s -> %s)";
    size_t fmtStringLength = strlen (fmtString);

    size_t eventStringLength = fmtStringLength + strlen(typeString)  + strlen(subtypeString) + 1;
    char * eventString = calloc (eventStringLength, sizeof(char));

    snprintf (eventString, eventStringLength, fmtString, typeString, subtypeString);
    return eventString;
}

// WKListener Event Recording

typedef struct {
    WKBoolean silent;
    BRArrayOf(CWMEvent *) events;
    pthread_mutex_t lock;
} CWMEventRecordingState;

static void
CWMEventRecordingStateNew (CWMEventRecordingState *state,
                           WKBoolean isSilent) {
    state->silent = isSilent;
    array_new (state->events, 100);
    pthread_mutex_init (&state->lock, NULL);
}

static void
CWMEventRecordingStateNewDefault (CWMEventRecordingState *state) {
    CWMEventRecordingStateNew (state, WK_FALSE);
}

static void
CWMEventRecordingStateFree (CWMEventRecordingState *state) {
    for (size_t index = 0; index < array_count(state->events); index++)
        free (state->events[index]);
    array_free (state->events);

    pthread_mutex_destroy (&state->lock);
}

// TODO(fix): The below callbacks leak managers/wallets/transfers, as well as any ref counted event fields

static void
_CWMEventRecordingSystemCallback (WKListenerContext context,
                                  WKSystem system,
                                  WKSystemEvent event) {

}

static void
_CWMEventRecordingNetworkCallback (WKListenerContext context,
                                   WKNetwork network,
                                   WKNetworkEvent event) {

}

static void
_CWMEventRecordingManagerCallback (WKListenerContext context,
                                   WKWalletManager manager,
                                   WKWalletManagerEvent event) {
    CWMEventRecordingState *state = (CWMEventRecordingState*) context;
    CWMEvent *cwmEvent = calloc (1, sizeof (CWMEvent));
    cwmEvent->type = SYNC_EVENT_WALLET_MANAGER_TYPE;
    cwmEvent->u.m.manager= manager;
    cwmEvent->u.m.event = event;

    pthread_mutex_lock (&state->lock);
    array_add (state->events, cwmEvent);
    if (!state->silent) printf ("Added MANAGER event: %s (%zu total)\n", wkWalletManagerEventTypeString (event.type), array_count (state->events));
    pthread_mutex_unlock (&state->lock);
}

static void
_CWMEventRecordingWalletCallback (WKListenerContext context,
                                  WKWalletManager manager,
                                  WKWallet wallet,
                                  WKWalletEvent event) {
    CWMEventRecordingState *state = (CWMEventRecordingState*) context;
    CWMEvent *cwmEvent = calloc (1, sizeof (CWMEvent));
    cwmEvent->type = SYNC_EVENT_WALLET_TYPE;
    cwmEvent->u.w.manager= manager;
    cwmEvent->u.w.wallet= wallet;
    cwmEvent->u.w.event = event;

    pthread_mutex_lock (&state->lock);
    array_add (state->events, cwmEvent);
    if (!state->silent) printf ("Added WALLET event: %s (%zu total)\n", wkWalletEventTypeString (wkWalletEventGetType (event)), array_count (state->events));
    pthread_mutex_unlock (&state->lock);
}

static void
_CWMEventRecordingTransferCallback (WKListenerContext context,
                                    WKWalletManager manager,
                                    WKWallet wallet,
                                    WKTransfer transfer,
                                    WKTransferEvent event) {
    CWMEventRecordingState *state = (CWMEventRecordingState*) context;
    CWMEvent *cwmEvent = calloc (1, sizeof (CWMEvent));
    cwmEvent->type = SYNC_EVENT_TXN_TYPE;
    cwmEvent->u.t.manager= manager;
    cwmEvent->u.t.wallet= wallet;
    cwmEvent->u.t.transfer = transfer;
    cwmEvent->u.t.event = event;

    pthread_mutex_lock (&state->lock);
    array_add (state->events, cwmEvent);
    if (!state->silent) printf ("Added TXN event: %s (%zu total)\n", wkTransferEventTypeString (event.type), array_count (state->events));
    pthread_mutex_unlock (&state->lock);
}

#pragma clang diagnostic push
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-function"
static size_t
CWMEventRecordingGetNextEventIndexByType(CWMEventRecordingState *state,
                                         size_t startIndex,
                                         CWMEventType type) {
    for (size_t index = startIndex; index < array_count(state->events); index++) {
        if (state->events[index]->type == type) {
            return index;
        }
    }
    return SIZE_MAX;
}

static size_t
CWMEventRecordingGetNextWalletManagerEventIndexForWallet(CWMEventRecordingState *state,
                                                         size_t startIndex,
                                                         WKWallet wallet) {
    for (size_t index = startIndex; index < array_count(state->events); index++) {
        if (state->events[index]->type == SYNC_EVENT_WALLET_MANAGER_TYPE) {
            switch (state->events[index]->u.m.event.type) {
                case WK_WALLET_MANAGER_EVENT_WALLET_ADDED:
                case WK_WALLET_MANAGER_EVENT_WALLET_CHANGED:
                case WK_WALLET_MANAGER_EVENT_WALLET_DELETED:
                    if (state->events[index]->u.m.event.u.wallet == wallet) {
                        return index;
                    }
                default:
                    continue;
            }
        }
    }
    return SIZE_MAX;
}

static size_t
CWMEventRecordingGetNextWalletManagerEventIndexForState(CWMEventRecordingState *state,
                                                        size_t startIndex) {
    for (size_t index = startIndex; index < array_count(state->events); index++) {
        if (state->events[index]->type == SYNC_EVENT_WALLET_MANAGER_TYPE) {
            switch (state->events[index]->u.m.event.type) {
                case WK_WALLET_MANAGER_EVENT_CREATED:
                case WK_WALLET_MANAGER_EVENT_CHANGED:
                case WK_WALLET_MANAGER_EVENT_DELETED:
                case WK_WALLET_MANAGER_EVENT_SYNC_STARTED:
                case WK_WALLET_MANAGER_EVENT_SYNC_CONTINUES:
                case WK_WALLET_MANAGER_EVENT_SYNC_STOPPED:
                    return index;
                default:
                    break;
            }
        }
    }
    return SIZE_MAX;
}
#pragma clang diagnostic pop
#pragma GCC diagnostic pop

static int
CWMEventRecordingVerifyEventPairs (CWMEventRecordingState *state) {
    int success = 1;
    pthread_mutex_lock (&state->lock);

    // TODO(fix): Implement this; verifying valid event pairs of logically connected events such as:
    //              - WK_WALLET_MANAGER_EVENT_ created/changed/deleted/sync
    //              - WK_WALLET_MANAGER_EVENT_WALLET_ added/changed/deleted

    pthread_mutex_unlock (&state->lock);
    return success;
}

static int
CWMEventRecordingVerifyEventSequence (CWMEventRecordingState *state,
                                      WKBoolean isCompleteSequence,
                                      CWMEvent *expected,
                                      size_t expectedCount,
                                      CWMEvent *ignored,
                                      size_t ignoredCount) {
    int success = 1;
    pthread_mutex_lock (&state->lock);

    size_t index = 0;
    size_t count = array_count (state->events);
    size_t expectedIndex = 0;

    for (; success && index < count; index++) {
        // We've found every expected event and we're not looking for a complete sequence, so
        // just break out
        if (!isCompleteSequence && expectedIndex == expectedCount) {
            break;

        // Check for the expected event
        } else if ((success = (expectedIndex < expectedCount &&
                               CWMEventEqual (state->events[index], &expected[expectedIndex])))) {
            expectedIndex++;

        // We didn't get the expected event, see if this is an ignored event
        } else {
            for (size_t ignoredIndex = 0; !success && ignoredIndex < ignoredCount; ignoredIndex++) {
                success = CWMEventEqual (state->events[index], &ignored[ignoredIndex]);
            }
        }

        if (success) {
            // do nothing

        } else if (expectedIndex >= expectedCount) {
            printf("%s: failed due to unexpected event (received at idx %zu -> %s)\n",
                   __func__,
                   index,
                   CWMEventString (state->events[index]));

        } else if (!success) {
            printf("%s: failed due to mismatched event types (expected at idx %zu -> %s, received at idx %zu -> %s)\n",
                   __func__,
                   expectedIndex,
                   CWMEventString (&expected[expectedIndex]),
                   index,
                   CWMEventString (state->events[index]));
        }
    }

    pthread_mutex_unlock (&state->lock);

    if (success &&
        isCompleteSequence && index != count && expectedIndex == expectedCount) {
        success = 0;
        printf("%s: failed due to reaching the end of expected events but more occurred\n",
               __func__);
    }

    if (success &&
        index == count && expectedIndex != expectedCount) {
        success = 0;
        printf("%s: failed due to reaching the end of occurred events but more expected\n",
               __func__);
    }

    return success;
}

///
/// Mark: Lifecycle Tests
///

static WKWalletManager
wkWalletManagerSetupForLifecycleTest (CWMEventRecordingState *state,
                                            WKAccount account,
                                            WKNetwork network,
                                            WKSyncMode mode,
                                            WKAddressScheme scheme,
                                            const char *storagePath)
{
    WKListener listener = wkListenerCreate (state,
                                                     _CWMEventRecordingSystemCallback,
                                                     _CWMEventRecordingNetworkCallback,
                                                     _CWMEventRecordingManagerCallback,
                                                     _CWMEventRecordingWalletCallback,
                                                     _CWMEventRecordingTransferCallback);

    WKClient client = (WKClient) {
        state,
        _CWMNopGetBlockNumberCallback,
        _CWMNopGetTransactionsCallback,
        _CWMNopGetTransfersCallback,
        _CWMNopSubmitTransactionCallback,
        _CWMNopEstimateTransactionFeeCallback
    };

    WKSystem system = wkSystemCreate (client, listener, account, storagePath, wkNetworkIsMainnet(network));

    return wkWalletManagerCreate (wkListenerCreateWalletManagerListener (listener, system),
                                      client,
                                      account,
                                      network,
                                      mode,
                                      scheme,
                                      storagePath);
}

static int
runCryptoWalletManagerLifecycleTest (WKAccount account,
                                     WKNetwork network,
                                     WKSyncMode mode,
                                     WKAddressScheme scheme,
                                     const char *storagePath) {
    int success = 1;

    // HACK: Managers set the height; we need to be able to restore it between tests
    WKBlockNumber originalNetworkHeight = wkNetworkGetHeight (network);

    printf("Testing WKWalletManager events for mode=\"%s\", network=\"%s (%s)\" and path=\"%s\"...\n",
           wkSyncModeString (mode),
           wkNetworkGetName (network),
           wkNetworkIsMainnet (network) ? "mainnet" : "testnet",
           storagePath);

   printf("Testing WKWalletManager connect, disconnect...\n");
   {
       // Test setup
       CWMEventRecordingState state = {0};
       CWMEventRecordingStateNewDefault (&state);

       WKWalletManager manager = wkWalletManagerSetupForLifecycleTest(&state, account, network, mode, scheme, storagePath);
       WKWallet wallet = wkWalletManagerGetWallet (manager);

       // connect, disconnect
       wkWalletManagerConnect (manager, NULL);
       sleep(1);
       wkWalletManagerDisconnect (manager);
       sleep(1);
       wkWalletManagerStop (manager);

       // Verification
       success = CWMEventRecordingVerifyEventSequence(&state,
                                                      WK_TRUE,
                                                      (CWMEvent []) {
                                                          // wkWalletManagerCreate()
                                                          CWMEventForWalletManagerType         (WK_WALLET_MANAGER_EVENT_CREATED),
                                                          CWMEventForWalletType                (WK_WALLET_EVENT_CREATED),
                                                          CWMEventForWalletManagerWalletType   (WK_WALLET_MANAGER_EVENT_WALLET_ADDED,
                                                                                                    wallet),
                                                           // wkWalletManagerConnect()
                                                          CWMEventForWalletManagerStateType    (WK_WALLET_MANAGER_EVENT_CHANGED,
                                                                                                    wkWalletManagerStateInit (WK_WALLET_MANAGER_STATE_CREATED),
                                                                                                    wkWalletManagerStateInit (WK_WALLET_MANAGER_STATE_CONNECTED)),
                                                          CWMEventForWalletManagerType         (WK_WALLET_MANAGER_EVENT_SYNC_STARTED),
                                                          CWMEventForWalletManagerStateType    (WK_WALLET_MANAGER_EVENT_CHANGED,
                                                                                                    wkWalletManagerStateInit (WK_WALLET_MANAGER_STATE_CONNECTED),
                                                                                                    wkWalletManagerStateInit (WK_WALLET_MANAGER_STATE_SYNCING)),
                                                           // wkWalletManagerDisconnect()
                                                          CWMEventForWalletManagerType         (WK_WALLET_MANAGER_EVENT_SYNC_STOPPED),
                                                          CWMEventForWalletManagerStateType    (WK_WALLET_MANAGER_EVENT_CHANGED,
                                                                                                    wkWalletManagerStateInit (WK_WALLET_MANAGER_STATE_SYNCING),
                                                                                                    wkWalletManagerStateInit (WK_WALLET_MANAGER_STATE_CONNECTED)),
                                                          CWMEventForWalletManagerStateType    (WK_WALLET_MANAGER_EVENT_CHANGED,
                                                                                                    wkWalletManagerStateInit (WK_WALLET_MANAGER_STATE_CONNECTED),
                                                                                                    wkWalletManagerStateDisconnectedInit (wkWalletManagerDisconnectReasonUnknown())),
                                                      },
                                                      9,
                                                      (CWMEvent []) {
                                                          CWMEventForWalletManagerType (WK_WALLET_MANAGER_EVENT_SYNC_CONTINUES),
                                                          CWMEventForWalletManagerType (WK_WALLET_MANAGER_EVENT_BLOCK_HEIGHT_UPDATED),
                                                      },
                                                      2);
       if (!success) {
           fprintf(stderr, "***FAILED*** %s:%d: BRRunTestWalletManagerSyncTestVerifyEventSequence failed\n", __func__, __LINE__);
           return success;
       }

       success = CWMEventRecordingVerifyEventPairs (&state);
       if (!success) {
           fprintf(stderr, "***FAILED*** %s:%d: BRRunTestWalletManagerSyncTestVerifyEventPairs failed\n", __func__, __LINE__);
           return success;
       }

       // Test teardown
       wkNetworkSetHeight (network, originalNetworkHeight);
       wkWalletGive (wallet);
       wkWalletManagerGive (manager);
       CWMEventRecordingStateFree (&state);
   }

    printf("Testing WKWalletManager repeated connect attempts...\n");
    {
        // Test setup
        CWMEventRecordingState state = {0};
        CWMEventRecordingStateNewDefault (&state);

        WKWalletManager manager = wkWalletManagerSetupForLifecycleTest(&state, account, network, mode, scheme, storagePath);
        WKWallet wallet = wkWalletManagerGetWallet (manager);

        // repeated connect attempts
        wkWalletManagerConnect (manager, NULL);
        sleep(1);
        wkWalletManagerConnect (manager, NULL);
        sleep(1);
        wkWalletManagerConnect (manager, NULL);
        sleep(1);
        wkWalletManagerDisconnect (manager);
        sleep(1);
        wkWalletManagerStop (manager);

        // Verification
        success = CWMEventRecordingVerifyEventSequence(&state,
                                                       WK_TRUE,
                                                       (CWMEvent []) {
                                                           // wkWalletManagerCreate()
                                                           CWMEventForWalletManagerType         (WK_WALLET_MANAGER_EVENT_CREATED),
                                                           CWMEventForWalletType                (WK_WALLET_EVENT_CREATED),
                                                           CWMEventForWalletManagerWalletType   (WK_WALLET_MANAGER_EVENT_WALLET_ADDED,
                                                                                                     wallet),
                                                            // wkWalletManagerConnect() - first, second and third do nothing
                                                           CWMEventForWalletManagerStateType    (WK_WALLET_MANAGER_EVENT_CHANGED,
                                                                                                     wkWalletManagerStateInit (WK_WALLET_MANAGER_STATE_CREATED),
                                                                                                     wkWalletManagerStateInit (WK_WALLET_MANAGER_STATE_CONNECTED)),
                                                           CWMEventForWalletManagerType         (WK_WALLET_MANAGER_EVENT_SYNC_STARTED),
                                                           CWMEventForWalletManagerStateType    (WK_WALLET_MANAGER_EVENT_CHANGED,
                                                                                                     wkWalletManagerStateInit (WK_WALLET_MANAGER_STATE_CONNECTED),
                                                                                                     wkWalletManagerStateInit (WK_WALLET_MANAGER_STATE_SYNCING)),
                                                            // wkWalletManagerDisconnect()
                                                           CWMEventForWalletManagerType         (WK_WALLET_MANAGER_EVENT_SYNC_STOPPED),
                                                           CWMEventForWalletManagerStateType    (WK_WALLET_MANAGER_EVENT_CHANGED,
                                                                                                     wkWalletManagerStateInit (WK_WALLET_MANAGER_STATE_SYNCING),
                                                                                                     wkWalletManagerStateInit (WK_WALLET_MANAGER_STATE_CONNECTED)),
                                                           CWMEventForWalletManagerStateType    (WK_WALLET_MANAGER_EVENT_CHANGED,
                                                                                                     wkWalletManagerStateInit (WK_WALLET_MANAGER_STATE_CONNECTED),
                                                                                                     wkWalletManagerStateDisconnectedInit (wkWalletManagerDisconnectReasonUnknown())),
                                                       },
                                                       9,
                                                       (CWMEvent []) {
                                                           CWMEventForWalletManagerType (WK_WALLET_MANAGER_EVENT_SYNC_CONTINUES),
                                                           CWMEventForWalletManagerType (WK_WALLET_MANAGER_EVENT_BLOCK_HEIGHT_UPDATED),
                                                       },
                                                       2);
        if (!success) {
            fprintf(stderr, "***FAILED*** %s:%d: BRRunTestWalletManagerSyncTestVerifyEventSequence failed\n", __func__, __LINE__);
            return success;
        }

        success = CWMEventRecordingVerifyEventPairs (&state);
        if (!success) {
            fprintf(stderr, "***FAILED*** %s:%d: BRRunTestWalletManagerSyncTestVerifyEventPairs failed\n", __func__, __LINE__);
            return success;
        }

        // Test teardown
        wkNetworkSetHeight (network, originalNetworkHeight);
        wkWalletGive (wallet);
        wkWalletManagerGive (manager);
        CWMEventRecordingStateFree (&state);
    }

    printf("Testing BRWalletManager connect, scan, disconnect...\n");
    {
        // TODO(fix): Add this test; issue here is the variance in P2P, namely that there are multiple possible
        //            cases depending on peer connections:
        //      - a reconnect, if the BRWalletManagerScan was after the P2P connection and after receiving blocks (case 1)
        //      - a sync restart, if the BRWalletManagerScan was after the P2P connection but before the sync received blocks (case 2)
        //      - a sync, if the BRWalletManagerScan beat the P2P connections being established (case 3)
    }

    printf("Testing WKWalletManager repeated disconnect attempts...\n");
    {
        // Test setup
        CWMEventRecordingState state = {0};
        CWMEventRecordingStateNewDefault (&state);

        WKWalletManager manager = wkWalletManagerSetupForLifecycleTest(&state, account, network, mode, scheme, storagePath);
        WKWallet wallet = wkWalletManagerGetWallet (manager);

        // repeated disconnect attempts
        wkWalletManagerDisconnect (manager);
        sleep(1);
        wkWalletManagerDisconnect (manager);
        sleep(1);
        wkWalletManagerDisconnect (manager);
        sleep(1);
        wkWalletManagerStop (manager);

        // Verification
        success = CWMEventRecordingVerifyEventSequence(&state,
                                                       WK_TRUE,
                                                       (CWMEvent []) {
                                                           // wkWalletManagerCreate()
                                                           CWMEventForWalletManagerType         (WK_WALLET_MANAGER_EVENT_CREATED),
                                                           CWMEventForWalletType                (WK_WALLET_EVENT_CREATED),
                                                           CWMEventForWalletManagerWalletType   (WK_WALLET_MANAGER_EVENT_WALLET_ADDED,
                                                                                                     wallet),
                                                       },
                                                       3,
                                                       (CWMEvent []) {
                                                           CWMEventForWalletManagerType (WK_WALLET_MANAGER_EVENT_SYNC_CONTINUES),
                                                           CWMEventForWalletManagerType (WK_WALLET_MANAGER_EVENT_BLOCK_HEIGHT_UPDATED),
                                                       },
                                                       2);
        if (!success) {
            fprintf(stderr, "***FAILED*** %s:%d: BRRunTestWalletManagerSyncTestVerifyEventSequence failed\n", __func__, __LINE__);
            return success;
        }

        success = CWMEventRecordingVerifyEventPairs (&state);
        if (!success) {
            fprintf(stderr, "***FAILED*** %s:%d: BRRunTestWalletManagerSyncTestVerifyEventPairs failed\n", __func__, __LINE__);
            return success;
        }

        // Test teardown
        wkNetworkSetHeight (network, originalNetworkHeight);
        wkWalletGive (wallet);
        wkWalletManagerGive (manager);
        CWMEventRecordingStateFree (&state);
    }

    printf("Testing WKWalletManager sync, connect, disconnect...\n");
    {
        // Test setup
        CWMEventRecordingState state = {0};
        CWMEventRecordingStateNewDefault (&state);

        WKWalletManager manager = wkWalletManagerSetupForLifecycleTest(&state, account, network, mode, scheme, storagePath);
        WKWallet wallet = wkWalletManagerGetWallet (manager);

        // sync, connect, disconnect
        wkWalletManagerSync (manager);
        sleep(1);
        wkWalletManagerConnect (manager, NULL);
        sleep(1);
        wkWalletManagerDisconnect (manager);
        sleep(1);
        wkWalletManagerStop (manager);

        // Verification
        success = CWMEventRecordingVerifyEventSequence(&state,
                                                       WK_TRUE,
                                                       (CWMEvent []) {
                                                           // wkWalletManagerCreate()
                                                           CWMEventForWalletManagerType         (WK_WALLET_MANAGER_EVENT_CREATED),
                                                           CWMEventForWalletType                (WK_WALLET_EVENT_CREATED),
                                                           CWMEventForWalletManagerWalletType   (WK_WALLET_MANAGER_EVENT_WALLET_ADDED,
                                                                                                     wallet),
                                                            // wkWalletManagerConnect()
                                                           CWMEventForWalletManagerStateType    (WK_WALLET_MANAGER_EVENT_CHANGED,
                                                                                                     wkWalletManagerStateInit (WK_WALLET_MANAGER_STATE_CREATED),
                                                                                                     wkWalletManagerStateInit (WK_WALLET_MANAGER_STATE_CONNECTED)),
                                                           CWMEventForWalletManagerType         (WK_WALLET_MANAGER_EVENT_SYNC_STARTED),
                                                           CWMEventForWalletManagerStateType    (WK_WALLET_MANAGER_EVENT_CHANGED,
                                                                                                     wkWalletManagerStateInit (WK_WALLET_MANAGER_STATE_CONNECTED),
                                                                                                     wkWalletManagerStateInit (WK_WALLET_MANAGER_STATE_SYNCING)),
                                                            // wkWalletManagerDisconnect()
                                                           CWMEventForWalletManagerType         (WK_WALLET_MANAGER_EVENT_SYNC_STOPPED),
                                                           CWMEventForWalletManagerStateType    (WK_WALLET_MANAGER_EVENT_CHANGED,
                                                                                                     wkWalletManagerStateInit (WK_WALLET_MANAGER_STATE_SYNCING),
                                                                                                     wkWalletManagerStateInit (WK_WALLET_MANAGER_STATE_CONNECTED)),
                                                           CWMEventForWalletManagerStateType    (WK_WALLET_MANAGER_EVENT_CHANGED,
                                                                                                     wkWalletManagerStateInit (WK_WALLET_MANAGER_STATE_CONNECTED),
                                                                                                     wkWalletManagerStateDisconnectedInit (wkWalletManagerDisconnectReasonUnknown())),
                                                       },
                                                       9,
                                                       (CWMEvent []) {
                                                           CWMEventForWalletManagerType (WK_WALLET_MANAGER_EVENT_SYNC_CONTINUES),
                                                           CWMEventForWalletManagerType (WK_WALLET_MANAGER_EVENT_BLOCK_HEIGHT_UPDATED),
                                                       },
                                                       2);
        if (!success) {
            fprintf(stderr, "***FAILED*** %s:%d: BRRunTestWalletManagerSyncTestVerifyEventSequence failed\n", __func__, __LINE__);
            return success;
        }

        success = CWMEventRecordingVerifyEventPairs (&state);
        if (!success) {
            fprintf(stderr, "***FAILED*** %s:%d: BRRunTestWalletManagerSyncTestVerifyEventPairs failed\n", __func__, __LINE__);
            return success;
        }

        // Test teardown
        wkNetworkSetHeight (network, originalNetworkHeight);
        wkWalletGive (wallet);
        wkWalletManagerGive (manager);
        CWMEventRecordingStateFree (&state);
    }

    printf("Testing WKWalletManager sync, disconnect...\n");
    {
        // Test setup
        CWMEventRecordingState state = {0};
        CWMEventRecordingStateNewDefault (&state);

        WKWalletManager manager = wkWalletManagerSetupForLifecycleTest(&state, account, network, mode, scheme, storagePath);
        WKWallet wallet = wkWalletManagerGetWallet (manager);

        // sync, disconnect
        wkWalletManagerSync (manager);
        sleep(1);
        wkWalletManagerDisconnect (manager);
        sleep(1);
        wkWalletManagerStop (manager);

        // Verification
        success = CWMEventRecordingVerifyEventSequence(&state,
                                                       WK_TRUE,
                                                       (CWMEvent []) {
                                                           // wkWalletManagerCreate()
                                                           CWMEventForWalletManagerType         (WK_WALLET_MANAGER_EVENT_CREATED),
                                                           CWMEventForWalletType                (WK_WALLET_EVENT_CREATED),
                                                           CWMEventForWalletManagerWalletType   (WK_WALLET_MANAGER_EVENT_WALLET_ADDED,
                                                                                                     wallet),
                                                       },
                                                       3,
                                                       (CWMEvent []) {
                                                           CWMEventForWalletManagerType (WK_WALLET_MANAGER_EVENT_SYNC_CONTINUES),
                                                           CWMEventForWalletManagerType (WK_WALLET_MANAGER_EVENT_BLOCK_HEIGHT_UPDATED),
                                                       },
                                                       2);
        if (!success) {
            fprintf(stderr, "***FAILED*** %s:%d: BRRunTestWalletManagerSyncTestVerifyEventSequence failed\n", __func__, __LINE__);
            return success;
        }

        success = CWMEventRecordingVerifyEventPairs (&state);
        if (!success) {
            fprintf(stderr, "***FAILED*** %s:%d: BRRunTestWalletManagerSyncTestVerifyEventPairs failed\n", __func__, __LINE__);
            return success;
        }

        // Test teardown
        wkNetworkSetHeight (network, originalNetworkHeight);
        wkWalletGive (wallet);
        wkWalletManagerGive (manager);
        CWMEventRecordingStateFree (&state);
    }

    printf("Testing WKWalletManager threading...\n");
    if (mode == WK_SYNC_MODE_P2P_ONLY && WK_NETWORK_TYPE_BTC == wkNetworkGetType (network)) {
        // TODO(fix): There is a thread-related issue in BRPeerManager/BRPeer where we have a use after free; re-enable once that is fixed
        fprintf(stderr, "***WARNING*** %s:%d: WKWalletManager threading test is disabled for WK_SYNC_MODE_P2P_ONLY and WK_NETWORK_TYPE_BTC\n", __func__, __LINE__);

    } else {
        // Test setup
        CWMEventRecordingState state = {0};
        CWMEventRecordingStateNew (&state, WK_TRUE);

        WKWalletManager manager = wkWalletManagerSetupForLifecycleTest(&state, account, network, mode, scheme, storagePath);
        WKWallet wallet = wkWalletManagerGetWallet (manager);

        // release the hounds
        CWMAbuseThreadState threadState = {0, manager};
        pthread_t connectThread = (pthread_t) NULL, disconnectThread = (pthread_t) NULL, scanThread = (pthread_t) NULL;

        success = (0 == pthread_create (&connectThread, NULL, _CWMAbuseConnectThread, (void*) &threadState) &&
                   0 == pthread_create (&disconnectThread, NULL, _CWMAbuseDisconnectThread, (void*) &threadState) &&
                   0 == pthread_create (&scanThread, NULL, _CWMAbuseSyncThread, (void*) &threadState));
        if (!success) {
            fprintf(stderr, "***FAILED*** %s:%d: pthread_creates failed\n", __func__, __LINE__);
            return success;
        }

        sleep (1);

        threadState.kill = 1;
        success = (0 == pthread_join (connectThread, NULL) &&
                   0 == pthread_join (disconnectThread, NULL) &&
                   0 == pthread_join (scanThread, NULL));
        if (!success) {
            fprintf(stderr, "***FAILED*** %s:%d: pthread_joins failed\n", __func__, __LINE__);
            return success;
        }

        wkWalletManagerStop (manager);

        // Verification
        success = CWMEventRecordingVerifyEventSequence(&state,
                                                       WK_FALSE,
                                                       (CWMEvent []) {
                                                           // wkWalletManagerCreate()
                                                           CWMEventForWalletManagerType         (WK_WALLET_MANAGER_EVENT_CREATED),
                                                           CWMEventForWalletType                (WK_WALLET_EVENT_CREATED),
                                                           CWMEventForWalletManagerWalletType   (WK_WALLET_MANAGER_EVENT_WALLET_ADDED,
                                                                                                     wallet),
                                                       },
                                                       3,
                                                       (CWMEvent []) {
                                                           CWMEventForWalletManagerType (WK_WALLET_MANAGER_EVENT_SYNC_CONTINUES),
                                                           CWMEventForWalletManagerType (WK_WALLET_MANAGER_EVENT_BLOCK_HEIGHT_UPDATED),
                                                       },
                                                       2);
        if (!success) {
            fprintf(stderr, "***FAILED*** %s:%d: BRRunTestWalletManagerSyncTestVerifyEventSequence failed\n", __func__, __LINE__);
            return success;
        }

        // Test teardown
        wkNetworkSetHeight (network, originalNetworkHeight);
        wkWalletGive (wallet);
        wkWalletManagerGive (manager);
        CWMEventRecordingStateFree (&state);
    }

    return success;
}

static int
runCryptoWalletManagerLifecycleWithSetModeTest (WKAccount account,
                                                WKNetwork network,
                                                WKSyncMode primaryMode,
                                                WKSyncMode secondaryMode,
                                                WKAddressScheme scheme,
                                                const char *storagePath) {
    int success = 1;

    // HACK: Managers set the height; we need to be able to restore it between tests
    WKBlockNumber originalNetworkHeight = wkNetworkGetHeight (network);

    printf("Testing WKWalletManager events for mode=\"%s/%s\", network=\"%s (%s)\" and path=\"%s\"...\n",
           wkSyncModeString (primaryMode),
           wkSyncModeString (secondaryMode),
           wkNetworkGetName (network),
           wkNetworkIsMainnet (network) ? "mainnet" : "testnet",
           storagePath);

   printf("Testing WKWalletManager mode swap while disconnected...\n");
   {
       // Test setup
       CWMEventRecordingState state = {0};
       CWMEventRecordingStateNewDefault (&state);

       WKWalletManager manager = wkWalletManagerSetupForLifecycleTest(&state, account, network, primaryMode, scheme, storagePath);
       WKWallet wallet = wkWalletManagerGetWallet (manager);

        // swap modes while disconnected
        wkWalletManagerSetMode (manager, secondaryMode);
        success = wkWalletManagerGetMode (manager) == secondaryMode;
        if (!success) {
            fprintf(stderr, "***FAILED*** %s:%d: BRWalletManagerSetMode failed\n", __func__, __LINE__);
            return success;
        }
        sleep (1);

        wkWalletManagerStop (manager);

       // Verification
        success = CWMEventRecordingVerifyEventSequence(&state,
                                                       WK_TRUE,
                                                       (CWMEvent []) {
                                                           // wkWalletManagerCreate()
                                                           CWMEventForWalletManagerType         (WK_WALLET_MANAGER_EVENT_CREATED),
                                                           CWMEventForWalletType                (WK_WALLET_EVENT_CREATED),
                                                           CWMEventForWalletManagerWalletType   (WK_WALLET_MANAGER_EVENT_WALLET_ADDED,
                                                                                                     wallet),
                                                       },
                                                       3,
                                                       (CWMEvent []) {
                                                           CWMEventForWalletManagerType (WK_WALLET_MANAGER_EVENT_SYNC_CONTINUES),
                                                           CWMEventForWalletManagerType (WK_WALLET_MANAGER_EVENT_BLOCK_HEIGHT_UPDATED),
                                                       },
                                                       2);
       if (!success) {
           fprintf(stderr, "***FAILED*** %s:%d: BRRunTestWalletManagerSyncTestVerifyEventSequence failed\n", __func__, __LINE__);
           return success;
       }

       success = CWMEventRecordingVerifyEventPairs (&state);
       if (!success) {
           fprintf(stderr, "***FAILED*** %s:%d: BRRunTestWalletManagerSyncTestVerifyEventPairs failed\n", __func__, __LINE__);
           return success;
       }

       // Test teardown
       wkNetworkSetHeight (network, originalNetworkHeight);
       wkWalletGive (wallet);
       wkWalletManagerGive (manager);
       CWMEventRecordingStateFree (&state);
   }

   printf("Testing WKWalletManager mode swap while connected...\n");
   {
       // Test setup
       CWMEventRecordingState state = {0};
       CWMEventRecordingStateNewDefault (&state);

       WKWalletManager manager = wkWalletManagerSetupForLifecycleTest(&state, account, network, primaryMode, scheme, storagePath);
       WKWallet wallet = wkWalletManagerGetWallet (manager);

        // swap modes while connected
        wkWalletManagerConnect (manager, NULL);
        sleep(1);

        wkWalletManagerSetMode (manager, secondaryMode);
        success = wkWalletManagerGetMode (manager) == secondaryMode;
        if (!success) {
            fprintf(stderr, "***FAILED*** %s:%d: BRWalletManagerSetMode failed\n", __func__, __LINE__);
            return success;
        }
        sleep (1);

        wkWalletManagerStop (manager);

       // Verification
       success = CWMEventRecordingVerifyEventSequence(&state,
                                                      WK_TRUE,
                                                      (CWMEvent []) {
                                                          // wkWalletManagerCreate()
                                                          CWMEventForWalletManagerType         (WK_WALLET_MANAGER_EVENT_CREATED),
                                                          CWMEventForWalletType                (WK_WALLET_EVENT_CREATED),
                                                          CWMEventForWalletManagerWalletType   (WK_WALLET_MANAGER_EVENT_WALLET_ADDED,
                                                                                                    wallet),
                                                           // wkWalletManagerConnect()
                                                          CWMEventForWalletManagerStateType    (WK_WALLET_MANAGER_EVENT_CHANGED,
                                                                                                    wkWalletManagerStateInit (WK_WALLET_MANAGER_STATE_CREATED),
                                                                                                    wkWalletManagerStateInit (WK_WALLET_MANAGER_STATE_CONNECTED)),
                                                          CWMEventForWalletManagerType         (WK_WALLET_MANAGER_EVENT_SYNC_STARTED),
                                                          CWMEventForWalletManagerStateType    (WK_WALLET_MANAGER_EVENT_CHANGED,
                                                                                                    wkWalletManagerStateInit (WK_WALLET_MANAGER_STATE_CONNECTED),
                                                                                                    wkWalletManagerStateInit (WK_WALLET_MANAGER_STATE_SYNCING)),
                                                           // wkWalletManagerSetMode()
                                                          CWMEventForWalletManagerType         (WK_WALLET_MANAGER_EVENT_SYNC_STOPPED),
                                                          CWMEventForWalletManagerStateType    (WK_WALLET_MANAGER_EVENT_CHANGED,
                                                                                                    wkWalletManagerStateInit (WK_WALLET_MANAGER_STATE_SYNCING),
                                                                                                    wkWalletManagerStateInit (WK_WALLET_MANAGER_STATE_CONNECTED)),
                                                          CWMEventForWalletManagerStateType    (WK_WALLET_MANAGER_EVENT_CHANGED,
                                                                                                    wkWalletManagerStateInit (WK_WALLET_MANAGER_STATE_CONNECTED),
                                                                                                    wkWalletManagerStateDisconnectedInit (wkWalletManagerDisconnectReasonUnknown())),
                                                      },
                                                      9,
                                                      (CWMEvent []) {
                                                          CWMEventForWalletManagerType (WK_WALLET_MANAGER_EVENT_SYNC_CONTINUES),
                                                          CWMEventForWalletManagerType (WK_WALLET_MANAGER_EVENT_BLOCK_HEIGHT_UPDATED),
                                                      },
                                                      2);
       if (!success) {
           fprintf(stderr, "***FAILED*** %s:%d: BRRunTestWalletManagerSyncTestVerifyEventSequence failed\n", __func__, __LINE__);
           return success;
       }

       success = CWMEventRecordingVerifyEventPairs (&state);
       if (!success) {
           fprintf(stderr, "***FAILED*** %s:%d: BRRunTestWalletManagerSyncTestVerifyEventPairs failed\n", __func__, __LINE__);
           return success;
       }

       // Test teardown
       wkNetworkSetHeight (network, originalNetworkHeight);
       wkWalletGive (wallet);
       wkWalletManagerGive (manager);
       CWMEventRecordingStateFree (&state);
   }

    printf("Testing WKWalletManager mode swap threading...\n");
    if (WK_NETWORK_TYPE_BTC == wkNetworkGetType (network) &&
        (primaryMode == WK_SYNC_MODE_P2P_ONLY || secondaryMode == WK_SYNC_MODE_P2P_ONLY)) {
        // TODO(fix): There is a thread-related issue in BRPeerManager/BRPeer where we have a use after free; re-enable once that is fixed
        fprintf(stderr, "***WARNING*** %s:%d: WKWalletManager threading test is disabled for WK_SYNC_MODE_P2P_ONLY and WK_NETWORK_TYPE_BTC\n", __func__, __LINE__);

    } else {
        // Test setup
        CWMEventRecordingState state = {0};
        CWMEventRecordingStateNew (&state, WK_TRUE);

        WKWalletManager manager = wkWalletManagerSetupForLifecycleTest(&state, account, network, primaryMode, scheme, storagePath);
        WKWallet wallet = wkWalletManagerGetWallet (manager);

        // release the hounds
        CWMAbuseThreadState threadState = {0, manager, primaryMode, secondaryMode};
        pthread_t connectThread = (pthread_t) NULL, disconnectThread = (pthread_t) NULL, scanThread = (pthread_t) NULL, swapThread = (pthread_t) NULL;

        success = (0 == pthread_create (&connectThread, NULL, _CWMAbuseConnectThread, (void*) &threadState) &&
                   0 == pthread_create (&disconnectThread, NULL, _CWMAbuseDisconnectThread, (void*) &threadState) &&
                   0 == pthread_create (&scanThread, NULL, _CWMAbuseSyncThread, (void*) &threadState) &&
                   0 == pthread_create (&swapThread, NULL, _CWMAbuseSwapThread, (void*) &threadState));
        if (!success) {
            fprintf(stderr, "***FAILED*** %s:%d: pthread_creates failed\n", __func__, __LINE__);
            return success;
        }

        sleep (1);

        threadState.kill = 1;
        success = (0 == pthread_join (connectThread, NULL) &&
                   0 == pthread_join (disconnectThread, NULL) &&
                   0 == pthread_join (scanThread, NULL) &&
                   0 == pthread_join (swapThread, NULL));
        if (!success) {
            fprintf(stderr, "***FAILED*** %s:%d: pthread_joins failed\n", __func__, __LINE__);
            return success;
        }

        wkWalletManagerStop (manager);

        // Verification
        success = CWMEventRecordingVerifyEventSequence(&state,
                                                       WK_FALSE,
                                                       (CWMEvent []) {
                                                           // wkWalletManagerCreate()
                                                           CWMEventForWalletManagerType         (WK_WALLET_MANAGER_EVENT_CREATED),
                                                           CWMEventForWalletType                (WK_WALLET_EVENT_CREATED),
                                                           CWMEventForWalletManagerWalletType   (WK_WALLET_MANAGER_EVENT_WALLET_ADDED,
                                                                                                     wallet),
                                                       },
                                                       3,
                                                       (CWMEvent []) {
                                                           CWMEventForWalletManagerType (WK_WALLET_MANAGER_EVENT_SYNC_CONTINUES),
                                                           CWMEventForWalletManagerType (WK_WALLET_MANAGER_EVENT_BLOCK_HEIGHT_UPDATED),
                                                       },
                                                       2);
        if (!success) {
            fprintf(stderr, "***FAILED*** %s:%d: BRRunTestWalletManagerSyncTestVerifyEventSequence failed\n", __func__, __LINE__);
            return success;
        }

        // Test teardown
        wkNetworkSetHeight (network, originalNetworkHeight);
        wkWalletGive (wallet);
        wkWalletManagerGive (manager);
        CWMEventRecordingStateFree (&state);
    }

    return success;
}

///
/// Mark: Entrypoints
///

extern WKBoolean
runCryptoTestsWithAccountAndNetwork (WKAccount account,
                                     WKNetwork network,
                                     const char *storagePath) {
    WKBoolean success = WK_TRUE;

    WKNetworkType chainType = wkNetworkGetType (network);

 //   WKBoolean isGen = AS_WK_BOOLEAN (WK_NETWORK_TYPE_GEN == chainType);
    WKBoolean isEth = AS_WK_BOOLEAN (WK_NETWORK_TYPE_ETH == chainType);
    WKBoolean isBtc = AS_WK_BOOLEAN (WK_NETWORK_TYPE_BTC == chainType);
    WKBoolean isBch = AS_WK_BOOLEAN (WK_NETWORK_TYPE_BTC == chainType);
    WKBoolean isBsv = AS_WK_BOOLEAN (WK_NETWORK_TYPE_BSV == chainType);
    WKBoolean isGen = (!isEth && !isBch && !isBch && !isBsv);

    WKAddressScheme scheme = ((isBtc || isBch) ?
                                    WK_ADDRESS_SCHEME_BTC_LEGACY :
                                    WK_ADDRESS_SCHEME_NATIVE);

    if (isBtc || isEth || isGen) {
        success = AS_WK_BOOLEAN(runCryptoWalletManagerLifecycleTest (account,
                                                                         network,
                                                                         WK_SYNC_MODE_API_ONLY,
                                                                         scheme,
                                                                         storagePath));
        if (!success) {
            fprintf(stderr, "***FAILED*** %s:%d: failed\n", __func__, __LINE__);
            return success;
        }
    }

    if (isBtc || isBch || isBsv || isEth) {
        success = AS_WK_BOOLEAN(runCryptoWalletManagerLifecycleTest (account,
                                                                         network,
                                                                         WK_SYNC_MODE_P2P_ONLY,
                                                                         scheme,
                                                                         storagePath));
        if (!success) {
            fprintf(stderr, "***FAILED*** %s:%d: failed\n", __func__, __LINE__);
            return success;
        }
    }

    if (isEth) {
        success = AS_WK_BOOLEAN(runCryptoWalletManagerLifecycleTest (account,
                                                                         network,
                                                                         WK_SYNC_MODE_API_WITH_P2P_SEND,
                                                                         scheme,
                                                                         storagePath));
        if (!success) {
            fprintf(stderr, "***FAILED*** %s:%d: failed\n", __func__, __LINE__);
            return success;
        }
    }

    if (isBtc) {
        success = AS_WK_BOOLEAN(runCryptoWalletManagerLifecycleWithSetModeTest (account,
                                                                                    network,
                                                                                    WK_SYNC_MODE_P2P_ONLY,
                                                                                    WK_SYNC_MODE_API_ONLY,
                                                                                    scheme,
                                                                                    storagePath));
        if (!success) {
            fprintf(stderr, "***FAILED*** %s:%d: failed\n", __func__, __LINE__);
            return success;
        }

        success = AS_WK_BOOLEAN(runCryptoWalletManagerLifecycleWithSetModeTest (account,
                                                                                    network,
                                                                                    WK_SYNC_MODE_API_ONLY,
                                                                                    WK_SYNC_MODE_P2P_ONLY,
                                                                                    scheme,
                                                                                    storagePath));
        if (!success) {
            fprintf(stderr, "***FAILED*** %s:%d: failed\n", __func__, __LINE__);
            return success;
        }
    }

    if (isEth) {
        success = AS_WK_BOOLEAN(runCryptoWalletManagerLifecycleWithSetModeTest (account,
                                                                                    network,
                                                                                    WK_SYNC_MODE_P2P_ONLY,
                                                                                    WK_SYNC_MODE_API_WITH_P2P_SEND,
                                                                                    scheme,
                                                                                    storagePath));
        if (!success) {
            fprintf(stderr, "***FAILED*** %s:%d: failed\n", __func__, __LINE__);
            return success;
        }

        success = AS_WK_BOOLEAN(runCryptoWalletManagerLifecycleWithSetModeTest (account,
                                                                                    network,
                                                                                    WK_SYNC_MODE_API_WITH_P2P_SEND,
                                                                                    WK_SYNC_MODE_P2P_ONLY,
                                                                                    scheme,
                                                                                    storagePath));
        if (!success) {
            fprintf(stderr, "***FAILED*** %s:%d: failed\n", __func__, __LINE__);
            return success;
        }
    }

    return success;
}

extern void
runCryptoTests (void) {
    runCryptoAmountTests ();
    runCryptoTransferTests();
    return;
}
