//
//  WKFileService.h
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-05-14.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef WKFileService_h
#define WKFileService_h

#include "WKBase.h"
#include "support/util/BRUtil.h"
#include "support/BRFileService.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WK_FILE_SERVICE_TYPE_TRANSFER      "crypto_transfers"

typedef enum {
    WK_FILE_SERVICE_TYPE_TRANSFER_VERSION_1,
    WK_FILE_SERVICE_TYPE_TRANSFER_VERSION_2
} WKFileServiceTransferVersion;

private_extern UInt256
wkFileServiceTypeTransferV1Identifier (BRFileServiceContext context,
                                     BRFileService fs,
                                     const void *entity);

private_extern void *
wkFileServiceTypeTransferV1Reader (BRFileServiceContext context,
                                   BRFileService fs,
                                   uint8_t *bytes,
                                   uint32_t bytesCount);

private_extern void *
wkFileServiceTypeTransferV2Reader (BRFileServiceContext context,
                                   BRFileService fs,
                                   uint8_t *bytes,
                                   uint32_t bytesCount);

private_extern uint8_t *
wkFileServiceTypeTransferV1Writer (BRFileServiceContext context,
                                 BRFileService fs,
                                 const void* entity,
                                 uint32_t *bytesCount);


#define WK_FILE_SERVICE_TYPE_TRANSACTION      "crypto_transactions"

typedef enum {
    WK_FILE_SERVICE_TYPE_TRANSACTION_VERSION_1
} WKFileServiceTransactionVersion;

private_extern UInt256
wkFileServiceTypeTransactionV1Identifier (BRFileServiceContext context,
                                        BRFileService fs,
                                        const void *entity);

private_extern void *
wkFileServiceTypeTransactionV1Reader (BRFileServiceContext context,
                                    BRFileService fs,
                                    uint8_t *bytes,
                                    uint32_t bytesCount);

private_extern uint8_t *
wkFileServiceTypeTransactionV1Writer (BRFileServiceContext context,
                                    BRFileService fs,
                                    const void* entity,
                                    uint32_t *bytesCount);

extern BRFileServiceTypeSpecification wkFileServiceSpecifications[];
extern size_t wkFileServiceSpecificationsCount;

#endif /* WKFileService_h */
