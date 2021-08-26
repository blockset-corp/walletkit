//
//  WKBase.h
//  WalletKitCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKBase_h
#define WKBase_h

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stdatomic.h>
#include <memory.h>
#include <assert.h>

//
// An OwnershipKept annotation on a function's arguments implies that ownership is not passed into
// the function; instead the caller continues having ownership.  If the return value is annotated
// with OwnershipKept then the caller, receiving the return value, does not own the value and must
// take or copy if held beyond the caller's scope
//
// OwnershipKept is the default for any parameters that are not annotated.
//
#if !defined (OwnershipKept)
#define OwnershipKept
#endif

//
// An Ownership annotation on a function's arguments implies that ownership is passed into the
// function; the body of the function takes ownership and must release/free the objects - either
// directly or by passing to another function declaring the argument as OwnershipGiven.  If the
// return value is annotaed with OwnershipGiven then the caller, receiving the return value, owns
// the object and must dispose of the object.
#if !defined (OwnershipGiven)
#define OwnershipGiven
#endif

//
// A Nullable annotation declares that an argument or return value can be NULL
//
#if !defined (Nullable)
#define Nullable
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern const char* wkVersion;

// Forward Declarations - Required for WKListener

/**
 * @brief A Transfer is an exchange of an asset.  A Transfer occurs between 'source' and
 * 'target' addresses with an 'amount' in a specified currency.  Note: one creates a transfer
 * with a 'target' address; the 'source' address is derived from the User's account.
 *
 * @discussion A Transfer send by a User results in the submission of a Transaction to a
 * blockchain.  The structure of the Transaction is deteremined from the Network.  Once the
 * blockchain has included the Transaction in a block, then the Transfer's state will indicate
 * the result - generally 'INCLUDED' but perhaps 'ERRORED'.
 */
typedef struct WKTransferRecord      *WKTransfer;

/**
 * @brief A Wallet holds transfers and maintains a balance for a specified currency.
 */
typedef struct WKWalletRecord        *WKWallet;        // WK{Transfer,Payment}

/**
 * @brief A WalletManager managers one or more wallets on network.
 *
 * @discussion A WalletManger is an 'active object' that runs certain tasks asynchornously;
 * those tasks ensure that all transactions for the User's account are identified, added to
 * their corresponding wallet, and attributed to the wallet's balance.  The WalletManager
 * runs these tasks when 'connected'; the tasks are run continuously so that newly included
 * blockchain blocks can be searched for account transactions.
 */
typedef struct WKWalletManagerRecord *WKWalletManager; // WK{Wallet,Transfer,Payment}

/**
 * @brief A Network represents a cryptocurrency blockchain.  WalletKit supports a defined set of
 * Networks - some of which are for mainnet and some for testnet.
 *
 * @discussion
 *
 * A Network has a currency which represents the asset used to pay for network fees.  For
 * Bitcoin the currency is 'Bitcoin'; for Ethereum the currency is 'Ethereum'.
 *
 * A Network may support more than one currency.  For Ethereum additional currencies include
 * the ERC20 Smart Contracts of interest - for example, BRD.
 *
 * Every Network's currency has a defined base Unit, default Unit and an arbitrary set of other
 * units.  For Ethereum there are: WEI, ETHER, [WEI, GWEI, ..., ETHER, ...] respectively.
 */
typedef struct WKNetworkRecord       *WKNetwork;

/**
 * @brief A System represents a User's access to the supported Networks that provide data
 * for cryptocurrency blockchains.
 *
 * @discussion
 *
 * A System is an 'active object' - it runs asynchronously in one or more threads.
 *
 * A System 'runs' on zero or more Networks.
 *
 * A System 'manages' zero or more WalletManagers, one for each Network of interest.
 *
 * A System is defined for one-and-only-one Account.  As each system is independent certain
 * applications may chose to create multiple system is handling multiple accounts.
 *
 * The System must provide a Client, which supports system queries for blockchain data.  The
 * client implements a defined set of callbacks; these callbacks may be called asynchronously
 * and generally should not block their calling thread.
 *
 * The System must provide a Listener, which defines a set of callbacks to announce changes to
 * system state.
 */
typedef struct WKSystemRecord        *WKSystem;        // WK{Manager,Wallet,Transfer,Payment}

/**
 * @brief A Listener listens in on System, Network, WalletManager, Wallet and Transfers events.
 * This is the primary mechanism by which users of WalletKit are informed about state changes
 * in their created System.
 *
 * @discussion A Listener is created by providing a set of callback functions.  The callback
 * functions, for System, Network, etc, 'announce' state changes and would, for example,
 * implement UI updates based on the callback.
 */
typedef struct WKListenerRecord      *WKListener;

/**
 * @brief A Listener Context is an arbitrary pointer that is provided when a Listener is created
 * and that is passed back in all listener calls.  This allows the listener to establish the
 * context for handling the call.
 */
typedef void  *WKListenerContext;

