//
//  BREthereumStructure.c
//  WalletKitCore
//
//  Created by Ed Gamble on 9/16/2021.
//  Copyright © 2021 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "BREthereumStructure.h"
#include "BREthereumAddress.h"

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include "support/BRBase.h"
#include "support/BRArray.h"
#include "support/BRSet.h"
#include "support/BRData.h"
#include "support/util/BRUtil.h"
#include "support/rlp/BRRlp.h"


// MARK: - String Set

#if !defined(MIN)
#define MIN(a,b)   ((a) <= (b) ? (a) : (b))
#endif

static size_t
stringSetHash (const void *obj) {
    const char *string = (const char *) obj;

    size_t result = 0;;
    memcpy (&result, string, MIN (sizeof(size_t), strlen(string)));

    return result;
}

static int
stringSetEqual (const void *obj1, const void *obj2) {
    const char *string1 = (const char *) obj1;
    const char *string2 = (const char *) obj2;

    return 0 == strcmp (string1, string2);
}

static int
stringCompareHelper (const void *obj1, const void *obj2) {
    const char *string1 = *(const char **) obj1;
    const char *string2 = *(const char **) obj2;

    return strcmp (string1, string2);
}

static BRSetOf(char *)
stringSetCreate (size_t capacity) {
    return BRSetNew (stringSetHash, stringSetEqual, capacity);
}

// MARK: - Support

static OwnershipGiven BREthereumData
ethHashAsData (BREthereumHash hash) {
    BREthereumData data = { ETHEREUM_HASH_BYTES, malloc (ETHEREUM_HASH_BYTES) };
    memcpy (data.bytes, hash.bytes, ETHEREUM_HASH_BYTES);
    return data;
}

#if defined (ORGINAL_STRUCTURE_TYPES)

// MARK: - Structure Member Type

typedef enum {
    // Atomic Types: "The atomic types are bytes1 to bytes32, uint8 to uint256, int8 to int256,
    // bool and address."
    ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_ATOMIC_BYTE,
    ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_ATOMIC_UINT,
    ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_ATOMIC_XINT,
    ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_ATOMIC_BOOL,
    ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_ATOMIC_ADDRESS,

    // Dynamic Types: "The dynamic types are bytes and string. These are like the atomic types
    // for the purposed of type declaration, but their treatment in encoding is different."
    ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_DYNAMIC_BYTES,
    ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_DYNAMIC_STRING,

    // Reference Types: "The reference types are arrays and structs. Arrays are either fixed size
    // or dynamic and denoted by Type[n] or Type[] respectively. Structs are references to other
    // structs by their name. The standard supports recursive struct types."
    ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_REFERENCE_ARRAY,
    ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_REFERENCE_STRUCT,
} BREthereumStructureMemberTypeLabel;

static const char *
ethStructureMemberTypeLabelName (BREthereumStructureMemberTypeLabel label) {
    static const char *names[] = {
        "byte",
        "uint",
        "int",
        "bool",
        "address",
        "bytes",
        "string",
        "array",        // unused
        "struct"        // unused
    };
    return names[label];
}

///
/// Member Type: "A member type can be either an atomic type, a dynamic type or a reference type."
///
typedef struct {
    BREthereumStructureMemberTypeLabel label;
    union {
        // Atomic Types
        struct { uint16_t bits; } byte;
        struct { uint16_t bits; } uint;
        struct { uint16_t bits; } xint;

        // Dynamic Types

        // ReferenceTypes

        /// "Structs are references to other structs by their name"
        struct { const char *name; } xstruct;

        /// "Arrays are either fixed size or dynamic and denoted by Type[n] or Type[] respectively"
        struct { const char *name; ssize_t count; } array;
    } u;
    char *encoding;
} BREthereumStructureMemberType;

static BREthereumStructureMemberType
ethStructureMemberTypeCreate (BREthereumStructureMemberTypeLabel label) {
    switch (label) {
        case ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_ATOMIC_BOOL:
        case ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_ATOMIC_ADDRESS:
        case ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_DYNAMIC_BYTES:
        case ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_DYNAMIC_STRING:
            return (BREthereumStructureMemberType) {
                label,
                { .array = { NULL, 0 }},   // zero out large union element
                NULL
            };

        default:
            assert (false);
            return (BREthereumStructureMemberType) { 0 };
    }
}

static BREthereumStructureMemberType
ethStructureMemberTypeCreateBits (BREthereumStructureMemberTypeLabel label, uint16_t bits) {
    switch (label) {
        case ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_ATOMIC_BYTE:
            assert (bits <= 32);
            return (BREthereumStructureMemberType) {
                ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_ATOMIC_BYTE,
                { .byte = { bits }},
                NULL
            };

        case ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_ATOMIC_UINT:
            assert (bits <= 256);
            return (BREthereumStructureMemberType) {
                ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_ATOMIC_BYTE,
                { .uint = { bits }},
                NULL
            };

        case ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_ATOMIC_XINT:
            assert (bits <= 256);
            return (BREthereumStructureMemberType) {
                ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_ATOMIC_BYTE,
                { .xint = { bits }},
                NULL
            };

        default:
            assert (false);
            break;
    }
}

static BREthereumStructureMemberType
ethStructureMemberTypeCreateArray (const char *name, ssize_t count) {
    return (BREthereumStructureMemberType) {
        ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_REFERENCE_STRUCT,
        { .array = { name, count }},
        NULL
    };
}

static BREthereumStructureMemberType
ethStructureMemberTypeCreateStruct (const char *name) {
    return (BREthereumStructureMemberType) {
        ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_REFERENCE_STRUCT,
        { .xstruct = { name }},
        NULL
    };
}

static void
ethStructureMemberTypeInitializeEncodimg (BREthereumStructureMemberType *type) {
    const char *labelName = ethStructureMemberTypeLabelName (type->label);
    switch (type->label) {
        case ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_ATOMIC_BYTE:
            asprintf (&type->encoding, "%s%d", labelName, type->u.byte.bits);
            break;

        case ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_ATOMIC_UINT:
            asprintf (&type->encoding, "%s%d", labelName, type->u.uint.bits);
            break;

        case ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_ATOMIC_XINT:
            asprintf (&type->encoding, "%s%d", labelName, type->u.xint.bits);
            break;

        case ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_ATOMIC_BOOL:
        case ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_ATOMIC_ADDRESS:
        case ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_DYNAMIC_BYTES:
        case ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_DYNAMIC_STRING:
            type->encoding = strdup (labelName);
            break;

        case ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_REFERENCE_ARRAY:
            if (-1 == type->u.array.count)
                asprintf (&type->encoding, "%s[]", type->u.array.name);
            else
                asprintf (&type->encoding, "%s[%zu]", type->u.array.name, (size_t) type->u.array.count);
            break;

        case ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_REFERENCE_STRUCT:
            type->encoding = strdup (type->u.xstruct.name);
            break;

        default:
            assert (false);
    }
}

static const char*
ethStructureMemberTypeGetEncoding (BREthereumStructureMemberType *type) {
    if (NULL == type->encoding) ethStructureMemberTypeInitializeEncodimg (type);
    return type->encoding;
}

// MARK: Structure Member Variable

