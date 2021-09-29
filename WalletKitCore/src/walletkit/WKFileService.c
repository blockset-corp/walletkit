//
//  WKFileService.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-05-14.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "WKFileService.h"

#include "support/rlp/BRRlp.h"
#include "support/BRCrypto.h"

#include "WKClientP.h"
#include "WKNetworkP.h"
#include "WKWalletManagerP.h"

// MARK: - Client Transfer Bundle

private_extern UInt256
wkFileServiceTypeTransferV1Identifier (BRFileServiceContext context,
                                     BRFileService fs,
                                     const void *entity) {
    WKWalletManager        manager = (WKWalletManager) context; (void) manager;
    WKClientTransferBundle bundle  = (WKClientTransferBundle) entity;

    // Only the bundle->uids is guaranteed unique.  Both of bundle->{hash,identifer} refer to the
    // corresponding transaction's hash - but there can be multiple transfers in one transaction.

    UInt256 identifier = UINT256_ZERO;
    BRSHA256 (identifier.u8, bundle->uids, strlen(bundle->uids));

    return identifier;
}

private_extern void *
wkFileServiceTypeTransferV1Reader (BRFileServiceContext context,
                                   BRFileService fs,
                                   uint8_t *bytes,
                                   uint32_t bytesCount) {
    WKWalletManager manager = (WKWalletManager) context; (void) manager;

    BRRlpCoder coder = rlpCoderCreate();
    BRRlpData  data  = (BRRlpData) { bytesCount, bytes };
    BRRlpItem  item  = rlpDataGetItem (coder, data);

    WKClientTransferBundle bundle = wkClientTransferBundleRlpDecode(item, coder, WK_FILE_SERVICE_TYPE_TRANSFER_VERSION_1);

    rlpItemRelease (coder, item);
    rlpCoderRelease(coder);

    return bundle;
}

private_extern void *
wkFileServiceTypeTransferV2Reader (BRFileServiceContext context,
                                   BRFileService fs,
                                   uint8_t *bytes,
                                   uint32_t bytesCount) {
    WKWalletManager manager = (WKWalletManager) context; (void) manager;

    BRRlpCoder coder = rlpCoderCreate();
    BRRlpData  data  = (BRRlpData) { bytesCount, bytes };
    BRRlpItem  item  = rlpDataGetItem (coder, data);

    WKClientTransferBundle bundle = wkClientTransferBundleRlpDecode(item, coder, WK_FILE_SERVICE_TYPE_TRANSFER_VERSION_2);

    rlpItemRelease (coder, item);
    rlpCoderRelease(coder);

    return bundle;
}

private_extern uint8_t *
wkFileServiceTypeTransferV1Writer (BRFileServiceContext context,
                                 BRFileService fs,
                                 const void* entity,
                                 uint32_t *bytesCount) {
    WKWalletManager        manager = (WKWalletManager) context; (void) manager;
    WKClientTransferBundle bundle  = (WKClientTransferBundle) entity;

    BRRlpCoder coder = rlpCoderCreate();
    BRRlpItem  item  = wkClientTransferBundleRlpEncode (bundle, coder);
    BRRlpData  data  = rlpItemGetData (coder, item);

    rlpItemRelease  (coder, item);
    rlpCoderRelease (coder);

    *bytesCount = (uint32_t) data.bytesCount;
    return data.bytes;
}

// MARK: - Client Transaction Bundle

private_extern UInt256
wkFileServiceTypeTransactionV1Identifier (BRFileServiceContext context,
                                     BRFileService fs,
                                     const void *entity) {
    WKWalletManager           manager = (WKWalletManager) context; (void) manager;
    WKClientTransactionBundle bundle  = (WKClientTransactionBundle) entity;

    UInt256 identifier;

    BRSHA256 (identifier.u8, bundle->serialization, bundle->serializationCount);

    return identifier;
}

private_extern void *
wkFileServiceTypeTransactionV1Reader (BRFileServiceContext context,
                                 BRFileService fs,
                                 uint8_t *bytes,
                                 uint32_t bytesCount) {
    WKWalletManager manager = (WKWalletManager) context;
    (void) manager;

    BRRlpCoder coder = rlpCoderCreate();
    BRRlpData  data  = (BRRlpData) { bytesCount, bytes };
    BRRlpItem  item  = rlpDataGetItem (coder, data);

    WKClientTransactionBundle bundle = wkClientTransactionBundleRlpDecode(item, coder);

    rlpItemRelease (coder, item);
    rlpCoderRelease(coder);

    return bundle;
}

private_extern uint8_t *
wkFileServiceTypeTransactionV1Writer (BRFileServiceContext context,
                                 BRFileService fs,
                                 const void* entity,
                                 uint32_t *bytesCount) {
    WKWalletManager           manager = (WKWalletManager) context; (void) manager;
    WKClientTransactionBundle bundle  = (WKClientTransactionBundle) entity;

    BRRlpCoder coder = rlpCoderCreate();
    BRRlpItem  item  = wkClientTransactionBundleRlpEncode (bundle, coder);
    BRRlpData  data  = rlpItemGetData (coder, item);

    rlpItemRelease  (coder, item);
    rlpCoderRelease (coder);

    *bytesCount = (uint32_t) data.bytesCount;
    return data.bytes;
}

BRFileServiceTypeSpecification wkFileServiceSpecifications[] = {
    {
        WK_FILE_SERVICE_TYPE_TRANSFER,
        WK_FILE_SERVICE_TYPE_TRANSFER_VERSION_2, // current version
        2,
        {
            {
                WK_FILE_SERVICE_TYPE_TRANSFER_VERSION_1,
                wkFileServiceTypeTransferV1Identifier,
                wkFileServiceTypeTransferV1Reader,
                wkFileServiceTypeTransferV1Writer
            },

            {
                WK_FILE_SERVICE_TYPE_TRANSFER_VERSION_2,
                wkFileServiceTypeTransferV1Identifier,
                wkFileServiceTypeTransferV2Reader,
                wkFileServiceTypeTransferV1Writer
            },
        }
    },

    {
        WK_FILE_SERVICE_TYPE_TRANSACTION,
        WK_FILE_SERVICE_TYPE_TRANSACTION_VERSION_1, // current version
        1,
        {
            {
                WK_FILE_SERVICE_TYPE_TRANSACTION_VERSION_1,
                wkFileServiceTypeTransactionV1Identifier,
                wkFileServiceTypeTransactionV1Reader,
                wkFileServiceTypeTransactionV1Writer
            },
        }
    }
};
size_t wkFileServiceSpecificationsCount = (sizeof (wkFileServiceSpecifications) / sizeof (BRFileServiceTypeSpecification));
