//  testStructure.c
//  CoreTests
//
//  Created by Ed Gamble on 9/21/21.
//  Copyright © 2018-2019 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.


#include <stdio.h>
#include <assert.h>
#include "ethereum/base/BREthereumStructure.h"
#include "support/json/BRJson.h"
#include "ethereum/blockchain/BREthereumAccount.h"

static BRJson
testStructureExample1 (BRJsonStatus *status) {
    return jsonCreateObjectVargs (status, 4,
                                       ((BRJsonObjectMember) { "types", jsonCreateObjectVargs (status, 3,
                                                                                                    ((BRJsonObjectMember) { "EIP712Domain", jsonCreateArrayVargs (status, 4,
                                                                                                                                                                       jsonCreateObjectVargs (status, 2,
                                                                                                                                                                                                   ((BRJsonObjectMember) { "name", jsonCreateString ("name") }),
                                                                                                                                                                                                   ((BRJsonObjectMember) { "type", jsonCreateString ("string") })),
                                                                                                                                                                       jsonCreateObjectVargs (status, 2,
                                                                                                                                                                                                   ((BRJsonObjectMember) { "name", jsonCreateString ("version") }),
                                                                                                                                                                                                   ((BRJsonObjectMember) { "type", jsonCreateString ("string") })),
                                                                                                                                                                       jsonCreateObjectVargs (status, 2,
                                                                                                                                                                                                   ((BRJsonObjectMember) { "name", jsonCreateString ("chainId") }),
                                                                                                                                                                                                   ((BRJsonObjectMember) { "type", jsonCreateString ("uint256") })),
                                                                                                                                                                       jsonCreateObjectVargs (status, 2,
                                                                                                                                                                                                   ((BRJsonObjectMember) { "name", jsonCreateString ("verifyingContract") }),
                                                                                                                                                                                                   ((BRJsonObjectMember) { "type", jsonCreateString ("address") }))) }),
                                                                                                    ((BRJsonObjectMember) { "Person", jsonCreateArrayVargs (status, 2,
                                                                                                                                                                 jsonCreateObjectVargs (status, 2,
                                                                                                                                                                                             ((BRJsonObjectMember) { "name", jsonCreateString ("name") }),
                                                                                                                                                                                             ((BRJsonObjectMember) { "type", jsonCreateString ("string") })),
                                                                                                                                                                 jsonCreateObjectVargs (status, 2,
                                                                                                                                                                                             ((BRJsonObjectMember) { "name", jsonCreateString ("wallet") }),
                                                                                                                                                                                             ((BRJsonObjectMember) { "type", jsonCreateString ("address") }))) }),
                                                                                                    ((BRJsonObjectMember) { "Mail", jsonCreateArrayVargs (status, 3,
                                                                                                                                                               jsonCreateObjectVargs (status, 2,
                                                                                                                                                                                           ((BRJsonObjectMember) { "name", jsonCreateString ("from") }),
                                                                                                                                                                                           ((BRJsonObjectMember) { "type", jsonCreateString ("Person") })),
                                                                                                                                                               jsonCreateObjectVargs (status, 2,
                                                                                                                                                                                           ((BRJsonObjectMember) { "name", jsonCreateString ("to") }),
                                                                                                                                                                                           ((BRJsonObjectMember) { "type", jsonCreateString ("Person") })),
                                                                                                                                                               jsonCreateObjectVargs (status, 2,
                                                                                                                                                                                           ((BRJsonObjectMember) { "name", jsonCreateString ("contents") }),
                                                                                                                                                                                           ((BRJsonObjectMember) { "type", jsonCreateString ("string") }))) })) }),
                                       ((BRJsonObjectMember) { "primaryType", jsonCreateString ("Mail") }),
                                       ((BRJsonObjectMember) { "domain", jsonCreateObjectVargs (status, 4,
                                                                                                     ((BRJsonObjectMember) { "name",    jsonCreateString  ("Ether Mail") }),
                                                                                                     ((BRJsonObjectMember) { "version", jsonCreateString  ("1") }),
                                                                                                     ((BRJsonObjectMember) { "chainId", jsonCreateInteger (uint256Create(1), false) }),
                                                                                                     ((BRJsonObjectMember) { "verifyingContract", jsonCreateString ("0xCcCCccccCCCCcCCCCCCcCcCccCcCCCcCcccccccC") })) }),
                                       ((BRJsonObjectMember) { "message", jsonCreateObjectVargs (status, 3,
                                                                                                      ((BRJsonObjectMember) { "from", jsonCreateObjectVargs (status, 2,
                                                                                                                                                                  ((BRJsonObjectMember) { "name",   jsonCreateString ("Cow") }),
                                                                                                                                                                  ((BRJsonObjectMember) { "wallet", jsonCreateString ("0xCD2a3d9F938E13CD947Ec05AbC7FE734Df8DD826") })) }),
                                                                                                      ((BRJsonObjectMember) { "to",   jsonCreateObjectVargs (status, 2,
                                                                                                                                                                  ((BRJsonObjectMember) { "name",   jsonCreateString ("Bob") }),
                                                                                                                                                                  ((BRJsonObjectMember) { "wallet", jsonCreateString ("0xbBbBBBBbbBBBbbbBbbBbbbbBBbBbbbbBbBbbBBbB") })) }),
                                                                                                      ((BRJsonObjectMember) { "contents", jsonCreateString ("Hello, Bob!") })) }) );
}