// MemberVariable: "Member variables have a member type and a name"
typedef struct BREthereumStructureMemberVariableRecord {
    const char *name;
    BREthereumStructureMemberType type;
    char *encoding;
} *BREthereumStructureMemberVariable;

static BREthereumStructureMemberVariable
ethStructureMemberVariableCreate (const char *name, const BREthereumStructureMemberType type) {
    BREthereumStructureMemberVariable variable = calloc (1, sizeof (struct BREthereumStructureMemberVariableRecord));
    variable->name = name;
    variable->type = type;     // clone
    variable->encoding = NULL;
    return variable;
}

static void
ethStructureMemberVariableInitializeEncoding (BREthereumStructureMemberVariable variable) {
    asprintf (&variable->encoding, "%s %s",
              ethStructureMemberTypeGetEncoding (&variable->type),
              variable->name);
}

static const char *
ethStructureMemberVariableGetEncoding (BREthereumStructureMemberVariable variable) {
    if (NULL == variable->encoding) ethStructureMemberVariableInitializeEncoding (variable);
    return variable->encoding;
}

// MARK: - Structure Type

/// Structure Type: "A struct type has valid identifier as name and contains zero or more
/// member variables"
struct BREthereumStructureTypeRecord {
    char *name;
    BRArrayOf(BREthereumStructureMemberVariable) variables;
    char *encoding;
};

static BREthereumStructureType
ethStructureTypeCreate (const char *name,
                        OwnershipGiven BRArrayOf(BREthereumStructureMemberVariable) variables) {
    BREthereumStructureType type = malloc (sizeof (struct BREthereumStructureTypeRecord));
    type->name = strdup (name);
    type->variables = variables;
    type->encoding  = NULL;
    return type;
}

static void
ethStructureTypeRelease (BREthereumStructureType type) {
    free (type->name);

    // ....

    if (type->encoding) free (type->encoding);

    memset (type, 0, sizeof (struct BREthereumStructureTypeRecord));
    free (type);
}

static int
ethStructureTypeCompare (BREthereumStructureType type1,
                         BREthereumStructureType types2) {
    return strcmp (type1->name, types2->name);
}

static int
ethStructureTypeCompareHelper (const void *v1, const void *v2) {
    const BREthereumStructureType type1 = (const BREthereumStructureType) v1;
    const BREthereumStructureType type2 = (const BREthereumStructureType) v2;
    return ethStructureTypeCompare (type1, type2);
}


static bool // true if added
ethStructureTypeArrayAddIfUnknown (BRArrayOf(BREthereumStructureType) types,
                                   BREthereumStructureType type) {
    for (size_t index = 0; index < array_count(types); index++)
        if (0 == strcmp (type->name, types[index]->name))
            return false;
    array_add (types, type);
    return true;
}

static bool /// true if removed
ethStructureTypeArrayRemIfKnown (BRArrayOf(BREthereumStructureType) types,
                                   BREthereumStructureType type) {
    for (size_t index = 0; index < array_count(types); index++)
        if (0 == strcmp (type->name, types[index]->name)) {
            array_rm (types, index);
            return true;
        }
    return false;
}

static BREthereumStructureType
ethStructureTypeArrayLookupByName (BRArrayOf(BREthereumStructureType) types, const char *name) {
    for (size_t index = 0; index < array_count(types); index++)
        if (0 == strcmp (name, types[index]->name))
            return types[index];
    return NULL;
}

static void
ethStructureTypeFindDependents (BREthereumStructureType type,
                                BRArrayOf(BREthereumStructureType) typesDependent,
                                BRArrayOf(BREthereumStructureType) typesAllKnown) {
    assert (NULL != type);

    if (ethStructureTypeArrayAddIfUnknown (typesDependent, type)) {
        for (size_t index = 0; index < array_count(type->variables); index++) {
            BREthereumStructureMemberType memberType = type->variables[index]->type;
            switch (memberType.label) {
                case ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_REFERENCE_ARRAY:
                    ethStructureTypeFindDependents (ethStructureTypeArrayLookupByName (typesAllKnown, memberType.u.array.name),
                                                    typesDependent,
                                                    typesAllKnown);
                    break;

                case ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_REFERENCE_STRUCT:
                    ethStructureTypeFindDependents (ethStructureTypeArrayLookupByName (typesAllKnown, memberType.u.xstruct.name),
                                                    typesDependent,
                                                    typesAllKnown);
                    break;

                default:
                    break;
            }
        }
    }
}

static BRData
ethStructureTypeEncodeOne (BREthereumStructureType type) {
    char *encoding = NULL;

    if (0 == array_count(type->variables))
        asprintf (&encoding, "%s()", type->name);
    else {
        size_t variableCount = array_count(type->variables);

        BRArrayOf(const char *) variableEncodings;
        array_new (variableEncodings, variableCount);

        size_t variableEncodingsLength = 0;
        for (size_t index = 0; index < variableCount; index++) {
            array_add (variableEncodings, ethStructureMemberVariableGetEncoding (type->variables[index]));
            variableEncodingsLength += strlen (variableEncodings[index]);
        }

        char *variableEncoding = malloc (variableEncodingsLength + 1);
        variableEncoding[0] = variableEncoding[variableEncodingsLength] = '\0';

        for (size_t index = 0; index < variableCount; index++) {
            strcat (variableEncoding, variableEncodings[index]);
            if (index + 1 < variableCount)
                strcat (variableEncoding, ",");
        }
        array_free (variableEncodings);

        asprintf (&encoding, "%s(%s)", type->name, variableEncoding);
    }

    return ((BRData) { (uint8_t*) encoding, strlen(encoding) });
}

static void
ethStructureTypeInitializeEncoding (BREthereumStructureType type,
                            BRArrayOf(BREthereumStructureType) typesAllKnown) {
    BRData encodingSelf = ethStructureTypeEncodeOne(type);

    BRArrayOf (BRData) encodings;
    array_new (encodings, 10);
    array_add (encodings, encodingSelf);

    BRArrayOf(BREthereumStructureType) typesReferenced;
    array_new (typesReferenced, 10);

    // "If the struct type references other struct types (and these in turn reference even more
    // struct types), then the set of referenced struct types is collected, sorted by name and
    // appended to the encoding. An example encoding is Transaction(Person from,Person to,Asset tx)
    // Asset(address token,uint256 amount)Person(address wallet,string name)."

    ethStructureTypeFindDependents  (type, typesReferenced, typesAllKnown);
    ethStructureTypeArrayRemIfKnown (typesReferenced, type);

    mergesort_brd (typesReferenced, array_count(typesReferenced), sizeof(BREthereumStructureType), ethStructureTypeCompareHelper);

    for (size_t index = 0; index < array_count(typesReferenced); index++) {
        array_add (encodings, ethStructureTypeEncodeOne (typesReferenced[index]));
    }

    BRData encoding = dataConcat (encodings, array_count(encodings));

    array_free_all (encodings, dataFree);

    type->encoding = malloc (encoding.size + 1);
    memcpy (type->encoding, encoding.bytes, encoding.size);
    type->encoding[encoding.size] = '\0';

}

