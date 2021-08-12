//
//  WKConfig.h
//  WalletKitCore
//
//  Created by Ed Gamble on 12/9/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#define HAS_BTC_TESTNET     1
#define HAS_BCH_TESTNET     1
#define HAS_BSV_TESTNET     0
#define HAS_LTC_TESTNET     0
#define HAS_DOGE_TESTNET    0
#define HAS_ETH_TESTNET     1
#define HAS_XRP_TESTNET     0
#define HAS_HBAR_TESTNET    0
#define HAS_XTZ_TESTNET     0
#define HAS_XLM_TESTNET     0
#define HAS_AVAX_TESTNET    0
// __CONTROL__

#if !defined DEFINE_NETWORK
#define DEFINE_NETWORK(type, networkId, name, network, isMainnet, height, confirmations, confirmationPeriodInSeconds)
#endif

#if !defined DEFINE_NETWORK_FEE_ESTIMATE
#define DEFINE_NETWORK_FEE_ESTIMATE(networkId, amount, tier, confirmationTimeInMilliseconds)
#endif

#if !defined DEFINE_CURRENCY
#define DEFINE_CURRENCY(networkId, currencyId, name, code, type, address, verified)
#endif

#if !defined DEFINE_UNIT
#define DEFINE_UNIT(currencyId, name, code, decimals, symbol)
#endif

#if !defined DEFINE_ADDRESS_SCHEMES
#define DEFINE_ADDRESS_SCHEMES(networkId, defaultScheme, otherSchemes...)
#endif

#if !defined DEFINE_MODES
#define DEFINE_MODES(networkId, defaultMode, otherModes...)
#endif

#if !defined DEFINE_HANDLERS
#define DEFINE_HANDLERS(type,name)
#endif

// MARK: - BTC

#define NETWORK_NAME    "Bitcoin"
DEFINE_NETWORK (WK_NETWORK_TYPE_BTC,  "bitcoin-mainnet", NETWORK_NAME, "mainnet", true, 668821, 6, 10 * 60)
DEFINE_NETWORK_FEE_ESTIMATE ("bitcoin-mainnet", "18", "10m", 10 * 60 * 1000)
DEFINE_CURRENCY ("bitcoin-mainnet",     "bitcoin-mainnet:__native__",   NETWORK_NAME,  WK_NETWORK_CURRENCY_BTC,  "native",   NULL,   true)
    DEFINE_UNIT ("bitcoin-mainnet:__native__",      "Satoshi",    "sat",      0,      "SAT")
    DEFINE_UNIT ("bitcoin-mainnet:__native__",      NETWORK_NAME, "btc",      8,      "₿")
DEFINE_ADDRESS_SCHEMES  ("bitcoin-mainnet", WK_ADDRESS_SCHEME_BTC_SEGWIT,   WK_ADDRESS_SCHEME_BTC_LEGACY)
DEFINE_MODES            ("bitcoin-mainnet", WK_SYNC_MODE_API_ONLY,          WK_SYNC_MODE_P2P_ONLY)

DEFINE_NETWORK (WK_NETWORK_TYPE_BTC,  "bitcoin-testnet", NETWORK_NAME, "testnet", false, 1931985, 6, 10 * 60)
DEFINE_NETWORK_FEE_ESTIMATE ("bitcoin-testnet", "18", "10m", 10 * 60 * 1000)
DEFINE_CURRENCY ("bitcoin-testnet",     "bitcoin-testnet:__native__",   NETWORK_NAME,  WK_NETWORK_CURRENCY_BTC,  "native",   NULL,   true)
    DEFINE_UNIT ("bitcoin-testnet:__native__",      "Satoshi",    "sat",      0,      "SAT")
    DEFINE_UNIT ("bitcoin-testnet:__native__",      NETWORK_NAME, "btc",      8,      "₿")
