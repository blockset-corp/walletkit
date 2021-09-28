//
//  BRJson
//  WalletKitCore
//
//  Created by Ed Gamble on 9/20/20201.
//  Copyright © 2021 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include <stdarg.h>
#include <stdio.h>
#include <pthread.h>

#include "support/BROSCompat.h"
#include "support/BRSet.h"
#include "support/util/BRUtil.h"

#include "support/json/BRJson.h"

#if !defined(MIN)
#define MIN(a,b)   ((a) <= (b) ? (a) : (b))
#endif

// MARK: - Member

static BRJsonObjectMember *
jsonMemberCreate (const char *label,
                  BRJson value) {
    BRJsonObjectMember *member = malloc (sizeof (BRJsonObjectMember));

    member->label = strdup (label);
    member->value = /* ... */ value;

    return member;
}

static void
jsonMemberRelease (BRJsonObjectMember *member, bool memberOnly) {
    if (!memberOnly) {
        free (member->label);
        // Can release 'value'; no context for function call
        assert (false);
    }

    memset (member, 0, sizeof (BRJsonObjectMember));
    free (member);
}

static size_t
jsonMemberSetHash (const void *obj) {
    const BRJsonObjectMember *member = (const BRJsonObjectMember *) obj;

    size_t result = 0;;
    memcpy (&result, member->label, MIN (sizeof(size_t), strlen(member->label)));

    return result;
}

static int
jsonMemberSetEqual (const void *obj1, const void *obj2) {
    const BRJsonObjectMember *member1 = (const BRJsonObjectMember *) obj1;
    const BRJsonObjectMember *member2 = (const BRJsonObjectMember *) obj2;

    return 0 == strcmp (member1->label, member2->label);
}

static BRSetOf(BRJSONObjectMember*)
jsonMemberSetCreate (size_t capacity) {
    return BRSetNew (jsonMemberSetHash, jsonMemberSetEqual, capacity);
}

static BRJsonObjectMember *
jsonMemberSetLookup (BRSetOf(BRJSONObjectMember*) set,
                     const char *label) {
    BRJsonObjectMember member = { (char *) label, NULL };
    return (BRJsonObjectMember *) BRSetGet (set, &member);
}

static int
jsonMembersCompareHelper (const void *obj1, const void *obj2) {
    BRJsonObjectMember *member1 = *(BRJsonObjectMember **) obj1;
    BRJsonObjectMember *member2 = *(BRJsonObjectMember **) obj2;
    return strcmp (member1->label, member2->label);
}

static void
jsonMembersSort (BRJsonObjectMember *members[], size_t membersCount) {
    mergesort_brd (members, membersCount, sizeof (BRJsonObjectMember*), jsonMembersCompareHelper);
}

static int
jsonObjectMemberCompareHelper (const void *obj1, const void *obj2) {
    BRJsonObjectMember *member1 = (BRJsonObjectMember *) obj1;
    BRJsonObjectMember *member2 = (BRJsonObjectMember *) obj2;
    return strcmp (member1->label, member2->label);
}

extern void
jsonObjectMembersSort (BRArrayOf(BRJsonObjectMember) members) {
    mergesort_brd (members, array_count(members), sizeof (BRJsonObjectMember), jsonObjectMemberCompareHelper);
}

// MARK: - Value

typedef enum {
    JSON_NUMBER_TYPE_POSITIVE_INTEGER,
    JSON_NUMBER_TYPE_NEGATIVE_INTEGER,
    JSON_NUMBER_TYPE_REAL
} JSONNumberType;

struct BRJsonRecord {
    BRJsonType type;
    union {
        /// A null
        // unneeded

        /// A boolean
        bool boolean;

        /// A string
        char *string;

        /// A number
        struct {
            JSONNumberType type;
            union {
                UInt256 integer;
                double  real;
            } u;
        } number;

        /// An array of BRJSONValue
        BRArrayOf (BRJson) array;

        /// A set of BRJSONMember as { label, value }
        BRSetOf (BRJsonObjectMember) object;
    } u;

    //
    BRJson parent;
};

extern BRJsonType
jsonGetType (BRJson value) {
    return value->type;
}