#if defined (EXAMPLE_1)
{
    "types": {
        "EIP712Domain":[
            {"name":"name","type":"string"},
             {"name":"version","type":"string"},
             {"name":"chainId","type":"uint256"},
             {"name":"verifyingContract","type":"address"}
        ],

        "Person":[
            {"name":"name","type":"string"},
             {"name":"wallet","type":"address"}
        ],

        "Mail":[
            {"name":"from","type":"Person"},
             {"name":"to","type":"Person"},
             {"name":"contents","type":"string"}
        ]
    },

    "primaryType":"Mail",
    "domain":{
        "name":"Ether Mail",
        "version":"1",
        "chainId":1,
        "verifyingContract":"0xCcCCccccCCCCcCCCCCCcCcCccCcCCCcCcccccccC"
    },

    "message":{
        "from":{
            "name":"Cow",
            "wallet":"0xCD2a3d9F938E13CD947Ec05AbC7FE734Df8DD826"},
        "to":{
            "name":"Bob",
            "wallet":"0xbBbBBBBbbBBBbbbBbbBbbbbBBbBbbbbBbBbbBBbB"},
        "contents":"Hello, Bob!"
    }
}

{
    "result" : "0x4355c47d63924e8a72e509b65029052eb6c299d53a04e167c5775fd466751c9d07299936d304c153f6443dfa05f40ff007d72911b6f72307f996231605b915621c"
}

const privateKey = ethUtil.keccak256('cow');
const address = ethUtil.privateToAddress(privateKey);
const sig = ethUtil.ecsign(signHash(), privateKey);

const expect = chai.expect;
expect(encodeType('Mail')).to.equal('Mail(Person from,Person to,string contents)Person(string name,address wallet)');
expect(ethUtil.bufferToHex(typeHash('Mail'))).to.equal('0xa0cedeb2dc280ba39b857546d74f5549c3a1d7bdc2dd96bf881f76108e23dac2');

expect(ethUtil.bufferToHex(encodeData(typedData.primaryType, typedData.message))).to.equal('0xa0cedeb2dc280ba39b857546d74f5549c3a1d7bdc2dd96bf881f76108e23dac2fc71e5fa27ff56c350aa531bc129ebdf613b772b6604664f5d8dbe21b85eb0c8cd54f074a4af31b4411ff6a60c9719dbd559c221c8ac3492d9d872b041d703d1b5aadf3154a261abdd9086fc627b61efca26ae5702701d05cd2305f7c52a2fc8');
expect(ethUtil.bufferToHex(structHash(typedData.primaryType, typedData.message))).to.equal('0xc52c0ee5d84264471806290a3f2c4cecfc5490626bf912d01f240d7a274b371e');

expect(ethUtil.bufferToHex(structHash('EIP712Domain', typedData.domain))).to.equal('0xf2cee375fa42b42143804025fc449deafd50cc031ca257e0b194a650a912090f');

expect(ethUtil.bufferToHex(signHash())).to.equal('0xbe609aee343fb3c4b28e1df9e632fca64fcfaede20f02e86244efddf30957bd2');
expect(ethUtil.bufferToHex(address)).to.equal('0xcd2a3d9f938e13cd947ec05abc7fe734df8dd826');

expect(sig.v).to.equal(28);
expect(ethUtil.bufferToHex(sig.r)).to.equal('0x4355c47d63924e8a72e509b65029052eb6c299d53a04e167c5775fd466751c9d');
expect(ethUtil.bufferToHex(sig.s)).to.equal('0x07299936d304c153f6443dfa05f40ff007d72911b6f72307f996231605b91562');
#endif

