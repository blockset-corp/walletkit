//
//  BRCryptoFileService.h
//  Core
//
//  Created by Ehsan Rezaie on 2020-05-14.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef BRCryptoFileService_h
#define BRCryptoFileService_h

#include "BRCryptoBase.h"
#include "ethereum/util/BRUtil.h"
#include "support/BRFileService.h"

#ifdef __cplusplus
extern "C" {
#endif

#define fileServiceTypeTransactions      "transactions"

typedef enum {
    GENERIC_TRANSFER_VERSION_1,
    GENERIC_TRANSFER_VERSION_2,
} BRGenericFileServiceTransferVersion;

private_extern UInt256
fileServiceTypeTransferV1Identifier (BRFileServiceContext context,
                                     BRFileService fs,
                                     const void *entity);

private_extern void *
fileServiceTypeTransferV1Reader (BRFileServiceContext context,
                                 BRFileService fs,
                                 uint8_t *bytes,
                                 uint32_t bytesCount);

private_extern uint8_t *
fileServiceTypeTransferV1Writer (BRFileServiceContext context,
                                 BRFileService fs,
                                 const void* entity,
                                 uint32_t *bytesCount);

private_extern uint8_t *
fileServiceTypeTransferV2Writer (BRFileServiceContext context,
                                 BRFileService fs,
                                 const void* entity,
                                 uint32_t *bytesCount);

static BRFileServiceTypeSpecification fileServiceSpecifications[] = {
    {
        fileServiceTypeTransactions,
        GENERIC_TRANSFER_VERSION_2, // current version
        2,
        {
            {
                GENERIC_TRANSFER_VERSION_1,
                fileServiceTypeTransferV1Identifier,
                fileServiceTypeTransferV1Reader,
                fileServiceTypeTransferV1Writer
            },

            {
                GENERIC_TRANSFER_VERSION_2,
                fileServiceTypeTransferV1Identifier,
                fileServiceTypeTransferV1Reader,
                fileServiceTypeTransferV2Writer
            },
        }
    }
};
static size_t fileServiceSpecificationsCount = (sizeof (fileServiceSpecifications) / sizeof (BRFileServiceTypeSpecification));


#endif /* BRCryptoFileService_h */
