
#ifndef BRCryptoBTC_h
#define BRCryptoBTC_h

#include "../../BRCryptoAddressP.h"
#include "../../BRCryptoNetworkP.h"
#include "../../BRCryptoTransferP.h"
#include "../../BRCryptoWalletP.h"
#include "../../BRCryptoWalletManagerP.h"

#include "bitcoin/BRWallet.h"
#include "bitcoin/BRTransaction.h"
#include "bitcoin/BRChainParams.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK - Address

typedef struct BRCryptoAddressBTCRecord  *BRCryptoAddressBTC;

extern BRCryptoAddressHandlers cryptoAddressHandlersBTC;

extern BRCryptoAddress
cryptoAddressCreateAsBTC (BRCryptoBlockChainType type,
                          BRAddress addr);

extern BRCryptoAddress
cryptoAddressCreateFromStringAsBTC (BRAddressParams params, const char *btcAddress);

extern BRCryptoAddress
cryptoAddressCreateFromStringAsBCH (BRAddressParams params, const char *bchAddress);

private_extern BRAddress
cryptoAddressAsBTC (BRCryptoAddress address,
                    BRCryptoBoolean *isBitcoinAddr);

#if 0



#endif

// MARK: - Network

typedef struct BRCryptoNetworkBTCRecord  *BRCryptoNetworkBTC;

extern BRCryptoNetworkHandlers cryptoNetworkHandlersBTC;

extern const BRChainParams *
cryptoNetworkAsBTC (BRCryptoNetwork network);

extern uint64_t
cryptoNetworkFeeAsBTC (BRCryptoNetworkFee networkFee);


// MARK: - Transfer

typedef struct BRCryptoTransferBTCRecord *BRCryptoTransferBTC;

extern BRCryptoTransferHandlers cryptoTransferHandlersBTC;

extern BRCryptoTransfer
cryptoTransferCreateAsBTC (BRCryptoUnit unit,
                           BRCryptoUnit unitForFee,
                           BRWallet *wid,
                           OwnershipKept BRTransaction *tid,
                           BRCryptoBlockChainType type);

extern BRCryptoHash
cryptoTransferGetHashBTC (BRCryptoTransfer transferBase);

private_extern BRTransaction *
cryptoTransferAsBTC (BRCryptoTransfer transferBase);

private_extern BRCryptoBoolean
cryptoTransferHasBTC (BRCryptoTransfer transferBase,
                      BRTransaction *btc);

// MARK: - Wallet

typedef struct BRCryptoWalletBTCRecord   *BRCryptoWalletBTC;

extern BRCryptoWalletHandlers cryptoWalletHandlersBTC;

private_extern BRWallet *
cryptoWalletAsBTC (BRCryptoWallet wallet);

private_extern BRCryptoWallet
cryptoWalletCreateAsBTC (BRCryptoUnit unit,
                         BRCryptoUnit unitForFee,
//                         BRWalletManager bwm,
                         BRWallet *wid);

private_extern BRCryptoTransfer
cryptoWalletFindTransferAsBTC (BRCryptoWallet wallet,
                               BRTransaction *btc);

#if 0
private_extern BRWallet *
cryptoWalletAsBTC (BRCryptoWallet wallet);

private_extern BRCryptoWallet
cryptoWalletCreateAsBTC (BRCryptoUnit unit,
                         BRCryptoUnit unitForFee,
                         BRWalletManager bwm,
                         BRWallet *wid);

private_extern BRCryptoTransfer
cryptoWalletFindTransferAsBTC (BRCryptoWallet wallet,
                               BRTransaction *btc);

private_extern BRCryptoWallet
cryptoWalletCreateAsBTC (BRCryptoUnit unit,
                         BRCryptoUnit unitForFee,
                         BRWalletManager bwm,
                         BRWallet *wid);

#endif
// MARK: - (Wallet) Manager

typedef struct BRCryptoWalletManagerBTCRecord  *BRCryptoWalletManagerBTC;