DEFINE_ADDRESS_SCHEMES  ("bitcoin-testnet", WK_ADDRESS_SCHEME_BTC_SEGWIT,   WK_ADDRESS_SCHEME_BTC_LEGACY)
#if HAS_BTC_TESTNET
DEFINE_MODES            ("bitcoin-testnet", WK_SYNC_MODE_API_ONLY,          WK_SYNC_MODE_P2P_ONLY)
#else
DEFINE_MODES            ("bitcoin-testnet", CWK_SYNC_MODE_P2P_ONLY)
#endif
DEFINE_HANDLERS (WK_NETWORK_TYPE_BTC, BTC)
#undef NETWORK_NAME

// MARK: - BCH

#define NETWORK_NAME    "Bitcoin Cash"
DEFINE_NETWORK (WK_NETWORK_TYPE_BCH,  "bitcoincash-mainnet", NETWORK_NAME, "mainnet", true, 673068, 6, 10 * 60)
DEFINE_NETWORK_FEE_ESTIMATE ("bitcoincash-mainnet", "2", "10m", 20 * 60 * 1000)
DEFINE_CURRENCY ("bitcoincash-mainnet",     "bitcoincash-mainnet:__native__",   NETWORK_NAME,  WK_NETWORK_CURRENCY_BCH,  "native",   NULL,   true)
    DEFINE_UNIT ("bitcoincash-mainnet:__native__",      "Satoshi",    "sat",      0,      "SAT")
    DEFINE_UNIT ("bitcoincash-mainnet:__native__",      NETWORK_NAME, "bch",      8,      "BCH")
DEFINE_ADDRESS_SCHEMES  ("bitcoincash-mainnet", WK_ADDRESS_SCHEME_BTC_LEGACY)
DEFINE_MODES            ("bitcoincash-mainnet", WK_SYNC_MODE_API_ONLY, WK_SYNC_MODE_P2P_ONLY)

DEFINE_NETWORK (WK_NETWORK_TYPE_BCH,  "bitcoincash-testnet", NETWORK_NAME, "testnet", false, 1432976, 6, 10 * 60)
DEFINE_NETWORK_FEE_ESTIMATE ("bitcoincash-testnet", "2", "10m", 20 * 60 * 1000)
DEFINE_CURRENCY ("bitcoincash-testnet",     "bitcoincash-testnet:__native__",   NETWORK_NAME,  WK_NETWORK_CURRENCY_BCH,  "native",   NULL,   true)
    DEFINE_UNIT ("bitcoincash-testnet:__native__",      "Satoshi",    "sat",      0,      "SAT")
    DEFINE_UNIT ("bitcoincash-testnet:__native__",      NETWORK_NAME, "bch",      8,      "BCH")
DEFINE_ADDRESS_SCHEMES  ("bitcoincash-testnet", WK_ADDRESS_SCHEME_BTC_LEGACY)
#if HAS_BCH_TESTNET
DEFINE_MODES            ("bitcoincash-testnet", WK_SYNC_MODE_API_ONLY,  WK_SYNC_MODE_P2P_ONLY)
#else
DEFINE_MODES            ("bitcoincash-testnet", WK_SYNC_MODE_P2P_ONLY)
#endif
DEFINE_HANDLERS (WK_NETWORK_TYPE_BCH, BCH)
#undef NETWORK_NAME

// MARK: - BSV

#define NETWORK_NAME    "Bitcoin SV"
DEFINE_NETWORK (WK_NETWORK_TYPE_BSV,  "bitcoinsv-mainnet", NETWORK_NAME, "mainnet", true, 672728, 6, 10 * 60)
DEFINE_NETWORK_FEE_ESTIMATE ("bitcoinsv-mainnet", "2", "10m", 20 * 60 * 1000)
DEFINE_CURRENCY ("bitcoinsv-mainnet",     "bitcoinsv-mainnet:__native__",   NETWORK_NAME,  WK_NETWORK_CURRENCY_BSV,  "native",   NULL,   true)
    DEFINE_UNIT ("bitcoinsv-mainnet:__native__",      "Satoshi",    "sat",      0,      "SAT")
    DEFINE_UNIT ("bitcoinsv-mainnet:__native__",      NETWORK_NAME, "bsv",      8,      "BSV")
