//  BRJson
//  WalletKitCore
//
//  Created by Ed Gamble on 9/28/21.
//  Copyright © 2021 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "support/json/BRJson.h"
#include "yajl/yajl_parse.h"
#include <errno.h>
#include <ctype.h>

#define STATUS_CONTINUE 1
#define STATUS_ABORT    0

// MARK: - Parse Stack

#define CONTEXT_ERROR_BUFFER_LENGTH (1024)
#define CONTEXT_DEFAULT_STATE_SIZE  (100)

typedef enum {
    JSON_CONTEXT_VAL,
    JSON_CONTEXT_KEY,
    JSON_CONTEXT_ARRAY,
    JSON_CONTEXT_OBJECT
} BRJsonStackType;

typedef struct BRJsonStackRecord *BRJsonStack;

struct BRJsonStackRecord {
    BRJsonStackType type;
    union {
        char *key;
        BRArrayOf(BRJson) array;
        BRArrayOf(BRJsonObjectMember) object;
    } u;
    BRJsonStack parent;
};

static BRJsonStack
jsonStackCreateInternal (BRJsonStackType type) {
    BRJsonStack stack = calloc (1, sizeof (struct BRJsonStackRecord));

    stack->type   = type;
    stack->parent = NULL;

    return stack;
}

static BRJsonStack
jsonStackCreateKey (OwnershipGiven char *key) {
    BRJsonStack stack = jsonStackCreateInternal(JSON_CONTEXT_KEY);
    stack->u.key = key;
    return stack;
}

static BRJsonStack
jsonStackCreateArray (void) {
    BRJsonStack stack = jsonStackCreateInternal(JSON_CONTEXT_ARRAY);
    array_new (stack->u.array, 10);
    return stack;
}

static BRJsonStack
jsonStackCreateObject (void) {
    BRJsonStack stack = jsonStackCreateInternal(JSON_CONTEXT_OBJECT);
    array_new (stack->u.object, 10);
    return stack;
}

static void
jsonStackRelease (BRJsonStack stack) {
    switch (stack->type) {
        case JSON_CONTEXT_VAL:
            break;

        case JSON_CONTEXT_KEY:
            free (stack->u.key);
            break;

        case JSON_CONTEXT_ARRAY:
            array_free (stack->u.array);
            break;

        case JSON_CONTEXT_OBJECT:
            for (size_t index = 0; index < array_count (stack->u.object); index++)
                free (stack->u.object[index].label);
            array_free (stack->u.object);
            break;
    }
}

// MARK: - Parse Context

typedef struct {
    char *errbuf;
    size_t errbuf_size;

    BRJson json;
    BRJsonStack stack;
} BRJsonContext;

static BRJsonContext
jsonContextInit (void) {
    BRJsonContext context;

    context.errbuf = calloc (CONTEXT_ERROR_BUFFER_LENGTH + 1, 1);
    context.errbuf_size = CONTEXT_ERROR_BUFFER_LENGTH;
    context.json  = NULL;
    context.stack = NULL;
    return context;
}

static BRJsonStack
jsonContextPush (BRJsonContext *context, BRJsonStack stack) {
    stack->parent  = context->stack;
    context->stack = stack;
    return stack;
}

static BRJsonStack
jsonContextPop (BRJsonContext *context) {
    BRJsonStack result = context->stack;
    context->stack = result->parent;
    result->parent = NULL;
    return result;
}

static void
jsonContextRelease (BRJsonContext *context, bool error) {
    assert (NULL == context->json);
    assert (error || NULL == context->stack);

    if (error) while (context->stack) jsonStackRelease(jsonContextPop(context));
    free (context->errbuf);
    memset (context, 0, sizeof (BRJsonContext));
}

static void
jsonContextUpdate (BRJsonContext *context, BRJson value) {
    BRJsonStack stack = context->stack;

    if (NULL == stack) context->json = value;
    else
        switch (stack->type) {
            case JSON_CONTEXT_VAL:
                break;

            case JSON_CONTEXT_KEY: {
                BRJsonStack object = stack->parent;
                assert (JSON_CONTEXT_OBJECT == object->type);
                array_add (object->u.object, ((BRJsonObjectMember) { strdup (stack->u.key), value }));
                jsonStackRelease(stack);
                jsonContextPop(context);
                break;
            }

            case JSON_CONTEXT_ARRAY:
                array_add (stack->u.array, value);
                break;

            case JSON_CONTEXT_OBJECT:
                assert (false);
                break;
        }
}

// MARK: - Parse Handlers

static int jsonParseHandleNull (void *ctx) {
    BRJsonContext *context = ctx;
    jsonContextUpdate (context, jsonCreateNull());
    return STATUS_CONTINUE;
}

static int jsonParseHandleBoolean (void *ctx, int boolean_value) {
    BRJsonContext *context = ctx;
    jsonContextUpdate (context, jsonCreateBoolean(1 == boolean_value));
    return STATUS_CONTINUE;
}

static int jsonParseHandleString (void *ctx, const unsigned char *string, size_t string_length) {
    BRJsonContext *context = ctx;

    char *str = strndup ((const char*) string, string_length);
    jsonContextUpdate (context, jsonCreateString(str));
    free (str);

    return STATUS_CONTINUE;
}