static const char *
ethStructureTypeGetEncoding (BREthereumStructureType type,
                             BRArrayOf(BREthereumStructureType) typesAllKnown) {
    if (NULL == type->encoding) ethStructureTypeInitializeEncoding (type, typesAllKnown);
    return type->encoding;
}

static BREthereumHash
ethStructureTypeHashX (BREthereumStructureType type,
                      BRArrayOf(BREthereumStructureType) typesAllKnown) {
    return ethHashCreate (ethStructureTypeGetEncoding (type, typesAllKnown));
}

// MARK: - Structure Member Value

typedef struct BREthereumStructureMemberValueRecord *BREthereumStructureMemberValue;

struct BREthereumStructureMemberValueRecord {
    ///
    BREthereumStructureMemberTypeLabel label;

    /// The 'Member Variable' name
    const char *name;

    union {
        // Atomic Type Values

        // Boolean false and true are encoded as uint256 values 0 and 1 respectively
        // Integer values are sign-extended to 256-bit and encoded in big endian order.
        UInt256 value;

        // Addresses are encoded as uint160
        BREthereumAddress address;

        // Dynamic Type Values

        BRData bytes;
        char *string;

        // Reference Type Values

        // Array
        BRArrayOf (BREthereumStructureMemberValue) array;

        // struct
        BREthereumStructure xstruct;

    } u;

    // "Each encoded member value is exactly 32-byte long." "The dynamic values bytes and string
    // are encoded as a keccak256 hash of their contents." "The array values are encoded as the
    // keccak256 hash of the concatenated"
    BREthereumHash encoding;
} ;

static BREthereumStructureMemberValue
ethStructureMemberValueCreateInternal (BREthereumStructureMemberTypeLabel label,
                                       const char *name) {
    BREthereumStructureMemberValue value = calloc (1, sizeof (struct BREthereumStructureMemberValueRecord));
    value->label    = label;
    value->name     = name;
    value->encoding = ethHashCreateEmpty();
    return value;
}

static BREthereumStructureMemberValue
ethStructureMemberValueCreateByte (uint8_t *bytes, size_t bytesCount, const char *name) {
    assert (bytesCount <= 32);
    BREthereumStructureMemberValue value = ethStructureMemberValueCreateInternal (ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_ATOMIC_BYTE, name);
    // "bytes1 to bytes31 are arrays with a beginning (index 0) and an end (index length - 1), they
    // are zero-padded at the end to bytes32 and encoded in beginning to end order. This corresponds
    // to their encoding in ABI v1 and v2."
    value->u.value = UINT256_ZERO;  // 'zero-pad'
    memcpy (value->u.value.u32, bytes, bytesCount);     // reversed?

    return value;
}

// uint
// xint

static BREthereumStructureMemberValue
ethStructureMemberValueCreateBoolean (bool boolean, const char *name) {
    BREthereumStructureMemberValue value = ethStructureMemberValueCreateInternal (ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_ATOMIC_BOOL, name);
    value->u.value = uint256Create(boolean ? 1 : 0);
    return value;
}

static BREthereumStructureMemberValue
ethStructureMemberValueCreateAddress (BREthereumAddress address, const char *name) {
    BREthereumStructureMemberValue value = ethStructureMemberValueCreateInternal (ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_ATOMIC_ADDRESS, name);
    value->u.address = address;
    return value;
}

static BREthereumStructureMemberValue
ethStructureMemberValueCreateBytes (BRData bytes, const char *name) {
    BREthereumStructureMemberValue value = ethStructureMemberValueCreateInternal (ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_DYNAMIC_BYTES, name);
    value->u.bytes = dataClone(bytes);
    return value;
}

static BREthereumStructureMemberValue
ethStructureMemberValueCreateString (const char *string, const char *name) {
    BREthereumStructureMemberValue value = ethStructureMemberValueCreateInternal (ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_DYNAMIC_STRING, name);
    value->u.string = strdup (string);
    return value;
}

// array

// struct

static void
ethStructureMemberValueRelease (BREthereumStructureMemberValue value) {
    switch (value->label) {
        case ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_DYNAMIC_BYTES:
            dataFree (value->u.bytes);

        case ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_DYNAMIC_STRING:
            free (value->u.string);
            break;

        case ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_REFERENCE_ARRAY:
            // ...
            break;

        case ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_REFERENCE_STRUCT:
            // ...
            break;

        default:
            break;
    }

    memset (value, 0, sizeof (struct BREthereumStructureMemberValueRecord));
    free (value);
}

// Forward Declaration
static BREthereumHash ethStructureMemberValueGetEncoding (BREthereumStructureMemberValue value);

static void
ethStructureMemberValueInitializeEncoding (BREthereumStructureMemberValue value) {
    switch (value->label) {
        case ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_ATOMIC_BYTE:
        case ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_ATOMIC_UINT:
        case ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_ATOMIC_XINT:
        case ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_ATOMIC_BOOL:
            memcpy (value->encoding.bytes, value->u.value.u32, 32);
            break;

        case ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_ATOMIC_ADDRESS:
            value->encoding = ethHashCreateEmpty();
            memcpy (&value->encoding.bytes[12], value->u.address.bytes, 20);
            break;

            // Dynamc Types: "The dynamic values bytes and string are encoded as a keccak256 hash
            // of their contents."
        case ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_DYNAMIC_BYTES:
            BRKeccak256 (value->encoding.bytes, value->u.bytes.bytes, value->u.bytes.size);
            break;

        case ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_DYNAMIC_STRING:
            BRKeccak256 (value->encoding.bytes, value->u.string, strlen(value->u.string));
            break;

            // Reference Types

        case ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_REFERENCE_ARRAY: {
            // "The array values are encoded as the keccak256 hash of the concatenated encodeData
            // of their contents"
            size_t   encodingsCount = array_count (value->u.array);
            uint8_t *encodingsBytes = calloc (encodingsCount, sizeof (BREthereumHash));

            for (size_t index = 0; index < encodingsCount; index++) {
                BREthereumHash encoding = ethStructureMemberValueGetEncoding (value->u.array[index]);
                memcpy (&encodingsBytes[index * sizeof(BREthereumHash)], encoding.bytes, sizeof(BREthereumHash));
            }

            BRKeccak256 (value->encoding.bytes, encodingsBytes, encodingsCount * sizeof (BREthereumHash));
            break;
        }

        case ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_REFERENCE_STRUCT:

            break;
    }
}

static BREthereumHash
ethStructureMemberValueGetEncoding (BREthereumStructureMemberValue value) {
    if (ethHashEqual (value->encoding, ETHEREUM_EMPTY_HASH_INIT))
        ethStructureMemberValueInitializeEncoding (value);
    return value->encoding;
}


// MARK: - Structure

struct BREthereumStructureRecord {
    BREthereumStructureType type;
    BRArrayOf(BREthereumStructureMemberValue) values;
};

static BREthereumHash
ethStructureHash (BREthereumStructure data) {

}

// MARK: - Structure Coder

struct BREthereumStructureCoderRecord {
    BRArrayOf (BREthereumStructureType) types;
    BRArrayOf (BREthereumStructure) values;

};