DEFINE_ADDRESS_SCHEMES  ("bitcoinsv-mainnet", WK_ADDRESS_SCHEME_BTC_LEGACY)
DEFINE_MODES            ("bitcoinsv-mainnet", WK_SYNC_MODE_API_ONLY, WK_SYNC_MODE_P2P_ONLY)

DEFINE_NETWORK (WK_NETWORK_TYPE_BSV,  "bitcoinsv-testnet", NETWORK_NAME, "testnet", false, 528135, 6, 10 * 60)
DEFINE_NETWORK_FEE_ESTIMATE ("bitcoinsv-testnet", "2", "10m", 20 * 60 * 1000)
DEFINE_CURRENCY ("bitcoinsv-testnet",     "bitcoinsv-testnet:__native__",   NETWORK_NAME,  WK_NETWORK_CURRENCY_BSV,  "native",   NULL,   true)
    DEFINE_UNIT ("bitcoinsv-testnet:__native__",      "Satoshi",    "sat",      0,      "SAT")
    DEFINE_UNIT ("bitcoinsv-testnet:__native__",      NETWORK_NAME, "bsv",      8,      "BSV")
DEFINE_ADDRESS_SCHEMES  ("bitcoinsv-testnet", WK_ADDRESS_SCHEME_BTC_LEGACY)
#if HAS_BSV_TESTNET
DEFINE_MODES            ("bitcoinsv-testnet", WK_SYNC_MODE_API_ONLY,  WK_SYNC_MODE_P2P_ONLY)
#else
DEFINE_MODES            ("bitcoinsv-testnet", WK_SYNC_MODE_P2P_ONLY)
#endif
DEFINE_HANDLERS (WK_NETWORK_TYPE_BSV, BSV)
#undef NETWORK_NAME

// MARK: - LTC

#define NETWORK_NAME    "Litecoin"
DEFINE_NETWORK (WK_NETWORK_TYPE_LTC,  "litecoin-mainnet", NETWORK_NAME, "mainnet", true, 2056308, 12, (5*60)/2)  // 2.5 min
DEFINE_NETWORK_FEE_ESTIMATE ("litecoin-mainnet", "2", "5m", 5 * 60 * 1000)
DEFINE_CURRENCY ("litecoin-mainnet",     "litecoin-mainnet:__native__",   NETWORK_NAME,  WK_NETWORK_CURRENCY_LTC,  "native",   NULL,   true)
    DEFINE_UNIT ("litecoin-mainnet:__native__",      "Satoshi",    "sat",      0,      "SAT")
    DEFINE_UNIT ("litecoin-mainnet:__native__",      NETWORK_NAME, "ltc",      8,      "LTC")
DEFINE_ADDRESS_SCHEMES  ("litecoin-mainnet", WK_ADDRESS_SCHEME_BTC_SEGWIT, WK_ADDRESS_SCHEME_BTC_LEGACY)
DEFINE_MODES            ("litecoin-mainnet", WK_SYNC_MODE_API_ONLY, WK_SYNC_MODE_P2P_ONLY)

DEFINE_NETWORK (WK_NETWORK_TYPE_LTC,  "litecoin-testnet", NETWORK_NAME, "testnet", false, 1903181, 12, (5*60)/2)  // 2.5 min
DEFINE_NETWORK_FEE_ESTIMATE ("litecoin-testnet", "2", "5m", 5 * 60 * 1000)
DEFINE_CURRENCY ("litecoin-testnet",     "litecoin-testnet:__native__",   NETWORK_NAME,  WK_NETWORK_CURRENCY_LTC,  "native",   NULL,   true)
    DEFINE_UNIT ("litecoin-testnet:__native__",      "Satoshi",    "sat",      0,      "SAT")
    DEFINE_UNIT ("litecoin-testnet:__native__",      NETWORK_NAME, "ltc",      8,      "LTC")