extern BRJson
jsonRoot (BRJson value) {
    return (NULL == value->parent ? value : jsonRoot (value->parent));
}

// MARK: - JSON

static BRJson
jsonCreateInternal (BRJsonType type) {
    BRJson value = calloc (1, sizeof (struct BRJsonRecord));

    value->type = type;

    return value;
}

extern BRJsonStatus
jsonRelease (OwnershipGiven BRJson value) {
    if (NULL != value->parent) return JSON_STATUS_ERROR_IN_USE;

    switch (value->type) {
        case JSON_TYPE_STRING:
            free (value->u.string);
            break;

        case JSON_TYPE_ARRAY:
            for (size_t index = 0; index < array_count (value->u.array); index++)
                jsonRelease (value->u.array[index]);
            array_free (value->u.array);
            value->u.array = NULL;
            break;

        case JSON_TYPE_OBJECT: {
            size_t membersCount = BRSetCount(value->u.object);
            BRJsonObjectMember *members[membersCount];
            BRSetAll  (value->u.object, (void**) members, membersCount);
            BRSetFree (value->u.object);

            for (size_t index = 0; index < membersCount; index++) {
                BRJsonObjectMember *member = members[index];
                free (member->label);
                jsonRelease (member->value);
                jsonMemberRelease(member, true);
            }

            value->u.object = NULL;
            break;
        }

        default:
            // Nothing needed
            break;
    }

    memset (value, 0, sizeof (struct BRJsonRecord));
    free (value);

    return JSON_STATUS_OK;
}

static BRJson
jsonAssignStatus (BRJson value, BRJsonStatus *statusRef, BRJsonStatus status) {
    if (NULL != statusRef) *statusRef = status;
    return value;
}

extern OwnershipGiven BRJson
jsonCreateNull () {
    return jsonCreateInternal (JSON_TYPE_NULL);
}

extern bool
jsonExtractNull (BRJson value) {
    return (JSON_TYPE_NULL == value->type);
}

extern OwnershipGiven BRJson
jsonCreateBoolean (bool boolean) {
    BRJson value = jsonCreateInternal (JSON_TYPE_BOOLEAN);
    value->u.boolean = boolean;
    return value;
}

extern bool
jsonExtractBoolean (BRJson value,
                    bool *boolean) {
    if (JSON_TYPE_BOOLEAN != value->type) return false;

    if (NULL != boolean) *boolean = value->u.boolean;
    return true;
}

extern OwnershipGiven BRJson
jsonCreateString (const char *string) {
    BRJson value = jsonCreateInternal (JSON_TYPE_STRING);
    value->u.string = strdup (string);
    return value;
}

extern bool
jsonExtractString (BRJson value,
                   const char **string) {
    if (JSON_TYPE_STRING != value->type) return false;

    if (NULL != string) *string = value->u.string;
    return true;
}

extern OwnershipGiven BRJson
jsonCreateInteger (UInt256 integer,
                   bool negative) {
    BRJson value = jsonCreateInternal (JSON_TYPE_NUMBER);
    value->u.number.type = (negative ? JSON_NUMBER_TYPE_NEGATIVE_INTEGER : JSON_NUMBER_TYPE_POSITIVE_INTEGER);
    value->u.number.u.integer = integer;
    return value;
}

extern bool
jsonExtractInteger (BRJson value,
                    UInt256 *integer,
                    bool    *negative) {
    if (JSON_TYPE_NUMBER != value->type ||
        (JSON_NUMBER_TYPE_POSITIVE_INTEGER != value->u.number.type &&
         JSON_NUMBER_TYPE_NEGATIVE_INTEGER != value->u.number.type))
        return false;

    if (NULL != integer)  *integer  = value->u.number.u.integer;
    if (NULL != negative) *negative = (JSON_NUMBER_TYPE_NEGATIVE_INTEGER == value->u.number.type);
    return true;
}

extern OwnershipGiven BRJson
jsonCreateReal (double real) {
    BRJson value = jsonCreateInternal (JSON_TYPE_NUMBER);
    value->u.number.type = JSON_NUMBER_TYPE_REAL;
    value->u.number.u.real = real;
    return value;
}

