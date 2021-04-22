//
//  BRCryptoBase.h
//  WalletKitCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoBase_h
#define BRCryptoBase_h

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stdatomic.h>
#include <memory.h>
#include <assert.h>

// temporary

#if !defined (OwnershipGiven)
#define OwnershipGiven
#endif

#if !defined (OwnershipKept)
#define OwnershipKept
#endif

#if !defined (Nullable)
#define Nullable
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern const char* cryptoVersion;

// Forward Declarations - Required for BRCryptoListener

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
typedef struct BRCryptoTransferRecord      *BRCryptoTransfer;

/**
 * @brief A Wallet holds transfers and maintains a balance for a specified currency.
 */
typedef struct BRCryptoWalletRecord        *BRCryptoWallet;        // BRCrypto{Transfer,Payment}

/**
 * @brief A WalletManager managers one or more wallets on network.
 *
 * @discussion A WalletManger is an 'active object' that runs certain tasks asynchornously;
 * those tasks ensure that all transactions for the User's account are identified, added to
 * their corresponding wallet, and attributed to the wallet's balance.  The WalletManager
 * runs these tasks when 'connected'; the tasks are run continuously so that newly included
 * blockchain blocks can be searched for account transactions.
 */
typedef struct BRCryptoWalletManagerRecord *BRCryptoWalletManager; // BRCrypto{Wallet,Transfer,Payment}

/**
 * @brief A Network represents a cryptographic blockchain.  WalletKit supports a defined set of
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
typedef struct BRCryptoNetworkRecord       *BRCryptoNetwork;

/**
 * @brief A System represents a User's access to the supported Networks that provide data
 * for cryptographic blockchains.
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
typedef struct BRCryptoSystemRecord        *BRCryptoSystem;        // BRCrypto{Manager,Wallet,Transfer,Payment}

/**
 * @brief A Listener listens in on System, Network, WalletManager, Wallet and Transfers events.
 * This is the primary mechanism by which users of WalletKit are informed about state changes
 * in their created System.
 *
 * @discussion A Listener is created by providing a set of callback functions.  The callback
 * functions, for System, Network, etc, 'announce' state changes and would, for example,
 * implement UI updates based on the callback.
 */
typedef struct BRCryptoListenerRecord      *BRCryptoListener;

/**
 * @brief A Listener Context is an arbitrary pointer that is provided when a Listener is created
 * and that is passed back in all listener calls.  This allows the listener to establish the
 * context for handling the call.
 */
typedef void  *BRCryptoListenerContext;

/**
 * @brief A Cookie is used as a marker to match up an asynchronou operation with a handler
 * for the result of the operation
 */
typedef void *BRCryptoCookie;

// End Forward Declarations

/** An explice boolean type */
typedef enum {
    CRYPTO_FALSE = 0,
    CRYPTO_TRUE  = 1
} BRCryptoBoolean;

#define AS_CRYPTO_BOOLEAN(zeroIfFalse)   ((zeroIfFalse) ? CRYPTO_TRUE : CRYPTO_FALSE)

// For use in Swift/Java
typedef size_t BRCryptoCount;

// For use in Swift/Java
static inline void
cryptoMemoryFree (void *memory) {
    if (NULL != memory) free (memory);
}

// For use in Java (needing an 'extern' for a 'native' declaration)
extern void cryptoMemoryFreeExtern (void *memory);

// Same as: BRBlockHeight
typedef uint64_t BRCryptoBlockNumber;
#if !defined(BLOCK_HEIGHT_UNBOUND)
// See BRBase.h
#define BLOCK_HEIGHT_UNBOUND       (UINT64_MAX)
#endif
extern uint64_t BLOCK_HEIGHT_UNBOUND_VALUE;

#define BLOCK_NUMBER_UNKNOWN        (BLOCK_HEIGHT_UNBOUND)

/// The number of seconds since the Unix epoch).
typedef uint64_t BRCryptoTimestamp;

#define AS_CRYPTO_TIMESTAMP(unixSeconds)      ((BRCryptoTimestamp) (unixSeconds))
#define NO_CRYPTO_TIMESTAMP                   (AS_CRYPTO_TIMESTAMP (0))


/// MARK: - Data32 / Data16

typedef struct {
    uint8_t data[256/8];
} BRCryptoData32;

static inline void cryptoData32Clear (BRCryptoData32 *data32) {
    memset (data32, 0, sizeof (BRCryptoData32));
}

typedef struct {
    uint8_t data[128/8];
} BRCryptoData16;

static inline void cryptoData16Clear (BRCryptoData16 *data16) {
    memset (data16, 0, sizeof (BRCryptoData16));
}

/// MARK: - Variable Size Data

typedef struct {
    uint8_t * bytes;
    size_t size;
} BRCryptoData;

static inline BRCryptoData cryptoDataNew (size_t size) {
    BRCryptoData data;
    data.size = size;
    if (size < 1) data.size = 1;
    data.bytes = calloc (data.size, sizeof(uint8_t));
    assert (data.bytes != NULL);
    return data;
}

static inline BRCryptoData cryptoDataCopy (uint8_t * bytes, size_t size) {
    BRCryptoData data;
    data.bytes = malloc (size * sizeof(uint8_t));
    memcpy (data.bytes, bytes, size);
    data.size = size;
    return data;
}