DEFINE_ADDRESS_SCHEMES  ("litecoin-testnet", WK_ADDRESS_SCHEME_BTC_SEGWIT, WK_ADDRESS_SCHEME_BTC_LEGACY)
#if HAS_LTC_TESTNET
DEFINE_MODES            ("litecoin-testnet", WK_SYNC_MODE_API_ONLY,  WK_SYNC_MODE_P2P_ONLY)
#else
DEFINE_MODES            ("litecoin-testnet", WK_SYNC_MODE_P2P_ONLY)
#endif
DEFINE_HANDLERS (WK_NETWORK_TYPE_LTC, LTC)
#undef NETWORK_NAME

// MARK: - DOGE

#define NETWORK_NAME    "Dogecoin"
DEFINE_NETWORK (WK_NETWORK_TYPE_DOGE,  "dogecoin-mainnet", NETWORK_NAME, "mainnet", true, 3744046, 40, 1 * 60)
DEFINE_NETWORK_FEE_ESTIMATE ("dogecoin-mainnet", "400000", "10m", 10 * 60 * 1000) // 1 DOGE / 240 bytes => 400,000 SAT/byte
DEFINE_CURRENCY ("dogecoin-mainnet",     "dogecoin-mainnet:__native__",   NETWORK_NAME,  WK_NETWORK_CURRENCY_DOGE,  "native",   NULL,   true)
DEFINE_UNIT ("dogecoin-mainnet:__native__",      "Satoshi",    "sat",      0,      "SAT")
DEFINE_UNIT ("dogecoin-mainnet:__native__",      NETWORK_NAME, "doge",     8,      "Ð")
DEFINE_ADDRESS_SCHEMES  ("dogecoin-mainnet", WK_ADDRESS_SCHEME_BTC_LEGACY)
DEFINE_MODES            ("dogecoin-mainnet", WK_SYNC_MODE_API_ONLY, WK_SYNC_MODE_P2P_ONLY)

DEFINE_NETWORK (WK_NETWORK_TYPE_DOGE,  "dogecoin-testnet", NETWORK_NAME, "testnet", false, 3194118, 40, 1 * 60)
DEFINE_NETWORK_FEE_ESTIMATE ("dogecoin-testnet", "400000", "10m", 10 * 60 * 1000)
DEFINE_CURRENCY ("dogecoin-testnet",     "dogecoin-testnet:__native__",   NETWORK_NAME,  WK_NETWORK_CURRENCY_DOGE,  "native",   NULL,   true)
DEFINE_UNIT ("dogecoin-testnet:__native__",      "Satoshi",    "sat",      0,      "SAT")
DEFINE_UNIT ("dogecoin-testnet:__native__",      NETWORK_NAME, "doge",     8,      "Ð")
DEFINE_ADDRESS_SCHEMES  ("dogecoin-testnet", WK_ADDRESS_SCHEME_BTC_LEGACY)
#if HAS_DOGE_TESTNET
DEFINE_MODES            ("dogecoin-testnet", WK_SYNC_MODE_API_ONLY,  WK_SYNC_MODE_P2P_ONLY)
#else
DEFINE_MODES            ("dogecoin-testnet", WK_SYNC_MODE_P2P_ONLY)
#endif
DEFINE_HANDLERS (WK_NETWORK_TYPE_DOGE, DOGE)
#undef NETWORK_NAME


// MARK: - ETH

#define NETWORK_NAME    "Ethereum"
DEFINE_NETWORK (WK_NETWORK_TYPE_ETH,  "ethereum-mainnet", NETWORK_NAME, "mainnet", true, 11779945, 6, 15)
DEFINE_NETWORK_FEE_ESTIMATE ("ethereum-mainnet", "25000000000", "1m", 1 * 60 * 1000)
DEFINE_CURRENCY ("ethereum-mainnet",     "ethereum-mainnet:__native__",   NETWORK_NAME,  WK_NETWORK_CURRENCY_ETH,  "native",   NULL,   true)
    DEFINE_UNIT ("ethereum-mainnet:__native__",      "Wei",         "wei",      0,      "WEI")
    DEFINE_UNIT ("ethereum-mainnet:__native__",      "Gwei",        "gwei",     9,      "GWEI")
    DEFINE_UNIT ("ethereum-mainnet:__native__",      "Ether",       "eth",     18,      "Ξ")