static void
runStructureExample1Test (void) {
    printf ("    ==== Structure Example1\n");

    BRJsonStatus status;
    BRJson exp1 = testStructureExample1(&status);

    jsonShow (exp1, NULL);

    BREthereumStructureErrorType error;

    BREthereumStructureCoder coder = ethStructureCoderCreateFromTypedData (exp1, &error);

    const char *typeMailResult  = "Mail(Person from,Person to,string contents)Person(string name,address wallet)";
    char       *typeMailCompute = ethStructureEncodeType (coder, "Mail");
    assert (0 == strcmp (typeMailResult, typeMailCompute));
    free (typeMailCompute);

    BREthereumHash typeHashMailResult  = ethHashCreate("0xa0cedeb2dc280ba39b857546d74f5549c3a1d7bdc2dd96bf881f76108e23dac2");
    BREthereumHash typeHashMailFaked   = ethHashCreateFromBytes ((uint8_t*) typeMailResult, strlen (typeMailResult));
    assert (ETHEREUM_BOOLEAN_TRUE == ethHashEqual(typeHashMailResult, typeHashMailFaked));
    BREthereumHash typeHashMailCompute = ethStructureHashType (coder, "Mail");
    assert (ETHEREUM_BOOLEAN_TRUE == ethHashEqual(typeHashMailResult, typeHashMailCompute));


    // We don't encode the typeHash as the prefix of a data encoding
    BREthereumData dataMailResult  = ethDataCreateFromString("0xa0cedeb2dc280ba39b857546d74f5549c3a1d7bdc2dd96bf881f76108e23dac2fc71e5fa27ff56c350aa531bc129ebdf613b772b6604664f5d8dbe21b85eb0c8cd54f074a4af31b4411ff6a60c9719dbd559c221c8ac3492d9d872b041d703d1b5aadf3154a261abdd9086fc627b61efca26ae5702701d05cd2305f7c52a2fc8");
    BREthereumData dataMailCompute = ethStructureEncodeData(coder);
    assert (ETHEREUM_BOOLEAN_TRUE == ethDataEqual (&dataMailResult, &dataMailCompute));

    BREthereumHash hashMailResult  = ethHashCreate ("0xc52c0ee5d84264471806290a3f2c4cecfc5490626bf912d01f240d7a274b371e");
    BREthereumHash hashMailCompute = ethStructureHashData(coder);
    assert (ETHEREUM_BOOLEAN_TRUE == ethHashEqual (hashMailResult, hashMailCompute));

//    expect(ethUtil.bufferToHex(structHash('EIP712Domain', typedData.domain))).to.equal('0xf2cee375fa42b42143804025fc449deafd50cc031ca257e0b194a650a912090f');
    BREthereumHash hashDomainResult = ethHashCreate ("0xf2cee375fa42b42143804025fc449deafd50cc031ca257e0b194a650a912090f");
    BREthereumHash hashDomainCompute = ethStructureHashDomain (coder);
    assert (ETHEREUM_BOOLEAN_TRUE == ethHashEqual (hashDomainResult, hashDomainCompute));


    BRKey privateKey;
    BREthereumHash privateKeyBytes = ethHashCreateFromBytes((uint8_t*) "cow", 3);
    BRKeySetSecret (&privateKey, (const UInt256 *) &privateKeyBytes, 0);
    BRKeyPubKey (&privateKey, NULL, 0);

    BREthereumAccount account = ethAccountCreateWithPublicKey(privateKey);
    BREthereumAddress address = ethAccountGetPrimaryAddress (account);
    BREthereumAddress addressResult = ethAddressCreate("0xcd2a3d9f938e13cd947ec05abc7fe734df8dd826");
    assert (ETHEREUM_BOOLEAN_TRUE == ethAddressEqual (address, addressResult));

    BREthereumSignature sigResult = ((BREthereumSignature) { SIGNATURE_TYPE_RECOVERABLE_VRS_EIP, { .vrs = { 28 }}});
    hexDecode (sigResult.sig.vrs.r, 32, "4355c47d63924e8a72e509b65029052eb6c299d53a04e167c5775fd466751c9d", 64);
    hexDecode (sigResult.sig.vrs.s, 32, "07299936d304c153f6443dfa05f40ff007d72911b6f72307f996231605b91562", 64);

    BREthereumStructureSignResult sigCompute = ethStructureSignData (coder, privateKey);
    assert (ETHEREUM_BOOLEAN_TRUE == ethSignatureEqual (sigResult, sigCompute.signature));

    int sigExtractSuccess;
    BREthereumAddress signatureAddress = ethSignatureExtractAddress(sigCompute.signature,
                                                                    sigCompute.message.bytes, sigCompute.message.count,
                                                                    &sigExtractSuccess);
    assert (ETHEREUM_BOOLEAN_TRUE == ethAddressEqual (address, signatureAddress));
}