extern bool
jsonExtractReal (BRJson value,
                 double *real) {
    if (JSON_TYPE_NUMBER != value->type || JSON_NUMBER_TYPE_REAL != value->u.number.type) return false;

    if (NULL != real) *real = value->u.number.u.real;
    return true;
}

static BRJsonStatus
jsonValidateValueArray (BRJson *values,
                        size_t valuesCount) {
    for (size_t index = 0; index < valuesCount; index++) {
        BRJson value = values[index];

        if (NULL != value->parent)
            return JSON_STATUS_ERROR_IN_USE;
    }
    return JSON_STATUS_OK;
}

extern OwnershipGiven BRJson
jsonCreateArrayInternal (BRJsonStatus *status,
                         OwnershipGiven BRJson *values,
                         size_t valuesCount) {
    // Ensure NULL == array[index].parent
    *status = jsonValidateValueArray (values, valuesCount);
    if (JSON_STATUS_OK != *status) return NULL;

    BRJson value = jsonCreateInternal (JSON_TYPE_ARRAY);

    array_new (value->u.array, valuesCount);
    for (size_t index = 0; index < valuesCount; index++) {
        BRJson valueAtIndex = values[index];
        array_add (value->u.array, valueAtIndex);
        valueAtIndex->parent = value;
    }

    return value;
}

extern OwnershipGiven BRJson
jsonCreateArray (BRJsonStatus *status,
                 BRArrayOf(OwnershipGiven BRJson) values) {
    return jsonCreateArrayInternal (status, values, array_count(values));
}

extern OwnershipGiven BRJson
jsonCreateArrayVargs (BRJsonStatus *status,
                      size_t valuesCount,
                      /* OwnershipGiven BRJSONValue */
                      ...) {
    BRJson values[valuesCount];

    va_list args;
    va_start (args, valuesCount);
    for (size_t index = 0; index < valuesCount; index++)
        values[index] = va_arg (args, BRJson);
    va_end(args);

    return jsonCreateArrayInternal (status, values, valuesCount);

}

extern bool
jsonExtractArray (BRJson value,
                  OwnershipGiven BRArrayOf (OwnershipKept BRJson) *values) {
    if (JSON_TYPE_ARRAY != value->type) return false;

    if (NULL != values) {
        size_t valuesCount = array_count (value->u.array);
        array_new (*values, valuesCount);
        for (size_t index = 0; index < valuesCount; index++)
            array_add (*values, value->u.array[index]);
        //        array_set_count (*values, valuesCount);
        //        array_add_array (*values, value->u.array, valuesCount);
    }
    return true;
}

static BRJsonStatus
jsonValidatePairArray (BRJsonObjectMember *members,
                       size_t membersCount) {
    for (size_t index = 0; index < membersCount; index++) {
        BRJsonObjectMember *member = &members[index];

        if (NULL == member->value)
            return JSON_STATUS_ERROR_ARGUMENTS;

        if (NULL != member->value->parent)
            return JSON_STATUS_ERROR_IN_USE;
    }
    return JSON_STATUS_OK;
}

extern OwnershipGiven BRJson
jsonCreateObjectInternal (BRJsonStatus *status,
                          BRJsonObjectMember *members,
                          size_t membersCount) {

    // Ensure NULL == array[index].parent
    *status = jsonValidatePairArray (members, membersCount);
    if (JSON_STATUS_OK != *status) return NULL;

    // Ensure no duplicate labels

    BRJson value = jsonCreateInternal (JSON_TYPE_OBJECT);
    value->u.object = jsonMemberSetCreate (membersCount);

    for (size_t index = 0; index < membersCount; index++) {
        BRJsonObjectMember *member = &members[index];

        BRJsonObjectMember *memberForSet = jsonMemberCreate (member->label, member->value);
        memberForSet->value->parent = value;

        BRSetAdd (value->u.object, memberForSet);
    }

    return value;
}

extern OwnershipGiven BRJson
jsonCreateObject (BRJsonStatus *status,
                  BRArrayOf(BRJsonObjectMember) members) {
    return jsonCreateObjectInternal (status, members, array_count(members));
}