DEFINE_CURRENCY ("ethereum-mainnet",    "ethereum-mainnet:0x558ec3152e2eb2174905cd19aea4e34a23de9ad6",  "BRD Token",    "brd",  "erc20",   "0x558ec3152e2eb2174905cd19aea4e34a23de9ad6",   true)
    DEFINE_UNIT ("ethereum-mainnet:0x558ec3152e2eb2174905cd19aea4e34a23de9ad6",      "BRD Token INT",         "brdi",      0,      "BRDI")
    DEFINE_UNIT ("ethereum-mainnet:0x558ec3152e2eb2174905cd19aea4e34a23de9ad6",      "BRD Token",             "brd",       18,     "BRD")
DEFINE_ADDRESS_SCHEMES  ("ethereum-mainnet", WK_ADDRESS_SCHEME_NATIVE)
DEFINE_MODES            ("ethereum-mainnet", WK_SYNC_MODE_API_ONLY)

#if HAS_ETH_TESTNET
DEFINE_NETWORK (WK_NETWORK_TYPE_ETH,  "ethereum-ropsten", NETWORK_NAME, "testnet", false, 9588166, 6, 15)
DEFINE_NETWORK_FEE_ESTIMATE ("ethereum-ropsten", "17500000000", "1m", 1 * 60 * 1000)
DEFINE_CURRENCY ("ethereum-ropsten",     "ethereum-ropsten:__native__",   NETWORK_NAME,  WK_NETWORK_CURRENCY_ETH,  "native",   NULL,   true)
    DEFINE_UNIT ("ethereum-ropsten:__native__",      "Wei",         "wei",      0,      "WEI")
    DEFINE_UNIT ("ethereum-ropsten:__native__",      "Gwei",        "gwei",     9,      "GWEI")
    DEFINE_UNIT ("ethereum-ropsten:__native__",      "Ether",       "eth",     18,      "Ξ")
DEFINE_CURRENCY ("ethereum-ropsten",    "ethereum-ropsten:0x7108ca7c4718efa810457f228305c9c71390931a",  "BRD Token Testnet",    "brd",  "erc20",   "0x7108ca7c4718efa810457f228305c9c71390931a",   true)
    DEFINE_UNIT ("ethereum-ropsten:0x7108ca7c4718efa810457f228305c9c71390931a",      "BRD Token INT",         "brdi",      0,      "BRDI")
    DEFINE_UNIT ("ethereum-ropsten:0x7108ca7c4718efa810457f228305c9c71390931a",      "BRD Token",             "brd",       18,     "BRD")
DEFINE_CURRENCY ("ethereum-ropsten",    "ethereum-ropsten:0x722dd3f80bac40c951b51bdd28dd19d435762180",  "Standard Test Token",    "tst",  "erc20",   "0x722dd3f80bac40c951b51bdd28dd19d435762180",   true)
    DEFINE_UNIT ("ethereum-ropsten:0x722dd3f80bac40c951b51bdd28dd19d435762180",      "TST Token INT",         "tsti",      0,      "TSTI")
    DEFINE_UNIT ("ethereum-ropsten:0x722dd3f80bac40c951b51bdd28dd19d435762180",      "TST Token",             "tst",       18,     "TST")
DEFINE_ADDRESS_SCHEMES  ("ethereum-ropsten", WK_ADDRESS_SCHEME_NATIVE)
DEFINE_MODES            ("ethereum-ropsten", WK_SYNC_MODE_API_ONLY)
#endif // HAS_ETH_TESTNET
DEFINE_HANDLERS (WK_NETWORK_TYPE_ETH, ETH)
#undef NETWORK_NAME