static inline BRCryptoData
cryptoDataConcat (BRCryptoData * fields, size_t numFields) {
    size_t totalSize = 0;
    for (int i=0; i < numFields; i++) {
        totalSize += fields[i].size;
    }
    BRCryptoData concat = cryptoDataNew (totalSize);
    totalSize = 0;
    for (int i=0; i < numFields; i++) {
        memcpy (&concat.bytes[totalSize], fields[i].bytes, fields[i].size);
        totalSize += fields[i].size;
    }
    return concat;
}

static inline void cryptoDataFree (BRCryptoData data) {
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
///    crypto/BRCryptoConfig.h
///
typedef enum {
    CRYPTO_NETWORK_TYPE_BTC,
    CRYPTO_NETWORK_TYPE_BCH,
    CRYPTO_NETWORK_TYPE_BSV,
    CRYPTO_NETWORK_TYPE_ETH,
    CRYPTO_NETWORK_TYPE_XRP,
    CRYPTO_NETWORK_TYPE_HBAR,
    CRYPTO_NETWORK_TYPE_XTZ,
    // CRYPTO_NETWORK_TYPE_XLM,
} BRCryptoNetworkType;

#define NUMBER_OF_NETWORK_TYPES     (1 + CRYPTO_NETWORK_TYPE_XTZ)
#define CRYPTO_NETWORK_TYPE_UNKNOWN (UINT32_MAX)
//
// Crypto Network Base Currency
//
// These are the 'currency codes' used for DEFINE_CURRENCY in crypto/BRCryptoConfig.h.  Any
// time we need 'type -> string' we'll use these in cryptoNetworkCanonicalTypeGetCurrencyCode()
//
#define CRYPTO_NETWORK_CURRENCY_BTC     "btc"
#define CRYPTO_NETWORK_CURRENCY_BCH     "bch"
#define CRYPTO_NETWORK_CURRENCY_BSV     "bsv"
#define CRYPTO_NETWORK_CURRENCY_ETH     "eth"
#define CRYPTO_NETWORK_CURRENCY_XRP     "xrp"
#define CRYPTO_NETWORK_CURRENCY_HBAR    "hbar"
#define CRYPTO_NETWORK_CURRENCY_XTZ     "xtz"

extern const char *
cryptoBlockChainTypeGetCurrencyCode (BRCryptoNetworkType type);

// MARK: - Status

typedef enum {
    CRYPTO_SUCCESS = 0,
    // Generic catch-all failure. This should only be used as if creating a
    // specific error code does not make sense (you really should create
    // a specifc error code...).
    CRYPTO_ERROR_FAILED,

    // Reference access
    CRYPTO_ERROR_UNKNOWN_NODE = 10000,
    CRYPTO_ERROR_UNKNOWN_TRANSFER,
    CRYPTO_ERROR_UNKNOWN_ACCOUNT,
    CRYPTO_ERROR_UNKNOWN_WALLET,
    CRYPTO_ERROR_UNKNOWN_BLOCK,
    CRYPTO_ERROR_UNKNOWN_LISTENER,

    // Node
    CRYPTO_ERROR_NODE_NOT_CONNECTED = 20000,

    // Transfer
    CRYPTO_ERROR_TRANSFER_HASH_MISMATCH = 30000,
    CRYPTO_ERROR_TRANSFER_SUBMISSION,

    // Numeric
    CRYPTO_ERROR_NUMERIC_PARSE = 40000,

    // Acount
    // Wallet
    // Block
    // Listener
} BRCryptoStatus;

/// MARK: - Reference Counting

typedef struct {
    _Atomic(unsigned int) count;
    void (*free) (void *);
} BRCryptoRef;

#if defined (CRYPTO_REF_DEBUG)
#include <stdio.h>
static int cryptoRefDebug = 1;
#define cryptoRefShow   printf
#else
static int cryptoRefDebug = 0;
#define cryptoRefShow 
#endif

#define DECLARE_CRYPTO_GIVE_TAKE(type, preface)                                   \
  extern type preface##Take (type obj);                                           \
  extern type preface##TakeWeak (type obj);                                       \
  extern void preface##Give (type obj)

#define IMPLEMENT_CRYPTO_GIVE_TAKE(type, preface)                                 \
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
    if (cryptoRefDebug && 0 == _c) { cryptoRefShow ("CRY: Missed: %s\n", #type); }\
    return (_c != 0) ? obj : NULL;                                                \
  }                                                                               \
  extern void                                                                     \
  preface##Give (type obj) {                                                      \
    if (NULL == obj) return;                                                      \
    unsigned int _c = atomic_fetch_sub (&obj->ref.count, 1);                      \
    /* catch give after release */                                                \
    assert (0 != _c);                                                             \
    if (1 == _c) {                                                                \
        if (cryptoRefDebug) { cryptoRefShow ("CRY: Release: %s\n", #type); }      \
        obj->ref.free (obj);                                                      \
    }                                                                             \
}

#define CRYPTO_AS_FREE(release)     ((void (*) (void *)) release)

#define CRYPTO_REF_ASSIGN(release)  (BRCryptoRef) { 1, CRYPTO_AS_FREE (release) }

#if !defined (private_extern)
#  define private_extern          extern
#endif

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoBase_h */
