//
//  BRJson
//  WalletKitCore
//
//  Created by Ed Gamble on 9/20/20201.
//  Copyright © 2021 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BR_JSON_H
#define BR_JSON_H

#include <stdbool.h>
#include <stdio.h>
#include "support/BRInt.h"
#include "support/BRArray.h"
#include "support/util/BRUtil.h"            // uint256Create

#if !defined (OwnershipGiven)
#define OwnershipGiven
#endif

#if !defined (OwnershipKept)
#define OwnershipKept
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 */
typedef struct BRJsonRecord   *BRJson;

/**
 *
 */
typedef struct {
    char *label;
    BRJson value;
} BRJsonObjectMember;

extern void
jsonObjectMembersSort (BRArrayOf(BRJsonObjectMember) members);

// MARK: - JSON Status

typedef enum {
    JSON_STATUS_OK,
    JSON_STATUS_ERROR_MEMORY_LEAK,
    JSON_STATUS_ERROR_ARGUMENTS,
    JSON_STATUS_ERROR_IN_USE,
    
    JSON_STATUS_ERROR_NO_ARRAY_FOR_INDEX,
    JSON_STATUS_ERROR_NO_INDEX,
    
    JSON_STATUS_ERROR_NO_OBJECT_FOR_LABEL,
    JSON_STATUS_ERROR_NO_LABEL,

    JSON_STATUS_ERROR_PARSE,
    JSON_STATUS_ERROR_PARSE_NUMERIC,
    JSON_STATUS_ERROR_PARSE_INTERNAL,
    
    JSON_STATUS_ERROR_INTERNAL_ERROR
} BRJsonStatus;

// MARK: - JSON Value

typedef enum {
    JSON_TYPE_NULL,
    JSON_TYPE_BOOLEAN,
    JSON_TYPE_NUMBER,
    JSON_TYPE_STRING,
    JSON_TYPE_ARRAY,
    JSON_TYPE_OBJECT
} BRJsonType;

extern BRJsonType
jsonGetType (BRJson value);

static inline bool
jsonHasType (BRJson value, BRJsonType type) {
    return type == jsonGetType(value);
}

extern BRJsonStatus
jsonRelease (OwnershipGiven BRJson value);

extern OwnershipGiven BRJson
jsonClone (BRJson value);

extern BRJson
jsonRoot (BRJson value);

// MARK: - JSON Value Create

extern OwnershipGiven BRJson
jsonCreateNull ();

extern bool
jsonExtractNull (BRJson value);

extern OwnershipGiven BRJson
jsonCreateBoolean (bool boolean);

extern OwnershipGiven BRJson
jsonCreateString (const char *string);

extern OwnershipGiven BRJson
jsonCreateInteger (UInt256 integer,
                   bool negative);

extern OwnershipGiven BRJson
jsonCreateReal (double real);

extern OwnershipGiven BRJson
jsonCreateArray (BRJsonStatus *status,
                 BRArrayOf(OwnershipGiven BRJson) array);

extern OwnershipGiven BRJson
jsonCreateArrayVargs (BRJsonStatus *status,
                      size_t count,
                      /* OwnershipGiven BRJSONValue */
                      ...);

extern OwnershipGiven BRJson
jsonCreateObject (BRJsonStatus *status,
                  BRArrayOf(BRJsonObjectMember) members);

extern OwnershipGiven BRJson
jsonCreateObjectVargs (BRJsonStatus *status,
                       size_t membersCount,
                       /* BRJSONObjectMember */
                       ...);

// MARK: - JSON Value Extract

extern bool
jsonExtractBoolean (BRJson value,
                    bool *boolean);

extern bool
jsonExtractString (BRJson value,
                   const char **string);

extern bool
jsonExtractInteger (BRJson value,
                    UInt256 *integer,
                    bool    *negative);

extern bool
jsonExtractReal (BRJson value,
                 double *real);

extern bool
jsonExtractArray (BRJson value,
                  OwnershipGiven BRArrayOf (OwnershipKept BRJson) *values);

extern bool
jsonExtractObject (BRJson value,
                   OwnershipGiven BRArrayOf (BRJsonObjectMember) *members);

// MARK: - JSON Path

typedef enum {
    JSON_PATH_TYPE_INDEX,
    JSON_PATH_TYPE_LABEL
} BRJsonPathType;

typedef struct {
    BRJsonPathType type;
    union {
        size_t index;
        const char *label;
    } u;
} BRJsonPath;

static inline BRJsonPath
jsonPathCreateIndex (size_t index) {
    return ((BRJsonPath) {
        JSON_PATH_TYPE_INDEX,
        { .index = index }
    });
}

static inline BRJsonPath
jsonPathCreateLabel (const char *label) {
    return ((BRJsonPath) {
        JSON_PATH_TYPE_LABEL,
        { .label = label }
    });
}

extern OwnershipKept BRJson
jsonGetValue (BRJson   value,
              BRJsonStatus *status,
              size_t count,
              /* BRJSONPath */
              ...);

extern OwnershipKept BRJson
jsonGetValueArray (BRJson   value,
                   BRJsonPath   *paths,
                   size_t        pathsCount,
                   BRJsonStatus *status);


// MARK: - Write / Show / Parse

extern void
jsonWrite (BRJson value,
           FILE *file);
extern void
jsonWritePretty (BRJson value,
                 FILE *file,
                 size_t indentation,
                 const char *linePrefix);

static inline void
jsonShow (BRJson value,
          const char *linePrefix) {
    jsonWritePretty (value, stdout, 0, (NULL == linePrefix ? "" : linePrefix));
}

extern BRJson
jsonParse (const char *string,
           BRJsonStatus *status,
           char **error);

#ifdef __cplusplus
}
#endif

#endif //BR_RLP_Coder_H