extern BREthereumStructureCoder
ethStructureCoderCreate (void) {
    BREthereumStructureCoder coder = malloc (sizeof (struct BREthereumStructureCoderRecord));

    array_new (coder->types, 10);
    array_new (coder->types, 10);

    // Add the EIP712Domain

    return coder;
}

extern void
ethStructureCoderRelease (BREthereumStructureCoder coder) {
    array_free (coder->types);
    array_free (coder->values);

    memset (coder, 0, sizeof (struct BREthereumStructureCoderRecord));
    free (coder);
}

static bool
ethStructureMemberTypeIsAtomicTypeWithSize (const char *label) {

}

static bool
ethStructureMemberTypeIsAtomicType (const char *label) {
    return (0 == strcmp (label, ethStructureMemberTypeLabelName (ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_ATOMIC_BOOL)) ||
            0 == strcmp (label, ethStructureMemberTypeLabelName (ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_ATOMIC_ADDRESS)) ||
            ethStructureMemberTypeIsAtomicTypeWithSize(label));
}

static bool
ethStructureMemberTypeIsDynamicType (const char *label) {
    return (0 == strcmp (label, ethStructureMemberTypeLabelName (ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_DYNAMIC_BYTES)) ||
            0 == strcmp (label, ethStructureMemberTypeLabelName (ETHEREUM_STRUCTURE_MEMBER_TYPE_LABEL_DYNAMIC_STRING)));
}

static bool
ethStructureMemberTypeIsReferenceType (const char *label,
                                       BRArrayOf (BREthereumStructureType) types) {
    for (size_t index = 0; index < array_count(types); index++) {
        if (0 == strcmp (label, types[index]->name))
            return true;
    }
    return false;
}

extern bool
ethStructureCoderHasType (BREthereumStructureCoder coder,
                          const char *memberType) {
    return (ethStructureMemberTypeIsAtomicType (memberType) ||
            ethStructureMemberTypeIsDynamicType (memberType) ||
            ethStructureMemberTypeIsReferenceType (memberType, coder->types));
}

extern void
ethStructureCoderAddType (BREthereumStructureCoder coder,
                          const char *name,
                          BRArrayOf(const char *) memberNames,
                          BRArrayOf(const char *) memberTypes) {  // Foo, Foo[], Foo[n]

}

extern void
ethStructureCoderAddValue (BREthereumStructureCoder coder,
                           )

#endif // defined (ORGINAL_STRUCTURE_TYPES)

struct BREthereumStructureCoderRecord {
    BRJson typedData;

    const char *primaryTypeName;

    BRJson types;
    BRJson domain;
    BRJson message;

//    BRArrayOf (BREthereumStructureType) types;
//    BRArrayOf (BREthereumStructure) values;
};

// MARK: - Atomic Type
/**
 * Check the `count` is a valid number of bits for a an integer (`int` or `uint`)
 */
static bool
ethConfirmAtomicTypeIntegerSize (int count) {
    return (  8 == count ||
             16 == count ||
             32 == count ||
             64 == count ||
            128 == count ||
            256 == count);
}

/**
 * Check if `typeName` is a valid 'atomic type'
 */
static bool
ethConfirmTypeNameIsAtomicType (const char *typeName) {
    int count = 0;

    return  (0 == strcmp (typeName, "address") ||
             0 == strcmp (typeName, "bool")    ||
             (1 == sscanf (typeName, "byte%d", &count) && 1 <= count && count <= 32) ||
             (1 == sscanf (typeName, "uint%d", &count) && ethConfirmAtomicTypeIntegerSize (count)) ||
             (1 == sscanf (typeName,  "int%d", &count) && ethConfirmAtomicTypeIntegerSize (count)));
}

static bool
ethConfirmValueIsAtomicType (BRJson value, const char *typeName) {
    int count = 0;

    if (0 == strcmp (typeName, "address")) {
        const char *addressString;
        if (!jsonExtractString (value, &addressString)) return false;
        return (ETHEREUM_BOOLEAN_FALSE ==
                ethAddressEqual (ethAddressCreate(addressString), ETHEREUM_EMPTY_ADDRESS_INIT));
    }

    if (0 == strcmp (typeName, "bool")) {
        return jsonExtractBoolean (value, NULL);
    }

    if (1 == sscanf (typeName, "byte%d", &count) && 1 <= count && count <= 32) {
        return true;
    }

    if (1 == sscanf (typeName, "uint%d", &count) && ethConfirmAtomicTypeIntegerSize (count)) {
        return true;
    }

    if (1 == sscanf (typeName,  "int%d", &count) && ethConfirmAtomicTypeIntegerSize (count)) {
        return true;
    }

    return true;
}

static const char *
ethEncodeTypeAsAtomicType (const char *typeName) {
    return typeName;
}

static BREthereumData
ethEncodeValueAsAtomicType (BRJson value, const char *typeName) {
    BREthereumData data = { 32, calloc (1, 32) };

    int count = 0;

    if (0 == strcmp (typeName, "address")) {
        const char *addressString = NULL;
        jsonExtractString (value, &addressString);
        assert (NULL != addressString);

        BREthereumAddress address = ethAddressCreate(addressString);

        memcpy (&data.bytes[12], address.bytes, 20);
        return data;
    }

    // Boolean false and true are encoded as uint256 values 0 and 1 respectively.
    if (0 == strcmp (typeName, "bool")) {
        bool boolean;
        jsonExtractBoolean (value, &boolean);

        UInt256 integer = uint256Create(boolean ? 1 : 0);
        integer = UInt256Reverse(integer);

        memcpy (data.bytes, integer.u8, 32);
        return data;
    }

    // bytes1 to bytes31 are arrays with a beginning (index 0) and an end (index length - 1), they
    // are zero-padded at the end to bytes32 and encoded in beginning to end order.
    if (1 == sscanf (typeName, "byte%d", &count) && 1 <= count && count <= 32) {
        assert (false);
    }

    // Integer values are sign-extended to 256-bit and encoded in big endian order.
    if (1 == sscanf (typeName, "uint%d", &count) && ethConfirmAtomicTypeIntegerSize (count)) {
        UInt256 integer;
        jsonExtractInteger (value, &integer, NULL);
        integer = UInt256Reverse(integer);

        memcpy (data.bytes, integer.u8, 32);
        return data;
    }

    // Integer values are sign-extended to 256-bit and encoded in big endian order.
    if (1 == sscanf (typeName,  "int%d", &count) && ethConfirmAtomicTypeIntegerSize (count)) {
        UInt256 integer;
        bool negative;
        jsonExtractInteger (value, &integer, &negative);

        // What next?
        assert (false);
        integer = UInt256Reverse(integer);
    }

    return ((BREthereumData) { 0, NULL });
}

// MARK: - Dynamic Type

/**
 * Chekc if `typeName` is a valid dynamic type
 */
static bool
ethConfirmTypeNameIsDynamicType (const char *typeName) {
    return (0 == strcmp (typeName, "string") ||
            0 == strcmp (typeName, "bytes"));
}

static bool
ethConfirmValueIsDynamicType (BRJson value,
                              const char *typeName) {
    if (0 == strcmp (typeName, "string")) {
        return jsonExtractString (value, NULL);
    }

    if (0 == strcmp (typeName, "bytes")) {
        return true;
    }

    return false;
}