/**
 * @brief A Cookie is used as a marker to match up an asynchronou operation with a handler
 * for the result of the operation
 */
typedef void *WKCookie;

// End Forward Declarations

/** An explice boolean type */
typedef enum {
    WK_FALSE = 0,
    WK_TRUE  = 1
} WKBoolean;

#define AS_WK_BOOLEAN(zeroIfFalse)   ((zeroIfFalse) ? WK_TRUE : WK_FALSE)

// For use in Swift/Java
typedef size_t WKCount;

// For use in Swift/Java
static inline void
wkMemoryFree (void *memory) {
    if (NULL != memory) free (memory);
}

// For use in Java (needing an 'extern' for a 'native' declaration)
extern void wkMemoryFreeExtern (void *memory);

// Same as: BRBlockHeight
typedef uint64_t WKBlockNumber;
#if !defined(BLOCK_HEIGHT_UNBOUND)
// See BRBase.h
#define BLOCK_HEIGHT_UNBOUND       (UINT64_MAX)
#endif
extern uint64_t BLOCK_HEIGHT_UNBOUND_VALUE;

#define BLOCK_NUMBER_UNKNOWN        (BLOCK_HEIGHT_UNBOUND)

/// The number of seconds since the Unix epoch).
typedef uint64_t WKTimestamp;

#define AS_WK_TIMESTAMP(unixSeconds)      ((WKTimestamp) (unixSeconds))
#define NO_WK_TIMESTAMP                   (AS_WK_TIMESTAMP (0))


/// MARK: - Data32 / Data16

typedef struct {
    uint8_t data[256/8];
} WKData32;

static inline void wkData32Clear (WKData32 *data32) {
    memset (data32, 0, sizeof (WKData32));
}

typedef struct {
    uint8_t data[128/8];
} WKData16;

static inline void wkData16Clear (WKData16 *data16) {
    memset (data16, 0, sizeof (WKData16));
}

/// MARK: - Variable Size Data

typedef struct {
    uint8_t * bytes;
    size_t size;
} WKData;

static inline WKData wkDataNew (size_t size) {
    WKData data;
    data.size = size;
    if (size < 1) data.size = 1;
    data.bytes = calloc (data.size, sizeof(uint8_t));
    assert (data.bytes != NULL);
    return data;
}

static inline WKData wkDataCreate (uint8_t *bytes, size_t bytesCount) {
    WKData data = wkDataNew (bytesCount);
    memcpy (data.bytes, bytes, bytesCount);
    return data;
}

static inline WKData wkDataCreateEmpty (void) {
    return (WKData) { NULL, 0};
}

static inline WKData wkDataCopy (uint8_t * bytes, size_t size) {
    WKData data = { NULL, 0 };

    if (NULL != bytes && 0 != size) {
        data.bytes = malloc (size * sizeof(uint8_t));
        memcpy (data.bytes, bytes, size);
        data.size = size;
    }
    
    return data;
}

static inline WKData wkDataClone (WKData data) {
    return wkDataCopy (data.bytes, data.size);
}

static inline WKData
wkDataConcat (WKData * fields, size_t numFields) {
    size_t totalSize = 0;
    for (int i=0; i < numFields; i++) {
        totalSize += fields[i].size;
    }
    WKData concat = wkDataNew (totalSize);
    totalSize = 0;
    for (int i=0; i < numFields; i++) {
        memcpy (&concat.bytes[totalSize], fields[i].bytes, fields[i].size);
        totalSize += fields[i].size;
    }
    return concat;
}

static inline WKData
wkDataConcatTwo (WKData data1, WKData data2) {
    WKData data[2] = { data1, data2 };
    return wkDataConcat (data, 2);
}

static inline void wkDataFree (WKData data) {
    if (data.bytes) free(data.bytes);
    data.bytes = NULL;
    data.size = 0;
}

/// MARK: Network Canonical Type

///
/// Crypto Network Type
///
/// Try as we might, there are certain circumstances where the type of the network needs to
/// be known.  Without this enumeration, one uses hack-arounds like:
///    "btc" == network.currency.code
/// So, provide these and expect them to grow.
///
/// Enumerations here need to be consistent with the networks defined in;
///    walletkit/WKConfig.h
///
typedef enum {
    WK_NETWORK_TYPE_BTC,
    WK_NETWORK_TYPE_BCH,
    WK_NETWORK_TYPE_BSV,
    WK_NETWORK_TYPE_LTC,
    WK_NETWORK_TYPE_DOGE,
    WK_NETWORK_TYPE_ETH,
    WK_NETWORK_TYPE_XRP,
    WK_NETWORK_TYPE_HBAR,
    WK_NETWORK_TYPE_XTZ,
    WK_NETWORK_TYPE_XLM,
    /* WK_NETWORK_TYPE___SYMBOL__ */
} WKNetworkType;

