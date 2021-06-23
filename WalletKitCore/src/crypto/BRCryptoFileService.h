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

#define CRYPTO_FILE_SERVICE_TYPE_TRANSFER      "crypto_transfers"

typedef enum {
    CRYPTO_FILE_SERVICE_TYPE_TRANSFER_VERSION_1,
    CRYPTO_FILE_SERVICE_TYPE_TRANSFER_VERSION_2
} BRCryptoFileServiceTransferVersion;

private_extern UInt256
cryptoFileServiceTypeTransferV1Identifier (BRFileServiceContext context,
                                     BRFileService fs,
                                     const void *entity);

private_extern void *
cryptoFileServiceTypeTransferV1Reader (BRFileServiceContext context,
                                       BRFileService fs,
                                       uint8_t *bytes,
                                       uint32_t bytesCount);

private_extern void *
cryptoFileServiceTypeTransferV2Reader (BRFileServiceContext context,
                                       BRFileService fs,
                                       uint8_t *bytes,
                                       uint32_t bytesCount);

private_extern uint8_t *
cryptoFileServiceTypeTransferV1Writer (BRFileServiceContext context,
                                 BRFileService fs,
                                 const void* entity,
                                 uint32_t *bytesCount);


#define CRYPTO_FILE_SERVICE_TYPE_TRANSACTION      "crypto_transactions"

typedef enum {
    CRYPTO_FILE_SERVICE_TYPE_TRANSACTION_VERSION_1
} BRCryptoFileServiceTransactionVersion;

private_extern UInt256
cryptoFileServiceTypeTransactionV1Identifier (BRFileServiceContext context,
                                        BRFileService fs,
                                        const void *entity);

private_extern void *
cryptoFileServiceTypeTransactionV1Reader (BRFileServiceContext context,
                                    BRFileService fs,
                                    uint8_t *bytes,
                                    uint32_t bytesCount);

private_extern uint8_t *
cryptoFileServiceTypeTransactionV1Writer (BRFileServiceContext context,
                                    BRFileService fs,
                                    const void* entity,
                                    uint32_t *bytesCount);

extern BRFileServiceTypeSpecification cryptoFileServiceSpecifications[];
extern size_t cryptoFileServiceSpecificationsCount;

#endif /* BRCryptoFileService_h */