static const char *
ethEncodeTypeAsDynamicType (const char *typeName) {
    return typeName;
}

/// The dynamic values bytes and string are encoded as a keccak256 hash of their contents.
static BREthereumData
ethEncodeValueAsDynamicType (BRJson value,
                               const char *typeName) {
    BREthereumHash hash = ETHEREUM_EMPTY_HASH_INIT;

    if (0 == strcmp (typeName, "string")) {
        const char *valueString;
        jsonExtractString (value, &valueString);
        BRRlpData data = { strlen (valueString), (uint8_t*) valueString };
        hash = ethHashCreateFromData(data);
    }

    if (0 == strcmp (typeName, "bytes")) {
        const char *valueString;
        jsonExtractString (value, &valueString);

        size_t bytesCount;
        uint8_t *bytes = hexDecodeCreate (&bytesCount, valueString, strlen(valueString));

        BRRlpData data = { bytesCount, bytes };
        hash = ethHashCreateFromData(data);

        free (bytes);
    }

    return ethHashAsData(hash);
}

// MARK: - Structure Type

// Forward Declaration
static bool ethConfirmTypeName (BRJson types, const char *typeName);
static BREthereumData ethEncodeValue (BRJson value, BRJson types, const char *typeName, bool recursive);
static bool ethExtractTypeNameAsReferenceType (BRJson types,
                                               const char *typeName,
                                               char **type,
                                               bool *array,
                                               int *arrayCount);
static BREthereumHash
ethHashTypeAsStructureType (BRJson type,
                            const char *typeName,
                            BRJson types);
/**
 * Check if `type` is a valid structure type.  To be valid, `type` must be a known type and be
 * composed as an array of {name,type} 'member' objects.  The member name must be a string; the
 * member type must be valid.
 *
 * A valid structure type looks like:
 *
 * "Mail":[
 *    { "name":"from",     "type":"Person" },
 *    { "name":"to",       "type":"Person" },
 *    { "name":"contents", "type":"string" }
 * ]
 *
 */
static bool
ethConfirmTypeAsStructureType (BRJson type,
                BRJson types) {
    bool success = true;

    // Confirm `type` as an array of {name,type} objects
    BRArrayOf (BRJson) typeMembers;

    // The `type` must be an array
    if (!jsonExtractArray (type, &typeMembers)) return false;

    // Each of `typeMembers` must:
    for (size_t index = 0; index < array_count(typeMembers); index++) {
        BRJson typeMember = typeMembers[index];

        // 1) have `name` as a 'string'
        BRJson typeNameValue = jsonGetValue (typeMember, NULL, 1, jsonPathCreateLabel("name"));
        if (NULL == typeNameValue ||
            !jsonExtractString (typeNameValue, NULL)) {
            success = false;
            break;
        }

        // 2) have the `name` as a `string` not be a duplicate
        // TBD (second in array will be ignored...)

        // 3) have `type` as a 'string' and the value must be known (validity checked elsewhere)
        const char *typeTypeName;
        BRJson typeTypeValue = jsonGetValue (typeMember, NULL, 1, jsonPathCreateLabel("type"));
        if (NULL == typeTypeValue ||
            !jsonExtractString (typeTypeValue, &typeTypeName) ||
            !ethConfirmTypeName (types, typeTypeName)) {
            success = false;
            break;
        }
    }
    array_free (typeMembers);

    return success;
}

static bool
ethExtractTypeNameAndType (BRJson type,
                           const char **typeName,
                           const char **typeType) {
    // Get the "name" and "type" members of type.
    BRJson typeNameValue = jsonGetValue (type, NULL, 1, jsonPathCreateLabel("name"));
    BRJson typeTypeValue = jsonGetValue (type, NULL, 1, jsonPathCreateLabel("type"));

    bool success = (NULL != typeNameValue &&
                    jsonExtractString (typeNameValue, typeName) &&
                    NULL != typeTypeValue &&
                    jsonExtractString (typeTypeValue, typeType));

    // If not success, assign both `typeName` and `typeType` to NULL.
    if (!success) {
        if (NULL != typeName) *typeName = NULL;
        if (NULL != typeType) *typeType = NULL;
    }

    return success;
}

/**
 * Check if `value` is a valid instance of `type`.  A value is valid if the value has a valid member
 * for each of the type's members.  Validity is determined recursively if `type` is itself composed
 * of other types.
 */
static bool
ethConfirmValueIsStructureType (BRJson value,
                       BRJson type,
                       BRJson types) {
    bool success = true;
    BRJsonStatus status;

    // Confirm `value` as an object
    if (!jsonExtractObject (value, NULL)) return false;

    // Confirm that `value` has members defined in `type`
    BRArrayOf (BRJson) typeMembers;
    if (!jsonExtractArray (type, &typeMembers)) return false;

    // `value` is an JSONObject
    for (size_t index = 0; index < array_count(typeMembers); index++) {
        BRJson member = typeMembers[index];

        const char *memberName;
        const char *memberType;
        if (!ethExtractTypeNameAndType (member, &memberName, &memberType)) {
            success = false;
            break;
        }

        // Confirm `value` has `memberName`
        BRJson memberValue = jsonGetValue (value, &status, 1, jsonPathCreateLabel(memberName));
        if (NULL == memberValue) {
            success = false;
            break;
        }

        if (ethConfirmTypeNameIsAtomicType(memberType))
            success = ethConfirmValueIsAtomicType (memberValue, memberType);

        else if (ethConfirmTypeNameIsDynamicType (memberType))
            success = ethConfirmValueIsDynamicType (memberValue, memberType);

        else {
            BRJson memberTypeValue = jsonGetValue (types, NULL, 1, jsonPathCreateLabel(memberType));
            success = ethConfirmValueIsStructureType (memberValue, memberTypeValue, types);
        }

        if (!success) break;
    }
    array_free (typeMembers);

    return success;
}

static char *
ethEncodeTypeAsStructureTypeOne (BRJson type,
                                 const char *typeName) {
    //
    // "The type of a struct is encoded as name ‖ "(" ‖ member₁ ‖ "," ‖ member₂ ‖ "," ‖ … ‖ memberₙ ")"
    // where each member is written as type ‖ " " ‖ name. For example, the above Mail struct is"
    // encoded as Mail(address from,address to,string contents).
    //

    BRArrayOf (BRJson) typeMembers;
    jsonExtractArray (type, &typeMembers);
    size_t typeMembersCount = array_count (typeMembers);

    BRArrayOf(char *) memberEncodings;
    array_new (memberEncodings, typeMembersCount);
    array_set_count (memberEncodings, typeMembersCount);

    size_t memberEncodingsSize = 0;
    for (size_t index = 0; index < typeMembersCount; index++) {
        BRJson member = typeMembers[index];

        const char *memberName;
        const char *memberType;
        ethExtractTypeNameAndType (member, &memberName, &memberType);

        asprintf(&memberEncodings[index], "%s %s,", memberType, memberName);

        // If this is the last member, remove the trailing ','
        if (index + 1 == typeMembersCount) {
            char *encoding = memberEncodings[index];
            encoding[strlen(encoding) - 1] = '\0';
        }

        memberEncodingsSize += strlen (memberEncodings[index]);
    }

    char *memberEncodingsString = malloc (memberEncodingsSize + 1);
    memberEncodingsString[0] = '\0';

    for (size_t index = 0; index < typeMembersCount; index++)
        strcat (memberEncodingsString, memberEncodings[index]);

    char *result;
    asprintf(&result, "%s(%s)", typeName, memberEncodingsString);

    free (memberEncodingsString);
    array_free_all (memberEncodings, free);

    return result;
}