#define WK_NETWORK_TYPE_LAST        WK_NETWORK_TYPE_XLM
#define NUMBER_OF_NETWORK_TYPES     (1 + WK_NETWORK_TYPE_LAST)
#define WK_NETWORK_TYPE_UNKNOWN (UINT32_MAX)
//
// Crypto Network Base Currency
//
// These are the 'currency codes' used for DEFINE_CURRENCY in crypto/WKConfig.h.  Any
// time we need 'type -> string' we'll use these in wkNetworkCanonicalTypeGetCurrencyCode()
//
#define WK_NETWORK_CURRENCY_BTC     "btc"
#define WK_NETWORK_CURRENCY_BCH     "bch"
#define WK_NETWORK_CURRENCY_BSV     "bsv"
#define WK_NETWORK_CURRENCY_LTC     "ltc"
#define WK_NETWORK_CURRENCY_DOGE    "doge"
#define WK_NETWORK_CURRENCY_ETH     "eth"
#define WK_NETWORK_CURRENCY_XRP     "xrp"
#define WK_NETWORK_CURRENCY_HBAR    "hbar"
#define WK_NETWORK_CURRENCY_XTZ     "xtz"
#define WK_NETWORK_CURRENCY_XLM     "xlm"
/* #define WK_NETWORK_CURRENCY___SYMBOL__    "__symbol__" */

extern const char *
wkNetworkTypeGetCurrencyCode (WKNetworkType type);

// MARK: - Status

typedef enum {
    WK_SUCCESS = 0,
    // Generic catch-all failure. This should only be used as if creating a
    // specific error code does not make sense (you really should create
    // a specifc error code...).
    WK_ERROR_FAILED,
    
    // Reference access
    WK_ERROR_UNKNOWN_NODE = 10000,
    WK_ERROR_UNKNOWN_TRANSFER,
    WK_ERROR_UNKNOWN_ACCOUNT,
    WK_ERROR_UNKNOWN_WALLET,
    WK_ERROR_UNKNOWN_BLOCK,
    WK_ERROR_UNKNOWN_LISTENER,
    
    // Node
    WK_ERROR_NODE_NOT_CONNECTED = 20000,
    
    // Transfer
    WK_ERROR_TRANSFER_HASH_MISMATCH = 30000,
    WK_ERROR_TRANSFER_SUBMISSION,
    
    // Numeric
    WK_ERROR_NUMERIC_PARSE = 40000,
    
    // Acount
    // Wallet
    // Block
    // Listener
} WKStatus;

/// MARK: - Reference Counting

typedef struct {
    _Atomic(unsigned int) count;
    void (*free) (void *);
} WKRef;

#if defined (WK_REF_DEBUG)
#include <stdio.h>
static int wkRefDebug = 1;
#define wkRefShow   printf
#else
static int wkRefDebug = 0;
#define wkRefShow 
#endif

#define DECLARE_WK_GIVE_TAKE(type, preface)                                   \
  extern type preface##Take (type obj);                                           \
  extern type preface##TakeWeak (type obj);                                       \
  extern void preface##Give (type obj)

#define IMPLEMENT_WK_GIVE_TAKE(type, preface)                                 \
  static void preface##Release (type obj);                                        \
  extern type                                                                     \
  preface##Take (type obj) {                                                      \
    if (NULL == obj) return NULL;                                                 \
    unsigned int _c = atomic_fetch_add (&obj->ref.count, 1);                      \
    /* catch take after release */                                                \
    assert (0 != _c);                                                             \
    return obj;                                                                   \
  }                                                                               \
  extern type                                                                     \
  preface##TakeWeak (type obj) {                                                  \
    if (NULL == obj) return NULL;                                                 \
    unsigned int _c = atomic_load(&obj->ref.count);                               \
    /* keep trying to take unless object is released */                           \
    while (_c != 0 &&                                                             \
           !atomic_compare_exchange_weak (&obj->ref.count, &_c, _c + 1)) {}       \
    if (wkRefDebug && 0 == _c) { wkRefShow ("CRY: Missed: %s\n", #type); }\
    return (_c != 0) ? obj : NULL;                                                \
  }                                                                               \
  extern void                                                                     \
  preface##Give (type obj) {                                                      \
    if (NULL == obj) return;                                                      \
    unsigned int _c = atomic_fetch_sub (&obj->ref.count, 1);                      \
    /* catch give after release */                                                \
    assert (0 != _c);                                                             \
    if (1 == _c) {                                                                \
        if (wkRefDebug) { wkRefShow ("CRY: Release: %s\n", #type); }      \
        obj->ref.free (obj);                                                      \
    }                                                                             \
}

#define WK_AS_FREE(release)     ((void (*) (void *)) release)

#define WK_REF_ASSIGN(release)  (WKRef) { 1, WK_AS_FREE (release) }

#if !defined (private_extern)
#  define private_extern          extern
#endif

#if !defined(ASSERT_UNIMPLEMENTED)
#define ASSERT_UNIMPLEMENTED    assert(false);
#endif

#ifdef __cplusplus
}
#endif

#endif /* WKBase_h */
