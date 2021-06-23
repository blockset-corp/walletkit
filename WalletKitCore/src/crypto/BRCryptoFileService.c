//
//  BRCryptoFileService.c
//  Core
//
//  Created by Ehsan Rezaie on 2020-05-14.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "BRCryptoFileService.h"

#include "support/rlp/BRRlp.h"
#include "support/BRCrypto.h"

#include "BRCryptoClientP.h"
#include "BRCryptoNetworkP.h"
#include "BRCryptoWalletManagerP.h"

// MARK: - Client Transfer Bundle

private_extern UInt256
cryptoFileServiceTypeTransferV1Identifier (BRFileServiceContext context,
                                     BRFileService fs,
                                     const void *entity) {
    BRCryptoWalletManager        manager = (BRCryptoWalletManager) context; (void) manager;
    BRCryptoClientTransferBundle bundle  = (BRCryptoClientTransferBundle) entity;

    // Only the bundle->uids is guaranteed unique.  Both of bundle->{hash,identifer} refer to the
    // corresponding transaction's hash - but there can be multiple transfers in one transaction.

    UInt256 identifier = UINT256_ZERO;
    BRSHA256 (identifier.u8, bundle->uids, strlen(bundle->uids));

    return identifier;
}

private_extern void *
cryptoFileServiceTypeTransferV1Reader (BRFileServiceContext context,
                                 BRFileService fs,
                                 uint8_t *bytes,
                                 uint32_t bytesCount) {
    BRCryptoWalletManager manager = (BRCryptoWalletManager) context; (void) manager;

    BRRlpCoder coder = rlpCoderCreate();
    BRRlpData  data  = (BRRlpData) { bytesCount, bytes };
    BRRlpItem  item  = rlpDataGetItem (coder, data);

    BRCryptoClientTransferBundle bundle = cryptoClientTransferBundleRlpDecode (item, coder,
                                                                               CRYPTO_FILE_SERVICE_TYPE_TRANSFER_VERSION_1);

    rlpItemRelease (coder, item);
    rlpCoderRelease(coder);

    return bundle;
}

private_extern void *
cryptoFileServiceTypeTransferV2Reader (BRFileServiceContext context,
                                       BRFileService fs,
                                       uint8_t *bytes,
                                       uint32_t bytesCount) {
    BRCryptoWalletManager manager = (BRCryptoWalletManager) context; (void) manager;

    BRRlpCoder coder = rlpCoderCreate();
    BRRlpData  data  = (BRRlpData) { bytesCount, bytes };
    BRRlpItem  item  = rlpDataGetItem (coder, data);

    BRCryptoClientTransferBundle bundle = cryptoClientTransferBundleRlpDecode(item, coder,
                                                                              CRYPTO_FILE_SERVICE_TYPE_TRANSFER_VERSION_2);

    rlpItemRelease (coder, item);
    rlpCoderRelease(coder);

    return bundle;

}
private_extern uint8_t *
cryptoFileServiceTypeTransferV1Writer (BRFileServiceContext context,
                                 BRFileService fs,
                                 const void* entity,
                                 uint32_t *bytesCount) {
    BRCryptoWalletManager        manager = (BRCryptoWalletManager) context; (void) manager;
    BRCryptoClientTransferBundle bundle  = (BRCryptoClientTransferBundle) entity;

    BRRlpCoder coder = rlpCoderCreate();
    BRRlpItem  item  = cryptoClientTransferBundleRlpEncode (bundle, coder);
    BRRlpData  data  = rlpItemGetData (coder, item);

    rlpItemRelease  (coder, item);
    rlpCoderRelease (coder);

    *bytesCount = (uint32_t) data.bytesCount;
    return data.bytes;
}

// MARK: - Client Transaction Bundle

private_extern UInt256
cryptoFileServiceTypeTransactionV1Identifier (BRFileServiceContext context,
                                     BRFileService fs,
                                     const void *entity) {
    BRCryptoWalletManager           manager = (BRCryptoWalletManager) context; (void) manager;
    BRCryptoClientTransactionBundle bundle  = (BRCryptoClientTransactionBundle) entity;

    UInt256 identifier;

    BRSHA256 (identifier.u8, bundle->serialization, bundle->serializationCount);

    return identifier;
}

private_extern void *
cryptoFileServiceTypeTransactionV1Reader (BRFileServiceContext context,
                                 BRFileService fs,
                                 uint8_t *bytes,
                                 uint32_t bytesCount) {
    BRCryptoWalletManager manager = (BRCryptoWalletManager) context;
    (void) manager;

    BRRlpCoder coder = rlpCoderCreate();
    BRRlpData  data  = (BRRlpData) { bytesCount, bytes };
    BRRlpItem  item  = rlpDataGetItem (coder, data);

    BRCryptoClientTransactionBundle bundle = cryptoClientTransactionBundleRlpDecode(item, coder);

    rlpItemRelease (coder, item);
    rlpCoderRelease(coder);

    return bundle;
}

private_extern uint8_t *
cryptoFileServiceTypeTransactionV1Writer (BRFileServiceContext context,
                                 BRFileService fs,
                                 const void* entity,
                                 uint32_t *bytesCount) {
    BRCryptoWalletManager           manager = (BRCryptoWalletManager) context; (void) manager;
    BRCryptoClientTransactionBundle bundle  = (BRCryptoClientTransactionBundle) entity;

    BRRlpCoder coder = rlpCoderCreate();
    BRRlpItem  item  = cryptoClientTransactionBundleRlpEncode (bundle, coder);
    BRRlpData  data  = rlpItemGetData (coder, item);

    rlpItemRelease  (coder, item);
    rlpCoderRelease (coder);

    *bytesCount = (uint32_t) data.bytesCount;
    return data.bytes;
}

BRFileServiceTypeSpecification cryptoFileServiceSpecifications[] = {
    {
        CRYPTO_FILE_SERVICE_TYPE_TRANSFER,
        CRYPTO_FILE_SERVICE_TYPE_TRANSFER_VERSION_2, // current version
        2,
        {
            {
                CRYPTO_FILE_SERVICE_TYPE_TRANSFER_VERSION_1,
                cryptoFileServiceTypeTransferV1Identifier,
                cryptoFileServiceTypeTransferV1Reader,
                cryptoFileServiceTypeTransferV1Writer
            },
            {
                CRYPTO_FILE_SERVICE_TYPE_TRANSFER_VERSION_2,
                cryptoFileServiceTypeTransferV1Identifier,
                cryptoFileServiceTypeTransferV2Reader,
                cryptoFileServiceTypeTransferV1Writer
            },
        }
    },

    {
        CRYPTO_FILE_SERVICE_TYPE_TRANSACTION,
        CRYPTO_FILE_SERVICE_TYPE_TRANSACTION_VERSION_1, // current version
        1,
        {
            {
                CRYPTO_FILE_SERVICE_TYPE_TRANSACTION_VERSION_1,
                cryptoFileServiceTypeTransactionV1Identifier,
                cryptoFileServiceTypeTransactionV1Reader,
                cryptoFileServiceTypeTransactionV1Writer
            },
        }
    }
};
size_t cryptoFileServiceSpecificationsCount = (sizeof (cryptoFileServiceSpecifications) / sizeof (BRFileServiceTypeSpecification));
