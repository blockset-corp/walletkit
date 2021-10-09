//
//  BREthereumStructure .h
//  WalletKitCore
//
//  Created by Ed Gamble on 9/16/2021.
//  Copyright © 2021 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BR_Ethereum_Structure_H
#define BR_Ethereum_Structure_H

#include <stdbool.h>
#include "BREthereumLogic.h"
#include "BREthereumHash.h"
#include "BREthereumData.h"
#include "BREthereumSignature.h"
#include "support/BRArray.h"
#include "support/json/BRJson.h"

#if !defined (OwnershipKept)
#define OwnershipKept
#endif

#if !defined (OwnershipGiven)
#define OwnershipGiven
#endif

#if !defined (Nullable)
#define Nullable
#endif

#if !defined(private_extern)
#define private_extern  extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BREthereumStructureCoderRecord *BREthereumStructureCoder;

#define ETHEREUM_STRUCTURE_ENCODING_BYTES_COUNT         (32)

typedef struct {
    uint8_t bytes[ETHEREUM_STRUCTURE_ENCODING_BYTES_COUNT];
} BREthereumStructureEncoding;

#define ETHEREUM_STRUCTURE_ENCODING_EMPTY_INIT   ((const BREthereumStructureEncoding) { \
0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0, \
0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0  \
})

typedef struct {
    BREthereumData message;
    BREthereumHash digest;
    BREthereumSignature signature;
} BREthereumStructureSignResult;


#if defined (ORGINAL_STRUCTURE_TYPES)
typedef struct BREthereumStructureTypeRecord *BREthereumStructureType;

typedef struct BREthereumStructureRecord *BREthereumStructure;

extern BREthereumStructureCoder
ethStructureCoderCreate (void);

extern void
ethStructureCoderRelease (BREthereumStructureCoder coder);

//extern BREthereumStructureErrorType
//ethStructureCoderValidateType (BREthereumStructureCoder coder,
//                               const char *memberType);

extern bool
ethStructureCoderHasType (BREthereumStructureCoder coder,
                          const char *memberType);

extern void // BREthereumStructureErrorType
ethStructureCoderAddType (BREthereumStructureCoder coder,
                          const char *name,
                          BRArrayOf(const char *) memberNames,
                          BRArrayOf(const char *) memberTypes);
#endif

typedef enum {
    ETHEREUM_STRUCTURE_ERROR_INVALID_ATOMIC_TYPE,

    ETHEREUM_STRUCTURE_ERROR_MISSED_PRIMARY_TYPE,
    ETHEREUM_STRUCTURE_ERROR_MISSED_DOMAIN,
    ETHEREUM_STRUCTURE_ERROR_MISSED_MESSAGE,
    ETHEREUM_STRUCTURE_ERROR_MISSED_TYPES,
    ETHEREUM_STRUCTURE_ERROR_MISSED_DOMAIN_TYPE,

    ETHEREUM_STRUCTURE_ERROR_UNKNOWN_PRIMARY_TYPE,

    ETHEREUM_STRUCTURE_ERROR_INVALID_TYPES_VALUE,
    ETHEREUM_STRUCTURE_ERROR_INVALID_DOMAIN_VALUE,
    ETHEREUM_STRUCTURE_ERROR_INVALID_MESSAGE_VALUE,
    // ...
} BREthereumStructureErrorType;

extern BREthereumStructureCoder
ethStructureCoderCreateFromTypedData (BRJson typedData,
                                      BREthereumStructureErrorType *error);

extern void
ethStructureCoderRelease (BREthereumStructureCoder coder);

// MARK: - Types

extern char *
ethStructureEncodeType (BREthereumStructureCoder coder,
                        const char *name);

extern BREthereumHash
ethStructureHashType (BREthereumStructureCoder coder,
                      const char *name);

// MARK: - Domain

extern BREthereumData
ethStructureEncodeDomain (BREthereumStructureCoder coder);

extern BREthereumHash
ethStructureHashDomain (BREthereumStructureCoder coder);

// MARK: - Data
extern BREthereumData
ethStructureEncodeData (BREthereumStructureCoder coder);

extern BREthereumHash
ethStructureHashData (BREthereumStructureCoder coder);

extern BREthereumStructureSignResult
ethStructureSignData (BREthereumSignatureType type,
                      BREthereumStructureCoder coder,
                      BRKey privateKey);

#ifdef __cplusplus
}
#endif

#endif /* BR_Ethereum_Structure_H */