extern OwnershipGiven BRJson
jsonCreateObjectVargs (BRJsonStatus *status,
                       size_t membersCount,
                       /* BRJSONObjectMember */
                       ...) {
    BRJsonObjectMember members[membersCount];

    va_list args;
    va_start (args, membersCount);
    for (size_t index = 0; index < membersCount; index++)
        members[index] = va_arg (args, BRJsonObjectMember);
    va_end(args);

    return jsonCreateObjectInternal (status, members, membersCount);
}

extern bool
jsonExtractObject (BRJson value,
                   BRArrayOf(BRJsonObjectMember) *members) {
    if (JSON_TYPE_OBJECT != value->type) return false;

    if (NULL != members) {
        size_t membersCount = BRSetCount (value->u.object);

        array_new (*members, membersCount);
        array_set_count (*members, membersCount);

        BRJsonObjectMember *membersInSet[membersCount];
        BRSetAll (value->u.object, (void**) membersInSet, membersCount);

        for (size_t index = 0; index < membersCount; index++)
            (*members)[index] = *membersInSet[index];
    }
    return true;
}

extern BRJson
jsonGetValueArray (BRJson   value,
                   BRJsonPath   *paths,
                   size_t        pathsCount,
                   BRJsonStatus *status) {
    if (NULL != paths)
        for (size_t index = 0;  index < pathsCount; index++) {
            BRJsonPath pathNext = paths[index];
            switch (pathNext.type) {
                case JSON_PATH_TYPE_INDEX:
                    if (JSON_TYPE_ARRAY != value->type)
                        return jsonAssignStatus (NULL, status, JSON_STATUS_ERROR_NO_ARRAY_FOR_INDEX);

                    if (pathNext.u.index >= array_count (value->u.array))
                        return jsonAssignStatus(NULL, status, JSON_STATUS_ERROR_NO_INDEX);

                    value = value->u.array[pathNext.u.index];
                    break;

                case JSON_PATH_TYPE_LABEL:
                    if (JSON_TYPE_OBJECT != value->type)
                        return jsonAssignStatus (NULL, status, JSON_STATUS_ERROR_NO_OBJECT_FOR_LABEL);


                    BRJsonObjectMember *member = jsonMemberSetLookup (value->u.object, pathNext.u.label);
                    if (NULL == member)
                        return jsonAssignStatus (NULL, status, JSON_STATUS_ERROR_NO_LABEL);

                    value = member->value;
                    break;

                default:
                    return jsonAssignStatus (NULL, status, JSON_STATUS_ERROR_INTERNAL_ERROR);
            }
        }

    return jsonAssignStatus (value, status, JSON_STATUS_OK);
}

extern OwnershipKept BRJson
jsonGetValue (BRJson   value,
              BRJsonStatus *status,
              size_t count,
              /* BRJSONPath */
              ...) {
    BRJsonPath path[count];

    va_list args;
    va_start (args, count);
    for (size_t index = 0; index < count; index++)
        path[index] = va_arg (args, BRJsonPath);
    va_end(args);

    return jsonGetValueArray (value, path, count, status);
}

extern OwnershipGiven BRJson
jsonClone (BRJson value) {
    BRJson clone = jsonCreateInternal (value->type);

    switch (clone->type) {
        case JSON_TYPE_NULL:
            break;

        case JSON_TYPE_BOOLEAN:
            clone->u.boolean = value->u.boolean;
            break;

        case JSON_TYPE_STRING:
            clone->u.string = strdup (clone->u.string);
            break;

        case JSON_TYPE_NUMBER:
            switch (clone->u.number.type) {
                case JSON_NUMBER_TYPE_POSITIVE_INTEGER:
                case JSON_NUMBER_TYPE_NEGATIVE_INTEGER:
                    clone->u.number.u.integer = value->u.number.u.integer;
                    clone->u.number.type      = value->u.number.type;
                    break;

                case JSON_NUMBER_TYPE_REAL:
                    clone->u.number.u.real = value->u.number.u.real;
                    break;
            }
            break;

        case JSON_TYPE_ARRAY: {
            BRArrayOf(BRJson) oldValues = value->u.array;
            BRArrayOf(BRJson) newValues;
            array_new (newValues, array_count(oldValues));
            for (size_t index = 0; index < array_count(oldValues); index++) {
                BRJson newValue = jsonClone (oldValues[index]);
                array_add (newValues, newValue);
                newValue->parent = clone;
            }
            clone->u.array = newValues;
            break;
        }

        case JSON_TYPE_OBJECT: {
            size_t membersCount = BRSetCount(value->u.object);
            BRJsonObjectMember *members[membersCount];
            BRSetAll  (value->u.object, (void**) members, membersCount);

            clone->u.object = jsonMemberSetCreate(membersCount);

            for (size_t index = 0; index < membersCount; index++) {
                BRJsonObjectMember *member = members[index];

                BRJson         newMemberValue = jsonClone (member->value);
                BRJsonObjectMember *newMember      = jsonMemberCreate (member->label, newMemberValue);
                newMemberValue->parent = clone;

                BRSetAdd (clone->u.object, newMember);
            }
            break;
        }
    }

    return clone;
}