// MARK: - Example 2

static BRJson
testStructureExample2 (BRJsonStatus *status) {
    BRJson typeEIP712Domain = jsonCreateArrayVargs (status, 4,
                                                              jsonCreateObjectVargs (status, 2,
                                                                                          ((BRJsonObjectMember) { "name", jsonCreateString ("name") }),
                                                                                          ((BRJsonObjectMember) { "type", jsonCreateString ("string") })),
                                                              jsonCreateObjectVargs (status, 2,
                                                                                          ((BRJsonObjectMember) { "name", jsonCreateString ("version") }),
                                                                                          ((BRJsonObjectMember) { "type", jsonCreateString ("string") })),
                                                              jsonCreateObjectVargs (status, 2,
                                                                                          ((BRJsonObjectMember) { "name", jsonCreateString ("chainId") }),
                                                                                          ((BRJsonObjectMember) { "type", jsonCreateString ("uint256") })),
                                                              jsonCreateObjectVargs (status, 2,
                                                                                          ((BRJsonObjectMember) { "name", jsonCreateString ("verifyingContract") }),
                                                                                          ((BRJsonObjectMember) { "type", jsonCreateString ("address") })));

    BRJson typeRelayRequest = jsonCreateArrayVargs (status, 4,
                                                              jsonCreateObjectVargs (status, 2,
                                                                                          ((BRJsonObjectMember) { "name", jsonCreateString ("target") }),
                                                                                          ((BRJsonObjectMember) { "type", jsonCreateString ("address") })),
                                                              jsonCreateObjectVargs (status, 2,
                                                                                          ((BRJsonObjectMember) { "name", jsonCreateString ("encodedFunction") }),
                                                                                          ((BRJsonObjectMember) { "type", jsonCreateString ("bytes") })),
                                                              jsonCreateObjectVargs (status, 2,
                                                                                          ((BRJsonObjectMember) { "name", jsonCreateString ("gasData") }),
                                                                                          ((BRJsonObjectMember) { "type", jsonCreateString ("GasData") })),
                                                              jsonCreateObjectVargs (status, 2,
                                                                                          ((BRJsonObjectMember) { "name", jsonCreateString ("relayData") }),
                                                                                          ((BRJsonObjectMember) { "type", jsonCreateString ("RelayData") })));

    BRJson typeGasData = jsonCreateArrayVargs (status, 4,
                                                         jsonCreateObjectVargs (status, 2,
                                                                                     ((BRJsonObjectMember) { "name", jsonCreateString ("gasLimit") }),
                                                                                     ((BRJsonObjectMember) { "type", jsonCreateString ("uint256") })),
                                                         jsonCreateObjectVargs (status, 2,
                                                                                     ((BRJsonObjectMember) { "name", jsonCreateString ("gasPrice") }),
                                                                                     ((BRJsonObjectMember) { "type", jsonCreateString ("uint256") })),
                                                         jsonCreateObjectVargs (status, 2,
                                                                                     ((BRJsonObjectMember) { "name", jsonCreateString ("pctRelayFee") }),
                                                                                     ((BRJsonObjectMember) { "type", jsonCreateString ("uint256") })),
                                                         jsonCreateObjectVargs (status, 2,
                                                                                     ((BRJsonObjectMember) { "name", jsonCreateString ("baseRelayFee") }),
                                                                                     ((BRJsonObjectMember) { "type", jsonCreateString ("uint256") })));

    BRJson typeRelayData = jsonCreateArrayVargs (status, 4,
                                                           jsonCreateObjectVargs (status, 2,
                                                                                       ((BRJsonObjectMember) { "name", jsonCreateString ("senderAddress") }),
                                                                                       ((BRJsonObjectMember) { "type", jsonCreateString ("address") })),
                                                           jsonCreateObjectVargs (status, 2,
                                                                                       ((BRJsonObjectMember) { "name", jsonCreateString ("senderNonce") }),
                                                                                       ((BRJsonObjectMember) { "type", jsonCreateString ("uint256") })),
                                                           jsonCreateObjectVargs (status, 2,
                                                                                       ((BRJsonObjectMember) { "name", jsonCreateString ("relayWorker") }),
                                                                                       ((BRJsonObjectMember) { "type", jsonCreateString ("address") })),
                                                           jsonCreateObjectVargs (status, 2,
                                                                                       ((BRJsonObjectMember) { "name", jsonCreateString ("paymaster") }),
                                                                                       ((BRJsonObjectMember) { "type", jsonCreateString ("address") })));

    return jsonCreateObjectVargs (status, 4,
                                       ((BRJsonObjectMember) { "types", jsonCreateObjectVargs (status, 4,
                                                                                                    ((BRJsonObjectMember) { "EIP712Domain", typeEIP712Domain }),
                                                                                                    ((BRJsonObjectMember) { "RelayRequest", typeRelayRequest }),
                                                                                                    ((BRJsonObjectMember) { "GasData",      typeGasData      }),
                                                                                                    ((BRJsonObjectMember) { "RelayData",    typeRelayData    }) )}),
                                       ((BRJsonObjectMember) { "primaryType", jsonCreateString ("RelayRequest") }),
                                       ((BRJsonObjectMember) { "domain", jsonCreateObjectVargs (status, 4,
                                                                                                     ((BRJsonObjectMember) { "name",    jsonCreateString  ("GSN Relayed Transaction") }),
                                                                                                     ((BRJsonObjectMember) { "version", jsonCreateString  ("1") }),
                                                                                                     ((BRJsonObjectMember) { "chainId", jsonCreateInteger (uint256Create(42), false) }),
                                                                                                     ((BRJsonObjectMember) { "verifyingContract", jsonCreateString ("0x6453D37248Ab2C16eBd1A8f782a2CBC65860E60B") })) }),
                                       ((BRJsonObjectMember) { "message", jsonCreateObjectVargs (status, 4,
                                                                                                      ((BRJsonObjectMember) { "target",          jsonCreateString ("0x9cf40ef3d1622efe270fe6fe720585b4be4eeeff") }),
                                                                                                      ((BRJsonObjectMember) { "encodedFunction", jsonCreateString ("0xa9059cbb0000000000000000000000002e0d94754b348d208d64d52d78bcd443afa9fa520000000000000000000000000000000000000000000000000000000000000007") }),
                                                                                                      ((BRJsonObjectMember) { "gasData",   jsonCreateObjectVargs (status, 4,
                                                                                                                                                                       ((BRJsonObjectMember) { "gasLimit",     jsonCreateString ("39507") }),
                                                                                                                                                                       ((BRJsonObjectMember) { "gasPrice",     jsonCreateString ("1700000000") }),
                                                                                                                                                                       ((BRJsonObjectMember) { "pctRelayFee",  jsonCreateString ("70") }),
                                                                                                                                                                       ((BRJsonObjectMember) { "baseRelayFee", jsonCreateString ("0") })) }),
                                                                                                      ((BRJsonObjectMember) { "relayData", jsonCreateObjectVargs (status, 4,
                                                                                                                                                                       ((BRJsonObjectMember) { "senderAddress", jsonCreateString ("0x22d491bde2303f2f43325b2108d26f1eaba1e32b") }),
                                                                                                                                                                       ((BRJsonObjectMember) { "senderNonce",   jsonCreateString ("3") }),
                                                                                                                                                                       ((BRJsonObjectMember) { "relayWorker",   jsonCreateString ("0x3baee457ad824c94bd3953183d725847d023a2cf") }),
                                                                                                                                                                       ((BRJsonObjectMember) { "paymaster",     jsonCreateString ("0x957F270d45e9Ceca5c5af2b49f1b5dC1Abb0421c") })) }) )}) );
}