extern BRCryptoWalletManagerHandlers cryptoWalletManagerHandlersBTC;


#ifdef REFACTOR
private_extern BRWalletManager
cryptoWalletManagerAsBTC (BRCryptoWalletManager manager) {
    assert (BLOCK_CHAIN_TYPE_BTC == manager->type);
    return manager->u.btc;
}

private_extern BREthereumEWM
cryptoWalletManagerAsETH (BRCryptoWalletManager manager) {
    assert (BLOCK_CHAIN_TYPE_ETH == manager->type);
    return manager->u.eth;
}

private_extern BRCryptoBoolean
cryptoWalletManagerHasBTC (BRCryptoWalletManager manager,
                           BRWalletManager bwm) {
    return AS_CRYPTO_BOOLEAN (BLOCK_CHAIN_TYPE_BTC == manager->type && bwm == manager->u.btc);
}

private_extern BRCryptoBoolean
cryptoWalletManagerHasETH (BRCryptoWalletManager manager,
                           BREthereumEWM ewm) {
    return AS_CRYPTO_BOOLEAN (BLOCK_CHAIN_TYPE_ETH == manager->type && ewm == manager->u.eth);
}

private_extern BRCryptoBoolean
cryptoWalletManagerHasGEN (BRCryptoWalletManager manager,
                           BRGenericManager gwm) {
    return AS_CRYPTO_BOOLEAN (BLOCK_CHAIN_TYPE_GEN == manager->type && gwm == manager->u.gen);
}

private_extern BRCryptoWallet
cryptoWalletManagerFindWalletAsBTC (BRCryptoWalletManager cwm,
                                    BRWallet *btc) {
    BRCryptoWallet wallet = NULL;
    pthread_mutex_lock (&cwm->lock);
    for (size_t index = 0; index < array_count (cwm->wallets); index++) {
        if (btc == cryptoWalletAsBTC (cwm->wallets[index])) {
            wallet = cryptoWalletTake (cwm->wallets[index]);
            break;
        }
    }
    pthread_mutex_unlock (&cwm->lock);
    return wallet;
}

private_extern BRCryptoWallet
cryptoWalletManagerFindWalletAsETH (BRCryptoWalletManager cwm,
                                    BREthereumWallet eth) {
    BRCryptoWallet wallet = NULL;
    pthread_mutex_lock (&cwm->lock);
    for (size_t index = 0; index < array_count (cwm->wallets); index++) {
        if (eth == cryptoWalletAsETH (cwm->wallets[index])) {
            wallet = cryptoWalletTake (cwm->wallets[index]);
            break;
        }
    }
    pthread_mutex_unlock (&cwm->lock);
    return wallet;
}

private_extern BRCryptoWallet
cryptoWalletManagerFindWalletAsGEN (BRCryptoWalletManager cwm,
                                    BRGenericWallet gen) {
    BRCryptoWallet wallet = NULL;
    pthread_mutex_lock (&cwm->lock);
    for (size_t index = 0; index < array_count (cwm->wallets); index++) {
        if (gen == cryptoWalletAsGEN (cwm->wallets[index])) {
            wallet = cryptoWalletTake (cwm->wallets[index]);
            break;
        }
    }
    pthread_mutex_unlock (&cwm->lock);
    return wallet;
}
#endif

// MARK: - Events

extern const BREventType *bwmEventTypes[];
extern const unsigned int bwmEventTypesCount;

// MARK: - Support

private_extern BRCryptoFeeBasis
cryptoFeeBasisCreateAsBTC (BRCryptoUnit unit,
                           uint32_t feePerKB,
                           uint32_t sizeInByte);

private_extern uint64_t // SAT-per-KB
cryptoFeeBasisAsBTC (BRCryptoFeeBasis feeBasis);

private_extern BRCryptoHash
cryptoHashCreateAsBTC (UInt256 btc);


#ifdef __cplusplus
}
#endif

#endif // BRCryptoBTC_h
