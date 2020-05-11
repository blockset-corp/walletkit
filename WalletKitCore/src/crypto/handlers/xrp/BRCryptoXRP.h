
#ifndef BRCryptoXRP_h
#define BRCryptoXRP_h

#include "../../BRCryptoAddressP.h"
#include "../../BRCryptoNetworkP.h"
#include "../../BRCryptoTransferP.h"
#include "../../BRCryptoWalletP.h"

#include "ripple/BRRippleBase.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct BRCryptoAddressXRPRecord  *BRCryptoAddressXRP;
typedef struct BRCryptoNetworkXRPRecord  *BRCryptoNetworkXRP;
typedef struct BRCryptoTransferXRPRecord *BRCryptoTransferXRP;
typedef struct BRCryptoWalletXRPRecord   *BRCryptoWalletXRP;

private_extern BRCryptoHash
cryptoHashCreateAsXRP (BRRippleTransactionHash hash);


#ifdef __cplusplus
}
#endif

#endif // BRCryptoXRP_h