static void
runStructureExample2Test (void) {
    printf ("    ==== Structure Example2\n");

    BRJsonStatus status;
    BRJson exp1 = testStructureExample2(&status);

    jsonShow (exp1, NULL);

    BREthereumStructureErrorType error;

    BREthereumStructureCoder coder = ethStructureCoderCreateFromTypedData (exp1, &error);
}

static BRJson
testStructureExample3 (BRJsonStatus *status) {
    const char *input = "{ \
    \"types\": {\
        \"EIP712Domain\": [\
            { \"name\": \"name\",    \"type\": \"string\" },\
              { \"name\": \"version\", \"type\": \"string\" },\
              { \"name\": \"verifyingContract\", \"type\": \"address\" }\
        ],\
\
        \"RelayRequest\": [\
            { \"name\": \"target\", \"type\": \"address\" },\
              { \"name\": \"encodedFunction\", \"type\": \"bytes\" },\
              { \"name\": \"gasData\", \"type\": \"GasData\" },\
              { \"name\": \"relayData\", \"type\": \"RelayData\" }\
        ],\
\
        \"GasData\": [\
            { \"name\": \"gasLimit\", \"type\": \"uint256\" },\
              { \"name\": \"gasPrice\", \"type\": \"uint256\" },\
              { \"name\": \"pctRelayFee\", \"type\": \"uint256\" },\
              { \"name\": \"baseRelayFee\", \"type\": \"uint256\" }\
        ],\
\
        \"RelayData\": [\
            { \"name\": \"senderAddress\", \"type\": \"address\" },\
              { \"name\": \"senderNonce\",   \"type\": \"uint256\" },\
              { \"name\": \"relayWorker\",   \"type\": \"address\" },\
              { \"name\": \"paymaster\",     \"type\": \"address\" }\
        ]\
    },\
\
    \"domain\": {\
        \"name\": \"GSN Relayed Transaction\",\
        \"version\": \"1\",\
        \"chainId\": 42,\
        \"verifyingContract\": \"0x6453D37248Ab2C16eBd1A8f782a2CBC65860E60B\"\
    },\
\
    \"primaryType\": \"RelayRequest\",\
\
    \"message\": {\
        \"target\": \"0x9cf40ef3d1622efe270fe6fe720585b4be4eeeff\",\
        \"encodedFunction\": \"0xa9059cbb0000000000000000000000002e0d94754b348d208d64d52d78bcd443afa9fa520000000000000000000000000000000000000000000000000000000000000007\",\
        \"gasData\": {\
            \"gasLimit\": \"39507\",\
            \"gasPrice\": \"1700000000\",\
            \"pctRelayFee\": \"70\",\
            \"baseRelayFee\": \"0\"\
        },\
        \"relayData\": {\
            \"senderAddress\": \"0x22d491bde2303f2f43325b2108d26f1eaba1e32b\",\
            \"senderNonce\": \"3\",\
            \"relayWorker\": \"0x3baee457ad824c94bd3953183d725847d023a2cf\",\
            \"paymaster\": \"0x957F270d45e9Ceca5c5af2b49f1b5dC1Abb0421c\"\
        }\
    }\
}";
    return jsonParse(input, status, NULL);
}

