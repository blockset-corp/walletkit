//
//  testJson.c
//  CoreTests
//
//  Created by Ed Gamble on 9/23/21.
//  Copyright © 2018-2019 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "support/json/BRJson.h"

static BRJson
jsonTestsCreateArrayBools () {
    BRJsonStatus status;

    BRJson valBoolTrue  = jsonCreateBoolean (true);
    BRJson valBoolFalse = jsonCreateBoolean (false);

    BRArrayOf(BRJson) valBoolArray;
    array_new (valBoolArray, 2);
    array_add (valBoolArray, valBoolTrue);
    array_add (valBoolArray, valBoolFalse);

    status = JSON_STATUS_OK;
    BRJson valBools = jsonCreateArray(&status, valBoolArray);
    assert (JSON_STATUS_OK == status);

    // Try to add again... will fail
    assert (NULL == jsonCreateArray (&status, valBoolArray));
    assert (JSON_STATUS_OK != status && JSON_STATUS_ERROR_IN_USE == status);
    array_free (valBoolArray);

    return valBools;
}

static BRJson
jsonTestsCreateObject () {
    BRJsonStatus status;

    BRArrayOf(BRJsonObjectMember) members;
    array_new (members, 2);
    array_add (members, ((BRJsonObjectMember) { "lastname", jsonCreateString ("Nakamoto") }));
    array_add (members, ((BRJsonObjectMember) { "fullname", jsonCreateString ("Satoshi Nakamoto") }));

    BRJson valObject = jsonCreateObject (&status, members);
    assert (NULL != valObject && JSON_STATUS_OK == status);

    assert (NULL == jsonCreateObject (&status, members));
    assert (JSON_STATUS_OK != status && JSON_STATUS_ERROR_IN_USE == status);

    array_free (members);

    return valObject;
}

static void
runJSONCreateTests () {
    printf ("  JSON Create Tests\n");
    BRJsonStatus status;

    BRJson valBoolTrue  = jsonCreateBoolean (true);
    BRJson valBoolFalse = jsonCreateBoolean (false);

    assert (JSON_STATUS_OK ==  jsonRelease (valBoolTrue));
    assert (JSON_STATUS_OK ==  jsonRelease (valBoolFalse));

    BRJson valBools = jsonTestsCreateArrayBools();

    // Can't release; invalid to even reference
    assert (JSON_STATUS_OK !=  jsonRelease (valBoolTrue));
    assert (JSON_STATUS_OK !=  jsonRelease (valBoolFalse));

    // Release the array;
    assert (JSON_STATUS_OK ==  jsonRelease (valBools));
}

static void
runJSONObjectTest () {
    printf ("  JSON Object\n");

    BRJsonStatus status;

    BRJson valObject = jsonTestsCreateObject ();

    assert (NULL != valObject);

    BRArrayOf(BRJsonObjectMember) members;
    assert (!jsonExtractNull    (valObject));
    assert (!jsonExtractBoolean (valObject, NULL));
    assert ( jsonExtractObject  (valObject, NULL));
    assert ( jsonExtractObject  (valObject, &members));

    // No ordering guarantee on members

    assert (NULL != members && 2 == array_count (members));
    assert (JSON_STATUS_ERROR_IN_USE ==  jsonRelease (members[0].value));
    assert (JSON_STATUS_ERROR_IN_USE ==  jsonRelease (members[1].value));
    // One of members label is "lastname"
    assert (0 == strcmp ("lastname", members[0].label) ||
            0 == strcmp ("lastname", members[1].label));
    // One of members label is "fullname"
    assert (0 == strcmp ("fullname", members[0].label) ||
            0 == strcmp ("fullname", members[1].label));
    // No duplicates in members label
    assert (0 != strcmp (members[0].label, members[1].label));

    jsonObjectMembersSort(members);
    assert (0 == strcmp ("fullname", members[0].label));
    assert (0 == strcmp ("lastname", members[1].label));

    assert (JSON_STATUS_OK ==  jsonRelease (valObject));
    array_free (members);
}

