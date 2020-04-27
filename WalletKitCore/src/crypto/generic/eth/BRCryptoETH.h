#ifndef BRCryptoETH_h
#define BRCryptoETH_h

#include "../../BRCryptoAddressP.h"
#include "../../BRCryptoNetworkP.h"
#include "../../BRCryptoTransferP.h"
#include "../../BRCryptoWalletP.h"

#include "ethereum/base/BREthereumAccount.h"
#include "ethereum/base/BREthereumHash.h"
#include "ethereum/contract/BREthereumToken.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BRCryptoAddressETHRecord  *BRCryptoAddressETH;
typedef struct BRCryptoNetworkETHRecord  *BRCryptoNetworkETH;
typedef struct BRCryptoTransferETHRecord *BRCryptoTransferETH;
typedef struct BRCryptoWalletETHRecord   *BRCryptoWalletETH;

private_extern BRCryptoCurrency
cryptoNetworkGetCurrencyforTokenETH (BRCryptoNetwork network,
                                     BREthereumToken token) {
    BRCryptoCurrency tokenCurrency = NULL;
    pthread_mutex_lock (&network->lock);
    for (size_t index = 0; index < array_count(network->associations); index++) {
        BRCryptoCurrency currency = network->associations[index].currency;
        const char *address = cryptoCurrencyGetIssuer (currency);

        if (NULL != address && ETHEREUM_BOOLEAN_IS_TRUE (ethTokenHasAddress (token, address))) {
            tokenCurrency = cryptoCurrencyTake (currency);
            break;
        }
    }
    pthread_mutex_unlock (&network->lock);
    return tokenCurrency;
}

// MARK: - Support

#ifdef REFACTOR
private_extern BRCryptoFeeBasis
cryptoFeeBasisCreateAsBTC (BRCryptoUnit unit,
                           uint32_t feePerKB,
                           uint32_t sizeInByte);

private_extern uint64_t // SAT-per-KB
cryptoFeeBasisAsBTC (BRCryptoFeeBasis feeBasis);
#endif

private_extern BRCryptoHash
cryptoHashCreateAsETH (BREtherumHash hash);

#ifdef __cplusplus
}
#endif

#endif // BRCryptoETH_h