// MARK: XRP

#define NETWORK_NAME    "Ripple"
DEFINE_NETWORK (WK_NETWORK_TYPE_XRP,  "ripple-mainnet", NETWORK_NAME, "mainnet", true, 61321875, 1, 5)
DEFINE_NETWORK_FEE_ESTIMATE ("ripple-mainnet", "10", "1m", 1 * 60 * 1000)
DEFINE_CURRENCY ("ripple-mainnet",     "ripple-mainnet:__native__",   NETWORK_NAME,  WK_NETWORK_CURRENCY_XRP,  "native",   NULL,   true)
    DEFINE_UNIT ("ripple-mainnet:__native__",      "Drop",       "drop",      0,      "DROP")
    DEFINE_UNIT ("ripple-mainnet:__native__",      NETWORK_NAME, "xrp",       6,      "XRP")
DEFINE_ADDRESS_SCHEMES  ("ripple-mainnet", WK_ADDRESS_SCHEME_NATIVE)
DEFINE_MODES            ("ripple-mainnet", WK_SYNC_MODE_API_ONLY)

#if HAS_XRP_TESTNET
DEFINE_NETWORK (WK_NETWORK_TYPE_XRP,  "ripple-testnet", NETWORK_NAME, "testnet", false, 50000, 1, 5)
DEFINE_NETWORK_FEE_ESTIMATE ("ripple-testnet", "10", "1m", 1 * 60 * 1000)
DEFINE_CURRENCY ("ripple-testnet",     "ripple-testnet:__native__",   NETWORK_NAME,  WK_NETWORK_CURRENCY_XRP,  "native",   NULL,   true)
    DEFINE_UNIT ("ripple-testnet:__native__",      "Drop",       "drop",      0,      "DROP")
    DEFINE_UNIT ("ripple-testnet:__native__",      NETWORK_NAME, "xrp",       6,      "XRP")
DEFINE_ADDRESS_SCHEMES  ("ripple-testnet", WK_ADDRESS_SCHEME_NATIVE)
DEFINE_MODES            ("ripple-testnet", WK_SYNC_MODE_API_ONLY)
#endif // HAS_XRP_TESTNET
DEFINE_HANDLERS (WK_NETWORK_TYPE_XRP, XRP)
#undef NETWORK_NAME

// MARK: HBAR

#define NETWORK_NAME    "Hedera"
DEFINE_NETWORK (WK_NETWORK_TYPE_HBAR,  "hedera-mainnet", NETWORK_NAME, "mainnet", true, 12295580, 1, 5)
DEFINE_NETWORK_FEE_ESTIMATE ("hedera-mainnet", "500000", "1m", 1 * 60 * 1000)
DEFINE_CURRENCY ("hedera-mainnet",     "hedera-mainnet:__native__",   NETWORK_NAME,  WK_NETWORK_CURRENCY_HBAR,  "native",   NULL,   true)
    DEFINE_UNIT ("hedera-mainnet:__native__",  "tinybar",     "thbar",  0,  "tℏ")
    DEFINE_UNIT ("hedera-mainnet:__native__",  NETWORK_NAME,  "hbar",   8,  "ℏ")
DEFINE_ADDRESS_SCHEMES  ("hedera-mainnet", WK_ADDRESS_SCHEME_NATIVE)
DEFINE_MODES            ("hedera-mainnet", WK_SYNC_MODE_API_ONLY)

#if HAS_HBAR_TESTNET
DEFINE_NETWORK (WK_NETWORK_TYPE_HBAR,  "hedera-testnet", NETWORK_NAME, "testnet", false, 50000, 1, 5)
DEFINE_NETWORK_FEE_ESTIMATE ("hedera-testnet", "500000", "1m", 1 * 60 * 1000)
DEFINE_CURRENCY ("hedera-testnet",     "hedera-testnet:__native__",   NETWORK_NAME,  WK_NETWORK_CURRENCY_HBAR,  "native",   NULL,   true)
    DEFINE_UNIT ("hedera-testnet:__native__",  "tinybar",     "thbar",  0,  "tℏ")
    DEFINE_UNIT ("hedera-testnet:__native__",  NETWORK_NAME,  "hbar",   8,  "ℏ")