static void
runJSONObjectPathTest () {
    printf ("  JSON Object Path\n");

    BRJsonStatus status;

    BRJson valObject = jsonTestsCreateObject ();
    assert (NULL != valObject);

    BRJson valLastName = jsonGetValue(valObject, &status, 1, jsonPathCreateLabel("lastname"));
    assert (NULL != valLastName && JSON_STATUS_OK == status);

    const char *name;

    assert (jsonExtractString (valLastName, &name));
    assert (0 == strcmp (name, "Nakamoto"));

    BRJson valFullName = jsonGetValue(valObject, &status, 1, jsonPathCreateLabel("fullname"));
    assert (NULL != valLastName && JSON_STATUS_OK == status);

    assert (jsonExtractString (valFullName, &name));
    assert (0 == strcmp (name, "Satoshi Nakamoto"));

    BRJson valNoneName = jsonGetValue(valObject, &status, 1, jsonPathCreateLabel("nonename"));
    assert (NULL == valNoneName && JSON_STATUS_ERROR_NO_LABEL == status);

    assert (JSON_STATUS_OK ==  jsonRelease (valObject));
}

static void
runJSONArrayTest () {
    printf ("  JSON Array\n");
}

static void
runJSONArrayPathTest () {
    printf ("  JSON Array Path\n");

    BRJsonStatus status;

    BRJson valBools = jsonTestsCreateArrayBools();

    BRJson valBool0 = jsonGetValue(valBools, &status, 1, jsonPathCreateIndex(0));
    assert (NULL != valBool0);
    assert (JSON_STATUS_OK == status);

    BRJson valBool1 = jsonGetValue(valBools, &status, 1, jsonPathCreateIndex(1));
    assert (NULL != valBool1);
    assert (JSON_STATUS_OK == status);

    BRJson valBool2 = jsonGetValue(valBools, &status, 1, jsonPathCreateIndex(2));
    assert (NULL == valBool2);
    assert (JSON_STATUS_ERROR_NO_INDEX == status);

    // Array of valBools
    BRArrayOf(BRJson) values;
    array_new (values, 1);
    array_add (values, valBools);

    BRJson valOfArray = jsonCreateArray (&status, values);
    assert (NULL == jsonGetValue (valOfArray, &status, 1, jsonPathCreateIndex(1)));

    BRJson valOfArray0 = jsonGetValue (valOfArray, &status, 1, jsonPathCreateIndex(0));
    assert (NULL != valOfArray0 && JSON_STATUS_OK == status);

    bool boolean;

    BRJson valOfArray00 = jsonGetValue (valOfArray, &status, 2,
                                             jsonPathCreateIndex(0),
                                             jsonPathCreateIndex(0));
    assert (NULL != valOfArray00 && JSON_STATUS_OK == status);

    BRJson valOfArray01 = jsonGetValue (valOfArray, &status, 2,
                                             jsonPathCreateIndex(0),
                                             jsonPathCreateIndex(1));
    assert (NULL != valOfArray01 && JSON_STATUS_OK == status);

    // true, the false; see jsonTestsCreateArrayBools()
    assert (jsonExtractBoolean (valOfArray00, &boolean) && true  == boolean);
    assert (jsonExtractBoolean (valOfArray01, &boolean) && false == boolean);

    assert (JSON_STATUS_OK == jsonRelease (valOfArray));
}

static void
runJSONShowTest () {
    printf ("  JSON Show\n");

    BRJsonStatus status;

    BRJson valBools  = jsonTestsCreateArrayBools();
    BRJson valObject = jsonTestsCreateObject    ();
    BRJson valComplex = jsonCreateObjectVargs (&status, 3,
                                                         ((BRJsonObjectMember) { "lastname", jsonCreateString ("Nakamoto") }),
                                                         ((BRJsonObjectMember) { "fullname", jsonCreateString ("Satoshi Nakamoto") }),
                                                         ((BRJsonObjectMember) { "children", jsonCreateArrayVargs (&status, 4,
                                                                                                                        jsonCreateString ("Bitcoin"),
                                                                                                                        jsonCreateString ("Bitcoin Cash"),
                                                                                                                        jsonCreateString ("Litecoin"),
                                                                                                                        jsonCreateString ("Doge")) }));

    assert (NULL != valComplex && JSON_STATUS_OK == status);

    const char *prefix = "      ";

    printf ("    JSON Show Array: ");
    jsonShow(valBools, prefix);
    printf ("\n");

    printf ("    JSON Show Object: ");
    jsonShow(valObject, prefix);
    printf ("\n");

    printf ("    JSON Show Complex: ");
    jsonShow(valComplex, prefix);
    printf ("\n");

    assert (JSON_STATUS_OK == jsonRelease(valBools));
    assert (JSON_STATUS_OK == jsonRelease(valObject));
    assert (JSON_STATUS_OK == jsonRelease(valComplex));
}