static void
ethEncodeTypeAsStructureTypeFindDependents (BRJson types,
                                            const char *typeName,
                                            BRSetOf(char *) typeNames) {
    // Check if `typeName` is unknown
    if (NULL == BRSetAdd (typeNames, (void*) typeName)) {
        BRJson type = jsonGetValue (types, NULL, 1, jsonPathCreateLabel(typeName));

        BRArrayOf (BRJson) typeMembers;
        jsonExtractArray (type, &typeMembers);
        size_t typeMembersCount = array_count (typeMembers);

        for (size_t index = 0; index < typeMembersCount; index++) {
            BRJson member = typeMembers[index];

            const char *memberName;
            const char *memberType;
            ethExtractTypeNameAndType (member, &memberName, &memberType);

            char *memberTypeName;
            bool array;
            int  arrayCount;

            if (ethExtractTypeNameAsReferenceType (types, memberType, &memberTypeName, &array, &arrayCount))
                ethEncodeTypeAsStructureTypeFindDependents (types, memberTypeName, typeNames);
        }
    }
}

static char *
ethEncodeTypeAsStructureType (BRJson type,
                              const char *typeName,
                              BRJson types) {

    //
    // If the struct type references other struct types (and these in turn reference even more
    // struct types), then the set of referenced struct types is collected, sorted by name and
    // appended to the encoding. An example encoding is:
    //     Transaction(Person from,Person to,Asset tx)Asset(address token,uint256 amount)Person(address wallet,string name)
    //

    BRSetOf(char*) typeNamesSet = stringSetCreate (10);
    assert (type == jsonGetValue (types, NULL, 1, jsonPathCreateLabel(typeName)));

    ethEncodeTypeAsStructureTypeFindDependents (types, typeName, typeNamesSet);

    // Remove typeName from typeNamesSet
    BRSetRemove (typeNamesSet, typeName);

    // Extract then sort `typeNames`
    size_t typeNamesCount = BRSetCount(typeNamesSet);
    BRArrayOf(char *) typeNamesArray;
    array_new (typeNamesArray, typeNamesCount);
    array_set_count (typeNamesArray, typeNamesCount);
    BRSetAll  (typeNamesSet, (void**) typeNamesArray, typeNamesCount);
    BRSetFree (typeNamesSet);
    mergesort_brd (typeNamesArray, typeNamesCount, sizeof(char*), stringCompareHelper);

    // Put `typeName` right up front.
    array_insert (typeNamesArray, 0, (char*) typeName);
    size_t typeEncodingsCount = array_count (typeNamesArray);

    BRArrayOf(char *) typeEncodings;
    array_new (typeEncodings, typeEncodingsCount);
    array_set_count (typeEncodings, typeEncodingsCount);

    size_t encodingsSize = 0;
    for (size_t index = 0; index < typeEncodingsCount; index++) {
        typeEncodings[index] = ethEncodeTypeAsStructureTypeOne (jsonGetValue (types, NULL, 1, jsonPathCreateLabel(typeNamesArray[index])),
                                                                typeNamesArray[index]);
        encodingsSize += strlen (typeEncodings[index]);
    }
    array_free (typeNamesArray);

    char *encoding = malloc (encodingsSize + 1);
    encoding[0] = '\0';

    for (size_t index = 0; index < typeEncodingsCount; index++)
        strcat (encoding, typeEncodings[index]);
    array_free_all(typeEncodings, free);

    return encoding;
}

static BREthereumHash
ethHashTypeAsStructureType (BRJson type,
                            const char *typeName,
                            BRJson types) {
    char *typeEncodingString = ethEncodeTypeAsStructureType (type, typeName, types);
    BREthereumHash hash = ethHashCreateFromBytes ((uint8_t *) typeEncodingString, strlen (typeEncodingString));
    free (typeEncodingString);
    return hash;
}


static BREthereumData
ethEncodeValueAsStructureType (BRJson value,
                               BRJson types,
                               BRJson type,
                               const char *typeName) {
    // The encoding of a struct instance is enc(value₁) ‖ enc(value₂) ‖ … ‖ enc(valueₙ), i.e. the
    // concatenation of the encoded member values in the order that they appear in the type. Each
    // encoded member value is exactly 32-byte long.

    BRArrayOf (BRJson) typeMembers;
    jsonExtractArray (type, &typeMembers);

    BRArrayOf(BREthereumData) encodings;
    array_new (encodings, 5);

//    BREthereumStructureEncoding  encoding;
//    BREthereumStructureEncoding *encodings = calloc (1 + array_count(typeMembers), sizeof (BREthereumStructureEncoding));


    // Prefix the encoding with the 'type hash'
    // TODO: Is this in the 'spec' or just in the example implementation of the spec
    array_add (encodings, ethHashAsData (ethHashTypeAsStructureType (type, typeName, types)));

    for (size_t index = 0; index < array_count(typeMembers); index++) {
        BRJson member = typeMembers[index];

        const char *memberName;
        const char *memberType;
        ethExtractTypeNameAndType (member, &memberName, &memberType);

        BRJson memberValue = jsonGetValue (value, NULL, 1, jsonPathCreateLabel(memberName));

        array_add (encodings, ethEncodeValue (memberValue, types, memberType, true));
    }
    array_free (typeMembers);

    BREthereumData result = ethDataConcat (encodings, array_count(encodings));
    array_free_all (encodings, ethDataRelease);

    return result;
}

// MARK: - Reference Type

/**
 * arrayCount is assigned -1 on "Type[]"
 */
static bool
ethExtractTypeNameAsReferenceType (BRJson types,
                                   const char *typeName,
                                   char **type,
                                   bool *array,
                                   int *arrayCount) {
    assert (NULL != type && NULL != array && NULL != arrayCount);
    bool success = false;

    *type  = NULL;
    *array = false;
    *arrayCount = 0;

    // If there is no `typeName`, return `false`
    if (NULL == typeName || '\0' == typeName[0]) return false; // do not 'goto done;'

    // Give use something to modify in the subsequent `strsep()` calls.
    char *typeNameToParse = strdup(typeName);
    char *typeNameToFree  = typeNameToParse;

    // Parse each of: "Type", "Type[]" and 'Type[<n>]"
    *type = strsep (&typeNameToParse, "[");

    // Handle: "[..."
    if (NULL == *type || '\0' == (*type)[0]) { success = false; goto done; }

    // Handle: "Type"
    if (NULL == typeNameToParse || '\0' == typeNameToParse[0]) { success = true; goto done; }

    // Get `count` as "...[count]"
    char *count = strsep (&typeNameToParse, "]");

    // Handle: "...[<n>]<chars>" as failure
    if ('\0' != typeNameToParse[0]) { success = false; goto done; }

    // Handle: "...[]"
    if (count == typeNameToParse) { success = true; *array = true; *arrayCount = -1; goto done; }

    char *countEnd;
    *arrayCount = (int) strtol (count, &countEnd, 10);

    // Handle: "...[<n>...]"
    if (NULL == countEnd || '\0' != countEnd[0]) { success = false; goto done; }

    // Handle: a non-positive count
    if (*arrayCount <= 0) { success = false; goto done; }

    *array = true;

done:
    // Confirm as a reference types
    success &= (NULL != *type) && jsonGetValue (types, NULL, 1, jsonPathCreateLabel(*type));

    *type = (!success ? NULL : strdup (*type));
    free (typeNameToFree);
    return success;
}