DEFINE_ADDRESS_SCHEMES  ("hedera-testnet", WK_ADDRESS_SCHEME_NATIVE)
DEFINE_MODES            ("hedera-testnet", WK_SYNC_MODE_API_ONLY)
#endif // HAS_HBAR_TESTNET
DEFINE_HANDLERS (WK_NETWORK_TYPE_HBAR, HBAR)
#undef NETWORK_NAME

// MARK: Tezos

#define NETWORK_NAME    "Tezos"
DEFINE_NETWORK (WK_NETWORK_TYPE_XTZ,  "tezos-mainnet", NETWORK_NAME, "mainnet", true, 1328407, 6, 60)
DEFINE_NETWORK_FEE_ESTIMATE ("tezos-mainnet", "1", "1m", 1 * 60 * 1000)  // 1 mutez/byte
DEFINE_CURRENCY ("tezos-mainnet",     "tezos-mainnet:__native__",   NETWORK_NAME,  WK_NETWORK_CURRENCY_XTZ,  "native",   NULL,   true)
    DEFINE_UNIT ("tezos-mainnet:__native__",  "mutez",     "mtz",   0,  "mutez")
    DEFINE_UNIT ("tezos-mainnet:__native__",  NETWORK_NAME,  "xtz",   6,  "XTZ")
DEFINE_ADDRESS_SCHEMES  ("tezos-mainnet", WK_ADDRESS_SCHEME_NATIVE)
DEFINE_MODES            ("tezos-mainnet", WK_SYNC_MODE_API_ONLY)

#if HAS_XTZ_TESTNET
DEFINE_NETWORK (WK_NETWORK_TYPE_XTZ,  "tezos-testnet", NETWORK_NAME, "testnet", false, 50000, 6, 60)
DEFINE_NETWORK_FEE_ESTIMATE ("tezos-testnet", "1", "1m", 1 * 60 * 1000)   // 1 mutez/byte
DEFINE_CURRENCY ("tezos-testnet",     "tezos-testnet:__native__",   NETWORK_NAME,  WK_NETWORK_CURRENCY_XTZ,  "native",   NULL,   true)
    DEFINE_UNIT ("tezos-testnet:__native__",  "mutez",     "mtz",   0,  "mutez")
    DEFINE_UNIT ("tezos-testnet:__native__",  NETWORK_NAME,  "xtz",   6,  "XTZ")
DEFINE_ADDRESS_SCHEMES  ("tezos-testnet", WK_ADDRESS_SCHEME_NATIVE)
DEFINE_MODES            ("tezos-testnet", WK_SYNC_MODE_API_ONLY)
#endif // HAS_XTZ_TESTNET
DEFINE_HANDLERS (WK_NETWORK_TYPE_XTZ, XTZ)
#undef NETWORK_NAME

// MARK: XLM Mainnet

#define NETWORK_NAME    "Stellar"
DEFINE_NETWORK (WK_NETWORK_TYPE_XLM,  "stellar-mainnet", NETWORK_NAME, "mainnet", true, 35516170, 1, 5)
DEFINE_NETWORK_FEE_ESTIMATE ("stellar-mainnet", "100", "5s", 5 * 1000)
DEFINE_CURRENCY ("stellar-mainnet",     "stellar-mainnet:__native__",   NETWORK_NAME,  WK_NETWORK_CURRENCY_XLM,  "native",   NULL,   true)
    DEFINE_UNIT ("stellar-mainnet:__native__",  "Stroop",     "stroop",   0,  "STROOP")
    DEFINE_UNIT ("stellar-mainnet:__native__",  NETWORK_NAME,  "xlm",     7,  "XLM")
DEFINE_ADDRESS_SCHEMES  ("stellar-mainnet", WK_ADDRESS_SCHEME_NATIVE)
DEFINE_MODES            ("stellar-mainnet", WK_SYNC_MODE_API_ONLY)