// MARK: JSON Write

static void
jsonValueWriteNewline (FILE *file, bool pretty, bool lineRequired, size_t lineIndentation, const char *linePrefix) {
    if (pretty && lineRequired) fprintf (file, "\n%s%*s", linePrefix, (int) lineIndentation, "");
}

static void
jsonValueWriteDetailed (BRJson value,
                        FILE *file,
                        bool pretty,
                        bool   lineRequired,
                        size_t lineIndentation,
                        const char *linePrefix) {
    jsonValueWriteNewline (file, pretty, lineRequired, lineIndentation, linePrefix);
    switch (value->type) {
        case JSON_TYPE_NULL:
            fputs ("null", file);
            break;
        case JSON_TYPE_BOOLEAN:
            fputs ((value->u.boolean ? "true" : "false"), file);
            break;

        case JSON_TYPE_NUMBER:
            switch (value->u.number.type) {
                case JSON_NUMBER_TYPE_NEGATIVE_INTEGER:
                    fputs ("-", file);
                    /* fall-through */
                case JSON_NUMBER_TYPE_POSITIVE_INTEGER: {
                    char *string = uint256CoerceString (value->u.number.u.integer, 10);
                    fputs (string, file);
                    free (string);
                    break;
                }
                case JSON_NUMBER_TYPE_REAL:
                    fprintf (file, "%f", value->u.number.u.real);
                    break;
            }
            break;
        case JSON_TYPE_STRING:
            fprintf (file, "\"%s\"", value->u.string);
            break;

        case JSON_TYPE_ARRAY: {
            char *prefix = "[";
            for (size_t index = 0; index < array_count(value->u.array); index++) {
                fputs (prefix, file);
                jsonValueWriteDetailed (value->u.array[index], file, pretty, true,
                                        lineIndentation + 2,
                                        linePrefix);
                prefix = ", ";
            }
            jsonValueWriteNewline (file, pretty, true, lineIndentation, linePrefix);
            fputs ("]", file);
            break;
        }
        case JSON_TYPE_OBJECT: {
            char *prefix = "{";

            size_t membersCount = BRSetCount (value->u.object);
            BRJsonObjectMember *members[membersCount];
            BRSetAll (value->u.object, (void**) members, membersCount);
            jsonMembersSort (members, membersCount);

            for (size_t index = 0; index < membersCount; index++) {
                BRJsonObjectMember *member = members[index];
                fputs (prefix, file);
                jsonValueWriteNewline (file, pretty, true, lineIndentation + 2, linePrefix);
                fprintf (file, "\"%s\":%s", member->label, (pretty ? " " : ""));
                jsonValueWriteDetailed (member->value, file, pretty, false,
                                        lineIndentation + 2,
                                        linePrefix);
                prefix=",";
            }
            jsonValueWriteNewline (file, pretty, true, lineIndentation, linePrefix);
            fputs ("}", file);
            break;
        }
    }
}

extern void
jsonWrite (BRJson value,
           FILE *file) {
    jsonValueWriteDetailed (value, file, false, false, 0, "");
}
extern void
jsonWritePretty (BRJson value,
                 FILE *file,
                 size_t lineIndentation,
                 const char *linePrefix) {
    jsonValueWriteDetailed (value, file, true, true, lineIndentation, linePrefix);
}