#if defined (TEST_EXAMPLE_2_RESULT)
// Result {
{
    "address": "0x6E6B9B0D6be28A248F2667560994490c16541799",
    "value"  : true
    "result" : "0x1b063e742ec91f90854726f05568b1dd46313288fb9b3cbe4ad0230b609ec30f11e92a6f099d97a6177748240fdc912e37ac2175946d8874e9baf06517a05a791b"
}
#endif

extern  void
runStructureExample3Test (void) {
    BRJsonStatus status;
    BRJson value = testStructureExample3 (&status);
    assert (JSON_STATUS_OK == status);

    BREthereumStructureErrorType error;

    BREthereumStructureCoder coder = ethStructureCoderCreateFromTypedData (value, &error);
    BREthereumHash           hash  = ethStructureHashData(coder);

}

extern UInt256 // Twos-Complement
uint256Negate (UInt256 value);

static void
runInteger (uint64_t value64) {
    UInt256 value = uint256Create(value64);
    UInt256 valueTwosC = uint256Negate(value);
    int64_t valueI64 = (int64_t) valueTwosC.u64[0];
    assert (valueI64 == -value64);
}

static void
runIntegerTests () {
    runInteger (1);
    runInteger (4);
    runInteger (127);

    BRCoreParseStatus status;
    int overflow;

    UInt256 valuePos = uint256CreateParse("1111111111111111111111111111111111111111", 10, &status);
    assert (CORE_PARSE_OK == status);
    UInt256 valueNeg = uint256Negate (valuePos);
    UInt256 valueSum = uint256Add_Overflow (valuePos, valueNeg, &overflow);
    assert (overflow && UInt256IsZero (valueSum));
}

static void

extern void
runStructureTests (void) {
    printf ("==== Structure\n");

    runIntegerTests();
    runStructureExample1Test ();
    runStructureExample2Test ();
    runStructureExample3Test ();
}