#define PARSE_TEST_1 "{\"types\":{\"EIP712Domain\":[{\"name\":\"name\",\"type\":\"string\"},{\"name\":\"version\",\"type\":\"string\"},{\"name\":\"verifyingContract\",\"type\":\"address\"}],\"RelayRequest\":[{\"name\":\"target\",\"type\":\"address\"},{\"name\":\"encodedFunction\",\"type\":\"bytes\"},{\"name\":\"gasData\",\"type\":\"GasData\"},{\"name\":\"relayData\",\"type\":\"RelayData\"}],\"GasData\":[{\"name\":\"gasLimit\",\"type\":\"uint256\"},{\"name\":\"gasPrice\",\"type\":\"uint256\"},{\"name\":\"pctRelayFee\",\"type\":\"uint256\"},{\"name\":\"baseRelayFee\",\"type\":\"uint256\"}],\"RelayData\":[{\"name\":\"senderAddress\",\"type\":\"address\"},{\"name\":\"senderNonce\",\"type\":\"uint256\"},{\"name\":\"relayWorker\",\"type\":\"address\"},{\"name\":\"paymaster\",\"type\":\"address\"}]},\"domain\":{\"name\":\"GSN Relayed Transaction\",\"version\":\"1\",\"chainId\":42,\"verifyingContract\":\"0x6453D37248Ab2C16eBd1A8f782a2CBC65860E60B\"},\"primaryType\":\"RelayRequest\",\"message\":{\"target\":\"0x9cf40ef3d1622efe270fe6fe720585b4be4eeeff\",\"encodedFunction\":\"0xa9059cbb0000000000000000000000002e0d94754b348d208d64d52d78bcd443afa9fa520000000000000000000000000000000000000000000000000000000000000007\",\"gasData\":{\"gasLimit\":\"39507\",\"gasPrice\":\"1700000000\",\"pctRelayFee\":\"70\",\"baseRelayFee\":\"0\"},\"relayData\":{\"senderAddress\":\"0x22d491bde2303f2f43325b2108d26f1eaba1e32b\",\"senderNonce\":\"3\",\"relayWorker\":\"0x3baee457ad824c94bd3953183d725847d023a2cf\",\"paymaster\":\"0x957F270d45e9Ceca5c5af2b49f1b5dC1Abb0421c\"}}}"
#define PARSE_TEST_2 "[ 1, -1, 1.2, -1.2, 111111111111111111111111111111111111111111111111, 111111111111111111111111111111111111111111111111.11111, -0e-3, -0.1e+3 ]"
#define PARSE_TEST_3 "[ 1, 2, 3, 111a111, 4, 5 ]"

static void
runJsonParseTest (void) {
    BRJsonStatus status;
    char *error = NULL;

    const char *prefix = "      ";
    BRJson json = jsonParse (PARSE_TEST_1, &status, &error);
    assert (JSON_STATUS_OK == status && NULL == error);
    printf ("    JSON Show Parse 1: ");
    jsonShow(json, prefix);
    printf ("\n");


    json = jsonParse(PARSE_TEST_2, &status, &error);
    assert (JSON_STATUS_OK == status && NULL == error);
    printf ("    JSON Show Parse 2: ");
    jsonShow(json, prefix);
    printf ("\n");

    json = jsonParse(PARSE_TEST_3, &status, &error);
    assert (JSON_STATUS_OK != status && NULL != error);
    printf ("    JSON Show Parse 3:\n");
    printf ("       Input: %s\n", PARSE_TEST_3);
    printf ("       Error: %s\n", error);
    free (error);
}

extern void
runJSONTests (void) {
    printf ("JSON Tests\n");
    runJSONShowTest ();

    runJSONCreateTests ();

    runJSONObjectTest();
    runJSONObjectPathTest();

    runJSONArrayTest ();
    runJSONArrayPathTest ();

    runJsonParseTest ();
}