static bool
ethConfirmTypeNameIsReferenceType (BRJson types,
                                   const char *typeName) {

    char *type;
    bool array;
    int  arrayCount;

    if (!ethExtractTypeNameAsReferenceType (types, typeName, &type, &array, &arrayCount))
        return false;

    BRJson typeValue = jsonGetValue (types, NULL, 1, jsonPathCreateLabel(type));
    if (NULL == typeValue)
        return false;

    // ...
    return true;
}

static bool
ethConfirmValueIsReferenceType (BRJson value,
                                BRJson types,
                                const char *typeDecl) {
    char *typeName;
    bool array;
    int  arrayCount;

    // Confirm that `typeName` parses properly
    if (!ethExtractTypeNameAsReferenceType (types, typeDecl, &typeName, &array, &arrayCount))
        return false;

    // Confirm `type` exists in `types`
    BRJson type = jsonGetValue (types, NULL, 1, jsonPathCreateLabel(typeName));
    if (NULL == type)
        return false;

    // If `!array`, confirm `value` as a structure type
    if (!array) return ethConfirmValueIsStructureType (value, type, types);

    // Confirm `value` as an array
    BRArrayOf(BRJson) values;
    if (!jsonExtractArray (value, &values)) return false;

    // Confirm the number of `values`
    if (-1 != arrayCount && arrayCount != array_count(values)) {
        array_free (values);
        return false;
    }

    // Confirm each of `values` as `type`
    for (size_t index = 0; index < array_count(values); index++) {
        if (!ethConfirmValueIsStructureType (values[index], type, types)) {
            array_free (value);
            return false;
        }
    }

    array_free (values);
    return true;
}

static char *
ethEncodeTypeAsReferenceType (BRJson types,
                              const char *typeDecl) {
    char *typeName;
    bool array;
    int  arrayCount;

    // This has already been confirmd

    ethExtractTypeNameAsReferenceType (types, typeDecl, &typeName, &array, &arrayCount);

    if (!array) {
        BRJson type = jsonGetValue (types, NULL, 1, jsonPathCreateLabel(typeName));
        return ethEncodeTypeAsStructureType (type, typeName, types);
    }
    else return strdup (typeDecl);
}

static BREthereumData
ethEncodeValueAsReferenceType (BRJson value,
                               BRJson types,
                               const char *typeDecl,
                               bool recursive) {
    BREthereumData encoding;

    char *typeName;
    bool array;
    int  arrayCount;

    ethExtractTypeNameAsReferenceType (types, typeDecl, &typeName, &array, &arrayCount);

    if (!array) {
        BRJson type = jsonGetValue (types, NULL, 1, jsonPathCreateLabel(typeName));
        encoding = ethEncodeValueAsStructureType (value, types, type, typeName);
    }

    else {

        // "The array values are encoded as the keccak256 hash of the concatenated encodeData of
        // their contents (i.e. the encoding of SomeType[5] is identical to that of a struct
        // containing five members of type SomeType)."
        BRArrayOf(BRJson) values;
        jsonExtractArray (value, &values);

        BRArrayOf(BREthereumData) encodings;
        array_new (encodings, array_count(values));

        for (size_t index = 0; index < array_count(values); index++)
            array_add (encodings, ethEncodeValue (values[index], types, typeName, true));

        encoding = ethDataConcat (encodings, array_count(encodings));
        array_free_all (encodings, ethDataRelease);
    }

    if (!recursive) return encoding;
    else {
        BREthereumData result = ethHashAsData (ethHashCreateFromBytes (encoding.bytes, encoding.count));
        ethDataRelease(encoding);
        return result;
    }
}

// MARK: - [Any|All] Type

/**
 * Check if `typeName` is a 'structure type' (in `types), a 'reference type' or an 'atomic type'.
 */
static bool
ethConfirmTypeName (BRJson types,
                    const char *typeName) {
    // Confirm `typeName` is:
    return (//   1) an atomic type
            ethConfirmTypeNameIsAtomicType (typeName) ||
            //   2) A dynamic type
            ethConfirmTypeNameIsDynamicType (typeName) ||
            //   3) A reference type
            ethConfirmTypeNameIsReferenceType(types, typeName));
}

static bool
ethConfirmValue (BRJson value,
                 BRJson types,
                 const char *typeName) {
    if (ethConfirmTypeNameIsAtomicType (typeName))
        return ethConfirmValueIsAtomicType (value, typeName);

    if (ethConfirmTypeNameIsDynamicType(typeName))
        return ethConfirmValueIsDynamicType (value, typeName);

    if (ethConfirmTypeNameIsReferenceType (types, typeName))
        return ethConfirmValueIsReferenceType (value, types, typeName);

    return false;
}

static char *
ethEncodeType (BRJson types,
               const char *typeName) {
    if (ethConfirmTypeNameIsAtomicType (typeName))
        return strdup (ethEncodeTypeAsAtomicType (typeName));

    if (ethConfirmTypeNameIsDynamicType(typeName))
        return strdup (ethEncodeTypeAsDynamicType (typeName));

    if (ethConfirmTypeNameIsReferenceType (types, typeName))
        return ethEncodeTypeAsReferenceType (types, typeName);

    return NULL;
}

static BREthereumData
ethEncodeValue (BRJson value,
                BRJson types,
                const char *typeName,
                bool recursive) {
    if (ethConfirmTypeNameIsAtomicType (typeName))
        return ethEncodeValueAsAtomicType (value, typeName);

    if (ethConfirmTypeNameIsDynamicType(typeName))
        return ethEncodeValueAsDynamicType (value, typeName);

    if (ethConfirmTypeNameIsReferenceType (types, typeName))
        return ethEncodeValueAsReferenceType (value, types, typeName, recursive);

    return ((BREthereumData) { 0, NULL });
}

// MARK: - Create

/**
 * Convenience function to assign an error type.
 */
static BREthereumStructureCoder
ethReturnCoderAndAssignStatus (BREthereumStructureCoder coder,
                               BREthereumStructureErrorType *errorRef,
                               BREthereumStructureErrorType  error) {
    if (NULL == coder && NULL != errorRef) *errorRef = error;
    return coder;
}

/**
 * Create a BREthereumStructureCoder instance from JSON typedData.  Checks the validity of the
 * `typedData`; the it is invalid the NULL is returned and `error` is assigned.
 */