#if HAS_XLM_TESTNET
DEFINE_NETWORK (WK_NETWORK_TYPE_XLM,  "stellar-testnet", NETWORK_NAME, "testnet", false, 1075721, 1, 5)
DEFINE_NETWORK_FEE_ESTIMATE ("stellar-testnet", "100", "5s", 5 * 1000)
DEFINE_CURRENCY ("stellar-testnet",     "stellar-testnet:__native__",   NETWORK_NAME,  WK_NETWORK_CURRENCY_XLM,  "native",   NULL,   true)
    DEFINE_UNIT ("stellar-testnet:__native__",  "Stroop",     "stroop",   0,  "STROOP")
    DEFINE_UNIT ("stellar-testnet:__native__",  NETWORK_NAME,  "xlm",     7,  "XLM")
DEFINE_ADDRESS_SCHEMES  ("stellar-testnet", WK_ADDRESS_SCHEME_NATIVE)
DEFINE_MODES            ("stellar-testnet", WK_SYNC_MODE_API_ONLY)
#endif // HAS_XLM_TESTNET
DEFINE_HANDLERS (WK_NETWORK_TYPE_XLM, XLM)
#undef NETWORK_NAME

// MARK: AVAX Mainnet

#define NETWORK_NAME    "Avalanche"
DEFINE_NETWORK (WK_NETWORK_TYPE_AVAX,  "avalanche-mainnet", NETWORK_NAME, "mainnet", true, 35516170, 1, 5)
DEFINE_NETWORK_FEE_ESTIMATE ("avalanche-mainnet", "100", "5s", 5 * 1000)
DEFINE_CURRENCY ("avalanche-mainnet",     "avalanche-mainnet:__native__",   NETWORK_NAME,  WK_NETWORK_CURRENCY_AVAX,  "native",   NULL,   true)
    DEFINE_UNIT ("avalanche-mainnet:__native__",  "lumen_i",     "xlm_i",   0,  "AVAX_I")
    DEFINE_UNIT ("avalanche-mainnet:__native__",  "lumen",       "xlm",     7,  "AVAX")
DEFINE_ADDRESS_SCHEMES  ("avalanche-mainnet", WK_ADDRESS_SCHEME_NATIVE)
DEFINE_MODES            ("avalanche-mainnet", WK_SYNC_MODE_API_ONLY)

#if HAS_AVAX_TESTNET
DEFINE_NETWORK (WK_NETWORK_TYPE_AVAX,  "avalanche-testnet", NETWORK_NAME, "testnet", false, 1075721, 1, 5)
DEFINE_NETWORK_FEE_ESTIMATE ("avalanche-testnet", "100", "5s", 5 * 1000)
DEFINE_CURRENCY ("avalanche-testnet",     "avalanche-testnet:__native__",   NETWORK_NAME,  WK_NETWORK_CURRENCY_AVAX,  "native",   NULL,   true)
    DEFINE_UNIT ("avalanche-testnet:__native__",  "lumen_i",     "txlm_i",   0,  "tAVAX_I")
    DEFINE_UNIT ("avalanche-testnet:__native__",  "lumen",       "txlm",     7,  "tAVAX")
DEFINE_ADDRESS_SCHEMES  ("avalanche-testnet", WK_ADDRESS_SCHEME_NATIVE)
DEFINE_MODES            ("avalanche-testnet", WK_SYNC_MODE_API_ONLY)
#endif
DEFINE_HANDLERS (WK_NETWORK_TYPE_AVAX, AVAX)
#undef NETWORK_NAME

// __CONFIG__

#undef DEFINE_NETWORK
#undef DEFINE_NETWORK_FEE_ESTIMATE
#undef DEFINE_CURRENCY
#undef DEFINE_UNIT
#undef DEFINE_ADDRESS_SCHEMES
#undef DEFINE_MODES
#undef DEFINE_HANDLERS