static int jsonParseHandleNumber (void *ctx, const char *string, size_t string_length) {
    assert (NULL != string);

    BRJsonContext *context = ctx;

    bool success  = true;
    bool negative = false;
    char *str  = strndup ((char *) string, string_length);
    char *strToFree = str;
    char *strEnd;

    // Try to parse a small integer
    errno = 0;
    long long numberLL = strtoll (str, &strEnd, 0);
    if ('\0' == *strEnd && !errno) {
        jsonContextUpdate (context, jsonCreateInteger (uint256Create((uint64_t) llabs(numberLL)), numberLL < 0));
        goto done;
    }

    // Try to parse a big integer
    while (NULL != str && isspace(*str)) str++;
    if ('-' == *str) { str++; negative = true;  }
    if ('+' == *str) { str++; negative = false; }

    BRCoreParseStatus status;
    UInt256 number = uint256CreateParse (str, 10, &status);
    if (CORE_PARSE_OK == status) {
        jsonContextUpdate (context, jsonCreateInteger (number, negative));
        goto done;
    }

    // Try to parse a double
    errno = 0;
    double numberD = strtod (str, &strEnd);
    if ('\0' == *strEnd && !errno) {
        jsonContextUpdate (context, jsonCreateReal(numberD));
        goto done;
    }

    // Failed
    success = false;

done:
    free (strToFree);
    return success ? STATUS_CONTINUE : STATUS_ABORT;
}

// MARK: - Handle Object

static int jsonParseHandleObjectBeg (void *ctx) {
    BRJsonContext *context = ctx;
    jsonContextPush (context, jsonStackCreateObject());
    return STATUS_CONTINUE;
}

static int jsonParseHandleObjectKey (void *ctx, const unsigned char *key, size_t key_length) {
    BRJsonContext *context = ctx;
    jsonContextPush (context, jsonStackCreateKey (strndup ((const char*) key, key_length)));
    return STATUS_CONTINUE;
}


static int jsonParseHandleObjectEnd (void *ctx) {
    BRJsonContext *context = ctx;
    BRJsonStack    stack   = jsonContextPop (context);
    assert (JSON_CONTEXT_OBJECT == stack->type);

    BRJsonStatus status;
    BRJson json = jsonCreateObject (&status, stack->u.object);
    jsonContextUpdate (context, json);

    return STATUS_CONTINUE;
}

// MAEK: - Handler Array

static int jsonParseHandleArrayBeg (void *ctx) {
    BRJsonContext *context = ctx;
    jsonContextPush (context, jsonStackCreateArray());
    return STATUS_CONTINUE;
}

static int jsonParseHandleArrayEnd (void *ctx) {
    BRJsonContext *context = ctx;
    BRJsonStack    stack   = jsonContextPop (context);
    assert (JSON_CONTEXT_ARRAY == stack->type);

    BRJsonStatus status;
    BRJson json = jsonCreateArray (&status, stack->u.array);
    jsonContextUpdate (context, json);

    return STATUS_CONTINUE;
}

// MARK: - Parse

extern BRJson
jsonParse (const char *input,
           BRJsonStatus *jsonStatus,
           char **jsonError) {
    static const yajl_callbacks callbacks =
    {
        /* null        = */ jsonParseHandleNull,
        /* boolean     = */ jsonParseHandleBoolean,
        /* integer     = */ NULL, // jsonParseHandleInteger,
        /* double      = */ NULL, // jsonParseHandleDouble,
        /* number      = */ jsonParseHandleNumber,
        /* string      = */ jsonParseHandleString,
        /* start map   = */ jsonParseHandleObjectBeg,
        /* map key     = */ jsonParseHandleObjectKey,
        /* end map     = */ jsonParseHandleObjectEnd,
        /* start array = */ jsonParseHandleArrayBeg,
        /* end array   = */ jsonParseHandleArrayEnd,
    };

    yajl_handle handle;
    yajl_status status;
    char * internal_err_str;

    BRJsonContext context = jsonContextInit();

    handle = yajl_alloc (&callbacks, NULL, &context);
    yajl_config(handle, yajl_allow_comments, 1);

    status = yajl_parse (handle, (unsigned char *) input, strlen (input));
    status = yajl_complete_parse (handle);

    if (status != yajl_status_ok) {
        if (context.errbuf != NULL && context.errbuf_size > 0) {
            internal_err_str = (char *) yajl_get_error(handle, 1,
                                                       (const unsigned char *) input,
                                                       strlen(input));
            snprintf(context.errbuf, context.errbuf_size, "%s", internal_err_str);
            free (internal_err_str);
        }

        if (NULL != context.json) jsonRelease(context.json);
        context.json = NULL;
    }
    yajl_free (handle);

    if (NULL != jsonStatus) *jsonStatus = (status == yajl_status_ok ? JSON_STATUS_OK : JSON_STATUS_ERROR_PARSE);
    if (NULL != jsonError)  *jsonError  = (status == yajl_status_ok ? NULL           : strdup (context.errbuf));

    BRJson json = context.json;
    context.json = NULL;
    jsonContextRelease (&context, status != yajl_status_ok);

    return json;
}