extern BREthereumStructureCoder
ethStructureCoderCreateFromTypedData (BRJson typedData,
                                      BREthereumStructureErrorType *error) {
    BRJsonStatus status;

    //
    // Confirm the `typedData` structure as follows:
    //
    //   1) confirm `types` exists
    BRJson typesValue = jsonGetValue (typedData, &status, 1, jsonPathCreateLabel("types"));
    if (NULL == typesValue)
        return ethReturnCoderAndAssignStatus (NULL, error, ETHEREUM_STRUCTURE_ERROR_MISSED_TYPES);

    //   2) confirm all the types
    BRArrayOf(BRJsonObjectMember) typeMembers;
    if (!jsonExtractObject (typesValue, &typeMembers))
        return ethReturnCoderAndAssignStatus (NULL, error, ETHEREUM_STRUCTURE_ERROR_INVALID_TYPES_VALUE);
    for (size_t index = 0; index < array_count(typeMembers); index++) {
        BRJsonObjectMember member = typeMembers[index];

        if (!ethConfirmTypeAsStructureType (member.value, typesValue)) {
            array_free (typeMembers);
            return ethReturnCoderAndAssignStatus (NULL, error, ETHEREUM_STRUCTURE_ERROR_INVALID_TYPES_VALUE);
        }
    }
    array_free (typeMembers);

    //   3) confirm the existence of the `EIP712Domain`
    BRJson domainTypeValue = jsonGetValue (typesValue, &status, 1, jsonPathCreateLabel("EIP712Domain"));
    if (NULL == domainTypeValue)
        return ethReturnCoderAndAssignStatus (NULL, error, ETHEREUM_STRUCTURE_ERROR_MISSED_DOMAIN_TYPE);

    //   4) confirm `domain` exists
    BRJson domainValue = jsonGetValue (typedData, &status, 1, jsonPathCreateLabel("domain"));
    if (NULL == domainValue)
        return ethReturnCoderAndAssignStatus (NULL, error, ETHEREUM_STRUCTURE_ERROR_MISSED_DOMAIN);

    //   5) confirm `domain` is valid
    if (!ethConfirmValueIsStructureType (domainValue, domainTypeValue, typesValue))
        return ethReturnCoderAndAssignStatus (NULL, error, ETHEREUM_STRUCTURE_ERROR_INVALID_DOMAIN_VALUE);

    //   6) confirm `primaryType` exists
    const char *primaryTypeName;
    BRJson primaryTypeValue = jsonGetValue (typedData, &status, 1, jsonPathCreateLabel("primaryType"));
    if (NULL == primaryTypeValue ||
        !jsonExtractString (primaryTypeValue, &primaryTypeName))
        return ethReturnCoderAndAssignStatus (NULL, error, ETHEREUM_STRUCTURE_ERROR_MISSED_PRIMARY_TYPE);

    //   7) confirm `primaryType` references a type.
    BRJson primaryTypeType = jsonGetValue (typesValue, NULL, 1, jsonPathCreateLabel(primaryTypeName));
    if (NULL == primaryTypeType)
        return ethReturnCoderAndAssignStatus (NULL, error, ETHEREUM_STRUCTURE_ERROR_UNKNOWN_PRIMARY_TYPE);

    //   8) confirm `message` exists
    BRJson messageValue = jsonGetValue (typedData, &status, 1, jsonPathCreateLabel("message"));
    if (NULL == messageValue)
        return ethReturnCoderAndAssignStatus (NULL, error, ETHEREUM_STRUCTURE_ERROR_MISSED_MESSAGE);

    //   9) confirm `message` is a valid as a `primaryType` instance
    if (!ethConfirmValue (messageValue, typesValue, primaryTypeName))
        return ethReturnCoderAndAssignStatus (NULL, error, ETHEREUM_STRUCTURE_ERROR_INVALID_MESSAGE_VALUE);

    //
    // Allocate and Initialize the `coder`
    //
    BREthereumStructureCoder coder = malloc (sizeof (struct BREthereumStructureCoderRecord));

    coder->typedData = typedData;
    coder->primaryTypeName = primaryTypeName;

    coder->types   = typesValue;
    coder->domain  = domainValue;
    coder->message = messageValue;

    return coder;
}

// MARK: - (Structure) Type Encode/Hash

extern char *
ethStructureEncodeType (BREthereumStructureCoder coder,
                        const char *typeName) {
    return (ethConfirmTypeName (coder->types, typeName)
            ? ethEncodeType (coder->types, typeName)
            : NULL);
}

extern BREthereumHash
ethStructureHashType (BREthereumStructureCoder coder,
                      const char *typeName) {
    char *encoding = ethStructureEncodeType (coder, typeName);
    BREthereumHash hash = (NULL != encoding
                           ? ethHashCreateFromBytes ((uint8_t*) encoding, strlen(encoding))
                           : ETHEREUM_EMPTY_HASH_INIT);
    free (encoding);
    return hash;
}

// MARK: - Domain Encode/Hash

extern BREthereumData
ethStructureEncodeDomain (BREthereumStructureCoder coder) {
    return ethEncodeValue (coder->domain, coder->types, "EIP712Domain", false);
}

extern BREthereumHash
ethStructureHashDomain (BREthereumStructureCoder coder) {
    BREthereumData encoding = ethStructureEncodeDomain (coder);
    BREthereumHash hash     = ethHashCreateFromBytes (encoding.bytes, encoding.count);
    ethDataRelease(encoding);
    return hash;
}

// MARK: - Message Encode/Hash

extern BREthereumData
ethStructureEncodeData (BREthereumStructureCoder coder) {
    return ethEncodeValue (coder->message, coder->types, coder->primaryTypeName, false);
}

extern BREthereumHash
ethStructureHashData (BREthereumStructureCoder coder) {
    BREthereumData encoding = ethStructureEncodeData (coder);
    BREthereumHash hash     = ethHashCreateFromBytes (encoding.bytes, encoding.count);
    ethDataRelease(encoding);
    return hash;
}

extern BREthereumStructureSignResult
ethStructureSignData (BREthereumStructureCoder coder,
                      BRKey privateKey) {
    BRKey privateKeyUncompressed = privateKey;
    privateKeyUncompressed.compressed = 0;

    BREthereumHash hashDomain = ethStructureHashDomain (coder);
    BREthereumHash hashData   = ethStructureHashData   (coder);

    size_t bufferSize = 2 + ETHEREUM_HASH_BYTES + ETHEREUM_HASH_BYTES;
    BREthereumData buffer = { bufferSize, malloc (bufferSize)};
    buffer.bytes[0] = 0x19;
    buffer.bytes[1] = 0x01;
    memcpy (&buffer.bytes[2],                       hashDomain.bytes, ETHEREUM_HASH_BYTES);
    memcpy (&buffer.bytes[2 + ETHEREUM_HASH_BYTES], hashData.bytes,   ETHEREUM_HASH_BYTES);

    BREthereumHash      digest;
    BREthereumSignature signature = ethSignatureCreate (SIGNATURE_TYPE_RECOVERABLE_VRS_EIP,
                                                        buffer.bytes,
                                                        buffer.count,
                                                        privateKey,
                                                        &digest);

    return ((BREthereumStructureSignResult) { buffer, digest, signature });
}
