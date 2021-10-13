//
//  testWalletConnect.c
//  WalletKitCore Tests
//
//  Created by Bryan Goring on 09/13/21.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
#include <stdio.h>

#include "WKBase.h"
#include "WKWalletConnector.h"
#include "walletkit/WKWalletConnectorP.h"
#include "walletkit/WKWalletManagerP.h"
#include "WKAccount.h"
#include "walletkit/handlers/eth/WKETH.h"
#include "walletkit/WKHandlersP.h"
#include "WKCurrency.h"
#include "support/BRBIP39WordsEn.h"

/** Initialization data
 *
 */
static const char* testPaperKey = "ginger settle marine tissue robot crane night number ramp coast roast critic";
static const char* testAccountUids = "9f2328e3-4de1-436b-b856-82d4d6200351";

/** The following values are reference test data obtained from Demo App with 'ginger' paperKey, paying attention to:
 *   - testNonce: assigned to the account used in transaction serialization must match what was used for generating
 *                test data from the Demo App.
 *   - testAmount: The amount for the test transaction must also match how the reference transaction data was generated
 * The 'reference...' outputs are the unsigned and signed transaction data, hex encoded.
 */
static uint64_t testNonce = 147;
static const char* testAmount = "2250894172460119296";
static const char* referenceUnsignedTransactionSerializationWithTestNonceHexEncode = "ec8193843b9aca00825208948fb4cb96f7c15f9c39b3854595733f728e1963bc881f3cc852f87b750080038080";
static const char* referenceSignedTransactionSerializationWithTestNonceHexEncode = "f86c8193843b9aca00825208948fb4cb96f7c15f9c39b3854595733f728e1963bc881f3cc852f87b7500802aa092f5974a1a0d8e8e743e43d3460c3a68071e5c3829199b6520264aebbedef1eea004891afcf40b5e22d1263dabb05c582280f2707f345377ee33e20e509e9d9816";

WKSecret walletConnector1Dot0WalletSigningKey = {
    .data = {
         97, 190, 112, 244, 197, 200, 176, 93,
        104,  60, 146, 148, 159, 217, 214, 144,
         45, 101, 225,  68,   8, 211, 242, 156,
        253, 219, 187,  43,  21, 155, 177,  15
     }
};

/** WalletConnector 1.0 Ethereum handlers
 *
 */
extern WKWalletConnectorHandlers wkWalletConnectorHandlersETH;

/** Create a connector for the tests with minimal required system
 *  setup, in particular that required to initialize an ETH manager,
 *  to permit signing through the WalletConnector API.
 */
WKWalletConnector createTestConnector() {

    // We need a valid ETH manager that can produce the nonce and RLP
    // data for (e.g.) during WalletConnector transaction creation.
    // In particular:
    //   Need an ETH account to support `ethTransactionSetNonce`
    //   Need an ETH network to support `ethTransactionGetRlpData`

    // Create accounts object which is needed on Ethereum for signing
    WKAccount accounts = wkAccountCreate(testPaperKey, 1514764800, testAccountUids);

    WKCurrency currency = wkCurrencyCreate("ethereum-ropsten:__native__",
                                           "Ethereum",
                                           "eth",
                                           "native",
                                           NULL);

    // Create an Ethereum network, directly through the handler
    const WKHandlers* ethHandlers = wkHandlersLookup(WK_NETWORK_TYPE_ETH);
    WKNetworkListener noNetListener = {NULL, NULL}; // Hope it works...
    WKNetwork ethNetwork = ethHandlers->network->create(noNetListener,
                                                        "ethereum-ropsten",
                                                        "Ethereum",
                                                        "testnet",
                                                        false,                  // isMainnet?
                                                        15,                     // confirmationPeriodInSeconds
                                                        WK_ADDRESS_SCHEME_NATIVE,
                                                        WK_SYNC_MODE_API_ONLY,
                                                        currency);

    // Need to introduce a currency association before adding the currency unit.
    WKClientCurrencyDenominationBundle ethNativeDenomination = wkClientCurrencyDenominationBundleCreate("Ether",    // name
                                                                                                        "eth",      // code
                                                                                                        "Ξ",        // symbol
                                                                                                        18          );// decimals
    WKClientCurrencyBundle ethNativeBundle = wkClientCurrencyBundleCreate("ethereum-ropsten:__native__",    // id
                                                                          "Ethereum",                       // name
                                                                          "eth",                            // code
                                                                          "native",                         // type
                                                                          "ethereum-ropsten",               // blockchainId
                                                                          NULL,                             // address
                                                                          true,                             // verified
                                                                          1,
                                                                          &ethNativeDenomination);
    wkNetworkAddCurrencyAssociationFromBundle(ethNetwork, ethNativeBundle, WK_FALSE);

    // Without a valid unit, manager creation will assert
    WKUnit weiUnit = wkUnitCreateAsBase(currency,
                                        "wei",
                                        "wei",
                                        "WEI");
    WKUnit gweiUnit = wkUnitCreate(currency,
                                   "gwei",
                                   "Gwei",      // name
                                   "GWEI",      // symbol
                                   weiUnit,     // baseUnit
                                   9       );   // powerOffset
    wkNetworkAddCurrencyUnit(ethNetwork, currency, gweiUnit);

    // Create a minimal Ethereum manager, for our WalletKit needs with no
    // application integration (clients or listeners).
    WKWalletManagerListener noMgrListener = {NULL, NULL};
    WKClient noClient = {0};
    WKWalletManager ethManager = ethHandlers->manager->create(noMgrListener,
                                                              noClient,
                                                              accounts,
                                                              ethNetwork,
                                                              WK_SYNC_MODE_API_ONLY,
                                                              WK_ADDRESS_SCHEME_NATIVE,
                                                              ""    );

    WKWalletConnector connector = wkWalletConnectorCreate(ethManager);
    assert (NULL != connector);

    return connector;
}

/** Runs signing key generation which is used throughout for
 *  turning phrases (a.k.a paperKeys) into signing keys.
 *
 *  Confirms: wkWalletConnectorCreateKey
 */
static void
runKeyCreationTest() {

    WKWalletConnectorStatus     status = WK_WALLET_CONNECTOR_STATUS_OK;
    WKWalletConnector           walletConnector = createTestConnector();

    WKKey signingKey = wkWalletConnectorCreateKey (walletConnector,
                                                   testPaperKey,
                                                   &status);
    assert (NULL != signingKey);
    assert (WK_WALLET_CONNECTOR_STATUS_OK == status);
    assert (wkKeyHasSecret (signingKey));
    printf ("    Signing key confirmed\n");
    wkKeyGive(signingKey);
    printf ("    Signing key released\n");
}

/** Tests signing arbitrary data. Returns the digest and
 *  signature for use in testing key recovery.
 *
 *     Confirms wkWalletConnectorGetDigest()
 *     Confirms wkWalletConnectorSignData()
 *
 * @param digest Output pointer to created digest
 * @param digestLength The number of bytes in the created digest
 * @param signature The signature on the digest
 * @param signatureLength The number of bytes in the signature
 */
static void
runSigningTest(
    uint8_t             **digest,
    size_t              *digestLength,
    uint8_t             **signature,
    size_t              *signatureLength) {

    WKBoolean                   addPrefix = WK_TRUE;
    WKWalletConnectorStatus     status = WK_WALLET_CONNECTOR_STATUS_OK;
    WKWalletConnector           walletConnector = createTestConnector();
    
    // test message for signing, generated by Wallet Connect 1.0 sample dApp:
    // My email is john@doe.com - Tue, 12 Oct 2021 18:08:11 GMT;
    const char* testMsg = "0x4d7920656d61696c206973206a6f686e40646f652e636f6d202d205475652c203132204f637420323032312031383a30383a313120474d54";
    
    // Intermediate and final results for comparison
    char* walletConnector1Dot0WalletMessageHash = "0x1327b0f0321d9595f66c69a1e387bf3a1b0345a03dda00037b88bf13d5bd0d36";
    
    // Create a standard message from the bare input
    size_t msgLen = strlen(testMsg) - 2;
    size_t msgDataLength = (strlen(testMsg) - 2) / 2;
    uint8_t msgData[msgDataLength];
    hexDecode(msgData, msgDataLength, testMsg + 2, strlen(testMsg) - 2);
    size_t standardTestMessageLength = 0;
    uint8_t *standardTestMessage = wkWalletConnectorCreateStandardMessage(walletConnector,
                                                                          msgData,
                                                                          msgDataLength,
                                                                          &standardTestMessageLength,
                                                                          &status);
    assert (WK_WALLET_CONNECTOR_STATUS_OK == status);
    assert (standardTestMessageLength > 0);
    printf ("    Created standard message len %lu\n", standardTestMessageLength);

    // Get a digest of the standard message and compare to what WalletConnect 1.0 samples
    // produce
    *digestLength = 0;
    *digest = wkWalletConnectorGetDigest(walletConnector,
                                         standardTestMessage,
                                         standardTestMessageLength,
                                         digestLength,
                                         &status);
    assert (WK_WALLET_CONNECTOR_STATUS_OK == status);
    assert (*digestLength > 0);
    
    // Final compare digest. Note that the WalletConnect 1.0 sample wallet does not
    // appear to follow the rules of the spec for 'eth_sign()' in terms of creating the
    // standard message, nor doing the keccak256(). So the following test data hash
    // comes from what the WalletConnect 1.0 sample dApp does to verify the signature
    // and recover the address.
    char digestBuf[2 + *digestLength * 2 + 1];
    sprintf(digestBuf, "0x");
    hexEncode(digestBuf + 2, *digestLength * 2 + 1, *digest, *digestLength);
    printf ("    Our message digest             %s\n", digestBuf);
    printf ("    WalletConnect 1.0 dApp digest  %s\n", walletConnector1Dot0WalletMessageHash);
    assert (0 == strcmp(digestBuf, walletConnector1Dot0WalletMessageHash));

    // Get a signing key
    WKKey signingKey = wkKeyCreateFromSecret (walletConnector1Dot0WalletSigningKey);

    assert (NULL != signingKey);
    
    // Sign the standard message
    *signatureLength = 0;
    *signature = wkWalletConnectorSignData(walletConnector,
                                           standardTestMessage,
                                           standardTestMessageLength,
                                           signingKey,
                                           signatureLength,
                                           &status);
    assert (WK_WALLET_CONNECTOR_STATUS_OK == status);
    assert (*signatureLength > 0);
    
    // Final compare signature
    char signatureHex[2 + *signatureLength * 2 + 1];
    sprintf(signatureHex, "0x");
    hexEncode(signatureHex + 2, *signatureLength * 2 + 1, *signature, *signatureLength);
    printf ("    Our signature                  %s\n", signatureHex);
    
    wkWalletConnectorRelease(walletConnector);

    free (signingKey);
    free (standardTestMessage);
}

/** Tests public key recovery from previous signature and digest
 *
 *     Confirms wkWalletConnectorRecoverKey()
 *
 * @param digest The input message digest
 * @param digestLength The number of bytes in digest
 * @param signature The signature of the message digest
 * @param signatureLength The number of bytes in the signature
 */
static void
runRecoverTest(
    uint8_t             *digest,
    size_t              digestLength,
    uint8_t             *signature,
    size_t              signatureLength) {

    WKWalletConnectorStatus     status = WK_WALLET_CONNECTOR_STATUS_OK;
    WKWalletConnector           walletConnector = createTestConnector();

    char signatureHex[2 + signatureLength * 2 + 1];
    sprintf(signatureHex, "0x");
    hexEncode(signatureHex + 2, signatureLength * 2 + 1, signature, signatureLength);
    char digestHex[2 + digestLength * 2 + 1];
    sprintf(digestHex, "0x");
    hexEncode(digestHex + 2, digestLength * 2 + 1, digest, digestLength);
    printf ("    Recover key from:\n");
    printf ("      signature %s\n", signatureHex);
    printf ("      digest    %s\n", digestHex);
    
    WKKey recoveredKey = wkWalletConnectorRecoverKey (walletConnector,
                                                      digest,
                                                      digestLength,
                                                      signature,
                                                      signatureLength,
                                                      &status    );
    assert (WK_WALLET_CONNECTOR_STATUS_OK == status);
    assert (NULL != recoveredKey);

    printf ("    Recovered Public key from digest and signature\n");
    
    // Get a signing key
    WKKey originalSigningKey = wkKeyCreateFromSecret (walletConnector1Dot0WalletSigningKey);
    assert (NULL != originalSigningKey);
    
    assert ( wkKeyPublicMatch(originalSigningKey, recoveredKey));
    printf ("    Verified recovered public key match to original signing key\n");

    wkKeyGive (originalSigningKey);
    wkKeyGive (recoveredKey);
    wkWalletConnectorRelease (walletConnector);
}

/** Defines a `transactionFromArgs()` test
 *
 */
typedef struct transactionArgsCheckTag {
    const char*     name;
    bool            ok;
    struct {
        const char*     transactionKeys[8];
        const char*     transactionValues[8];
        size_t          entryCount;
    } pairs;
} TransactionArgsTestDefinition;

/** Runs a few different combinations of possible input to the
 *  WalletConnector API for transaction creation from arguments.
 *  Verifies the validation logic of creating transactions from
 *  these arguments, but not the actual output of the serialization.
 *
 *  Confirms: wkWalletConnectorCreateTransactionFromArguments
 */
static void
runTransactionFromArgumentsInputsTest() {

    WKWalletConnector walletConnector = createTestConnector();

    // Based on observing inputs from WalletConnect 1.0 sample dApp
    TransactionArgsTestDefinition transactionArgsTests[] = {

        // In the following
        // gas/gasLimit: 0x5208 (21000)
        // gasPrice: 0x0df8475800 (60,000,000,000)
        // value: 0x3b9aca00 (1,000,000,000)


        // Test: All required fields plus extras
        // 'chainId': May be supplied but is ignored (taken from network of wallet)
        // 'nonce': WalletConnect 1.0 dApp might provide it, we should ignore it
        // 'from': WalletConnect 1.0 wallet sample eliminates the from of transaction fields
        { "Complete", true, {
               {"chainId", "data", "gas", "gasPrice", "nonce", "to", "value", "from" },
               {"3", "0x", "0x5208", "0x0df8475800", "0x08",
                "0x3d39e3313c662ddc955980ef817a7548a670d973", "0x3b9aca00",
                "0x3d39e3313c662ddc955980ef817a7548a670d973" },
               8
           }
        },

        // Test: minimum required fields provided
        { "Minimum fields given", true, {
               {"gasPrice", "to", "gas" },
               {"0x0df8475800", "0x3d39e3313c662ddc955980ef817a7548a670d973", "0x5208"},
               3
           }
        },

        // Test: Missing mandatory 'gas'
        { "Missing mandatory", false, {
               {"gasPrice", "to"},
               {"0x5208", "0x3d39e3313c662ddc955980ef817a7548a670d973"},
               2
           }
        },

        // Test: 'gas' vs 'gasLimit'
        // WalletConnect 1.0 sample dApp provides 'gasLimit' and the sample wallet maps
        // 'gas' to 'gasLimit', but we support only 'gas' per the WalletConnect 1.0 JSON RPC
        // methods description of eth_sendTransaction.
        { "Fail on gasLimit", false, {
               {"chainId", "data", "gasLimit", "gasPrice", "nonce", "to", "value", "from" },
               {"3", "0x", "0x5208", "0x0df8475800", "0x08",
                "0x3d39e3313c662ddc955980ef817a7548a670d973", "0x3b9aca00",
                "0x3d39e3313c662ddc955980ef817a7548a670d973" },
               8
           }
        }
    };

    printf ("  -- %s\n", __FUNCTION__);
    int numberOfTests = sizeof (transactionArgsTests) / sizeof (TransactionArgsTestDefinition);
    for (int i=0; i < numberOfTests; i++) {

        TransactionArgsTestDefinition test = transactionArgsTests[i];
        const char** keys = test.pairs.transactionKeys;
        const char** vals = test.pairs.transactionValues;

        WKWalletConnectorStatus status = WK_WALLET_CONNECTOR_STATUS_OK;
        printf ("    TransactionFromArgs test: %s\n", test.name);
        size_t transactionLength = 0;
        uint8_t *transaction = wkWalletConnectorCreateTransactionFromArguments(walletConnector,
                                                                               keys,
                                                                               vals,
                                                                               test.pairs.entryCount,
                                                                               &transactionLength,
                                                                               &status);
        assert ((test.ok && WK_WALLET_CONNECTOR_STATUS_OK == status) ||
                (!test.ok && WK_WALLET_CONNECTOR_STATUS_INVALID_TRANSACTION_ARGUMENTS == status));
        assert (!test.ok || (test.ok && transactionLength > 0));

        free (transaction);
    }

    wkWalletConnectorRelease(walletConnector);
}

/** Verifies the output of transaction serialization from arguments
 *  by comparing the serialized output against vetted output from
 *  core Demo App, using the same sets of inputs.
 *
 *  Confirms: wkWalletConnectorCreateTransactionFromArguments output
 *
 *  @param transactionSerializationLength On return the created serialization length,
 *                                        or when NULL, is ignored
 *  @return Valid transaction serialization
 */
static uint8_t*
runTransactionFromArgumentsSerializationTest(
    size_t  *transactionSerializationLength) {

    TransactionArgsTestDefinition serializationTest = {
        "Complete", true, {
            {"data", "gas", "gasPrice", "to", "value" },
                {"", "21000", "1000000000",
                 "0x8fB4CB96F7C15F9C39B3854595733F728E1963Bc", testAmount },
                5
            }
    };

    WKWalletConnector walletConnector = createTestConnector();
    WKWalletManagerETH ethWalletManager = wkWalletManagerCoerceETH(walletConnector->manager);
    ethAccountSetAddressNonce(ethWalletManager->account,
                              ethAccountGetPrimaryAddress(ethWalletManager->account),
                              testNonce,
                              ETHEREUM_BOOLEAN_TRUE);

    const char** keys = serializationTest.pairs.transactionKeys;
    const char** vals = serializationTest.pairs.transactionValues;

    WKWalletConnectorStatus status = WK_WALLET_CONNECTOR_STATUS_OK;
    size_t serializationLength = 0;
    uint8_t *serialization = wkWalletConnectorCreateTransactionFromArguments(walletConnector,
                                                                             keys,
                                                                             vals,
                                                                             serializationTest.pairs.entryCount,
                                                                             &serializationLength,
                                                                             &status);
    
    assert (WK_WALLET_CONNECTOR_STATUS_OK == status);
    assert (serializationLength > 0);
    
    if (NULL != transactionSerializationLength)
        *transactionSerializationLength = serializationLength;

    // Compare result serialization hex encode to reference
    char walletConnectorSerializationHexEncode[2 * serializationLength + 1];
    hexEncode(walletConnectorSerializationHexEncode,
              2 * serializationLength + 1,
              serialization,
              serializationLength);

    printf("     WalletConnector transaction serialization %s\n", walletConnectorSerializationHexEncode);
    assert (strcmp (walletConnectorSerializationHexEncode,
                    referenceUnsignedTransactionSerializationWithTestNonceHexEncode) == 0);
    
    wkWalletConnectorRelease(walletConnector);

    return serialization;
}

/** Runs both transactionFromArguments tests
 *
 */
static void
runTransactionFromArgsTest() {
    runTransactionFromArgumentsInputsTest();
    uint8_t *ignore = runTransactionFromArgumentsSerializationTest(NULL);
    free (ignore);
}

/** Verifies the signature of a transaction, making a signature upon the
 *  validated serialization of known inputs. Again, a comparison of the
 *  signature against the vetted output for the same sets of inputs (values & signing key)
 *  produced by the core Demo App, is used to establish correctness.
 *
 *  Confirms: wkWalletConnectorSignTransactionData output
 *
 *  @param signedTransactionSerializationLength The number of bytes in signed transaction,
 *                                              or when NULL, is ignored
 *  @return The signed transaction serialization
 */
static uint8_t*
runSignTransactionTest(size_t *signedTransactionSerializationLength) {

    size_t                  transactionDataLength = 0;
    WKWalletConnector       walletConnector = createTestConnector();
    WKWalletConnectorStatus status = WK_WALLET_CONNECTOR_STATUS_OK;

    uint8_t* transactionDataInRLPFormUnsigned = runTransactionFromArgumentsSerializationTest(&transactionDataLength);

    assert (WK_WALLET_CONNECTOR_STATUS_OK == status);
    assert (transactionDataLength > 0);

    // Sign the transaction and compare to canned reference
    WKKey signingKey = wkWalletConnectorCreateKey (walletConnector,
                                                   testPaperKey,
                                                   &status);
    assert (NULL != signingKey);

    size_t signedSerializationLength = 0;
    uint8_t* signedTransactionSerialization = wkWalletConnectorSignTransactionData(walletConnector,
                                                                                   transactionDataInRLPFormUnsigned,
                                                                                   transactionDataLength,
                                                                                   signingKey,
                                                                                   &signedSerializationLength,
                                                                                   &status   );
    assert (WK_WALLET_CONNECTOR_STATUS_OK == status);
    assert (signedSerializationLength > 0);

    if (NULL != signedTransactionSerializationLength)
        *signedTransactionSerializationLength = signedSerializationLength;

    // Compare signed transaction to reference data
    char signedSerializationHexEncode[2 * signedSerializationLength + 1];
    hexEncode(signedSerializationHexEncode,
              2 * signedSerializationLength + 1,
              signedTransactionSerialization,
              signedSerializationLength);

    assert (strcmp (signedSerializationHexEncode,
                    referenceSignedTransactionSerializationWithTestNonceHexEncode) == 0);

    printf ("    Verified signed transaction serialization (len %lu): %s",
            signedSerializationLength,
            signedSerializationHexEncode    );

    free (transactionDataInRLPFormUnsigned);

    wkKeyGive(signingKey);
    wkWalletConnectorRelease(walletConnector);

    return signedTransactionSerialization;
}

/** Verifies that WalletConnector can produce a Transaction from
 *  encoded and serialized input, and return it again in serialized form.
 *
 *  Confirms: wkWalletConnectorCreateTransactionFromSerialization both
 *            for signed and unsigned transactions
 *
 */
static void
runTransactionFromTransactionDataTest() {

    size_t                  transactionDataLength = 0;
    WKWalletConnector       walletConnector = createTestConnector();
    WKWalletConnectorStatus status = WK_WALLET_CONNECTOR_STATUS_OK;
    size_t                  serializationLength = 0;
    WKBoolean               isSigned = WK_FALSE;

    // For unsigned transactions...
    uint8_t* transactionDataInRLPFormUnsigned = runTransactionFromArgumentsSerializationTest(&transactionDataLength);
    uint8_t* transactionFromSerialization = wkWalletConnectorCreateTransactionFromSerialization(walletConnector,
                                                                                                transactionDataInRLPFormUnsigned,
                                                                                                transactionDataLength,
                                                                                                &serializationLength,
                                                                                                &isSigned,
                                                                                                &status );
    assert (WK_WALLET_CONNECTOR_STATUS_OK == status);
    assert (serializationLength > 0);
    assert (isSigned == WK_FALSE);
    free (transactionDataInRLPFormUnsigned);
    free (transactionFromSerialization);
    printf ("    Transaction length %lu created from unsigned RLP serialization\n", serializationLength);

    // Repeat for signed transaction
    uint8_t* signedTransactionFromSerialization = runSignTransactionTest(&transactionDataLength);
    transactionFromSerialization = wkWalletConnectorCreateTransactionFromSerialization(walletConnector,
                                                                                       signedTransactionFromSerialization,
                                                                                       transactionDataLength,
                                                                                       &serializationLength,
                                                                                       &isSigned,
                                                                                       &status );
    assert (WK_WALLET_CONNECTOR_STATUS_OK == status);
    assert (serializationLength > 0);
    assert (isSigned == WK_TRUE);
    free (signedTransactionFromSerialization);
    free (transactionFromSerialization);
    printf ("    Transaction length %lu created from signed RLP serialization\n", serializationLength);

    wkWalletConnectorRelease(walletConnector);
}

static void
runSigningTypedDataTest() {

    WKWalletConnectorStatus status = WK_WALLET_CONNECTOR_STATUS_OK;
    WKWalletConnector       walletConnector = createTestConnector();
    uint8_t                 *typedDataSignature = NULL;
    size_t                  typedDataSignatureLength = 0;
    uint8_t                 *digestData = NULL;
    size_t                  digestLength = 0;

    // Sign the transaction and compare to canned reference
    WKKey signingKey = wkWalletConnectorCreateKey (walletConnector,
                                                   testPaperKey,
                                                   &status);
    assert (NULL != signingKey);

    const char* emptyJson = "";
    assert (NULL == (typedDataSignature = wkWalletConnectorSignTypedData(walletConnector,
                                                                         emptyJson,
                                                                         signingKey,
                                                                         &digestData,
                                                                         &digestLength,
                                                                         &typedDataSignatureLength,
                                                                         &status)) &&
            WK_WALLET_CONNECTOR_STATUS_INVALID_JSON == status);

    const char* invalidJson = "{a=1}";
    assert (NULL == (typedDataSignature = wkWalletConnectorSignTypedData(walletConnector,
                                                                         invalidJson,
                                                                         signingKey,
                                                                         &digestData,
                                                                         &digestLength,
                                                                         &typedDataSignatureLength,
                                                                         &status)) &&
            WK_WALLET_CONNECTOR_STATUS_INVALID_TYPED_DATA != status);

    const char* eip712TypedData =
    "{"
        "\"types\": {"
            "\"EIP712Domain\":["
                "{\"name\":\"name\",\"type\":\"string\"},"
                "{\"name\":\"version\",\"type\":\"string\"},"
                "{\"name\":\"chainId\",\"type\":\"uint256\"},"
                "{\"name\":\"verifyingContract\",\"type\":\"address\"}"
            "],"

            "\"Person\":["
                "{\"name\":\"name\",\"type\":\"string\"},"
                "{\"name\":\"wallet\",\"type\":\"address\"}"
            "],"

            "\"Mail\":["
                "{\"name\":\"from\",\"type\":\"Person\"},"
                "{\"name\":\"to\",\"type\":\"Person\"},"
                "{\"name\":\"contents\",\"type\":\"string\"}"
            "]"
        "},"

        "\"primaryType\":\"Mail\","
        "\"domain\":{"
            "\"name\":\"Ether Mail\","
            "\"version\":\"1\","
            "\"chainId\":1,"
            "\"verifyingContract\":\"0xCcCCccccCCCCcCCCCCCcCcCccCcCCCcCcccccccC\""
        "},"

        "\"message\":{"
            "\"from\":{"
                "\"name\":\"Cow\","
                "\"wallet\":\"0xCD2a3d9F938E13CD947Ec05AbC7FE734Df8DD826\"},"
            "\"to\":{"
                "\"name\":\"Bob\","
                "\"wallet\":\"0xbBbBBBBbbBBBbbbBbbBbbbbBBbBbbbbBbBbbBBbB\"},"
            "\"contents\":\"Hello, Bob!\""
        "}"
    "}";

    typedDataSignature = wkWalletConnectorSignTypedData(walletConnector,
                                                        eip712TypedData,
                                                        signingKey,
                                                        &digestData,
                                                        &digestLength,
                                                        &typedDataSignatureLength,
                                                        &status);
    assert (WK_WALLET_CONNECTOR_STATUS_OK == status);
    assert (NULL != typedDataSignature && 65 == typedDataSignatureLength);
    assert (NULL != digestData && 32 == digestLength);

    free (digestData);
    free (typedDataSignature);

    wkWalletConnectorRelease(walletConnector);
}

static void
runTypedDataSignatureTest() {

    const char* walletConnector1Dot0DappTypedDataRequest =
    "{"
        "\"types\": {"
             "\"EIP712Domain\": ["
                 "{\"name\": \"name\","
                  "\"type\": \"string\" },"
                 "{\"name\": \"version\","
                  "\"type\": \"string\" },"
                 "{\"name\": \"verifyingContract\","
                  "\"type\": \"address\" }"
             "],"
             "\"RelayRequest\": ["
                 "{\"name\": \"target\","
                  "\"type\": \"address\" },"
                 "{\"name\": \"encodedFunction\","
                  "\"type\": \"bytes\" },"
                 "{\"name\": \"gasData\","
                  "\"type\": \"GasData\"},"
                 "{\"name\": \"relayData\","
                  "\"type\": \"RelayData\"}"
             "],"
             "\"GasData\": ["
                 "{\"name\": \"gasLimit\","
                  "\"type\": \"uint256\" },"
                 "{\"name\": \"gasPrice\","
                  "\"type\": \"uint256\" },"
                 "{\"name\": \"pctRelayFee\","
                  "\"type\": \"uint256\"},"
                 "{\"name\": \"baseRelayFee\","
                  "\"type\": \"uint256\"}"
             "],"
             "\"RelayData\": ["
                 "{\"name\": \"senderAddress\","
                  "\"type\": \"address\" },"
                 "{\"name\": \"senderNonce\","
                  "\"type\": \"uint256\" },"
                 "{\"name\": \"relayWorker\","
                  "\"type\": \"address\" },"
                 "{\"name\": \"paymaster\","
                  "\"type\": \"address\" }"
             "]"
        "},"
        "\"domain\": {"
            "\"name\": \"GSN Relayed Transaction\","
            "\"version\": \"1\","
            "\"chainId\": 42,"
            "\"verifyingContract\": \"0x6453D37248Ab2C16eBd1A8f782a2CBC65860E60B\""
        "},"
        "\"primaryType\": \"RelayRequest\","
        "\"message\": {"
            "\"target\": \"0x9cf40ef3d1622efe270fe6fe720585b4be4eeeff\","
            "\"encodedFunction\": \"0xa9059cbb0000000000000000000000002e0d94754b348d208d64d52d78bcd443afa9fa520000000000000000000000000000000000000000000000000000000000000007\","
            "\"gasData\": {"
                "\"gasLimit\": \"39507\","
                "\"gasPrice\": \"1700000000\","
                "\"pctRelayFee\": \"70\","
                "\"baseRelayFee\": \"0\""
            "},"
            "\"relayData\":{"
                "\"senderAddress\":\"0x22d491bde2303f2f43325b2108d26f1eaba1e32b\","
                "\"senderNonce\": \"3\","
                "\"relayWorker\": \"0x3baee457ad824c94bd3953183d725847d023a2cf\","
                "\"paymaster\": \"0x957F270d45e9Ceca5c5af2b49f1b5dC1Abb0421c\""
            "}"
        "}"
    "}";

    uint8_t walletConnector1Dot0WalletTypedDataRequestHash[32] = {
        171, 199, 159,  82, 114, 115, 185, 231,
        188, 161, 179, 241, 172, 106, 209, 168,
         67,  31, 166, 220,  52, 236, 233,   0,
        222, 171, 205, 105, 105, 133, 107,  94
    };

    const char* walletConnector1Dot0WalletSigningResult = "0x7cd2107da9c93030ac5996c0c5da3d27479d9968a3d12cfde88eeba1ef74fdec4f5c137d18fe9ed7b0616f0a9f9af1795105ed0f662f4cbacb92fffb396d7a8d1c";

    WKWalletConnectorStatus status = WK_WALLET_CONNECTOR_STATUS_OK;
    WKWalletConnector       walletConnector = createTestConnector();
    uint8_t                 *typedDataSignature = NULL;
    size_t                  typedDataSignatureLength = 0;
    uint8_t                 *digestData = NULL;
    size_t                  digestLength = 0;
    WKSecret                pKey;
    BREthereumSignatureRSV  walletConnector1Dot0WalletSigningResultRsv;

    WKKey signingKey = wkKeyCreateFromSecret (walletConnector1Dot0WalletSigningKey);

    typedDataSignature = wkWalletConnectorSignTypedData(walletConnector,
                                                        walletConnector1Dot0DappTypedDataRequest,
                                                        signingKey,
                                                        &digestData,
                                                        &digestLength,
                                                        &typedDataSignatureLength,
                                                        &status);
    
    // Verify basic requirements and that the same input 'RelayRequest' typed data input
    // produces the same hash by WalletKit as what the Wallet Connect 1.0 sample wallet does
    assert (WK_WALLET_CONNECTOR_STATUS_OK == status);
    assert (NULL != typedDataSignature && 65 == typedDataSignatureLength);
    assert (NULL != digestData && 32 == digestLength);
    assert (0 == memcmp(digestData, walletConnector1Dot0WalletTypedDataRequestHash, 32));

    // For display only...
    char walletConnectSigHex[typedDataSignatureLength * 2 + 1];
    hexEncode(walletConnectSigHex,
              typedDataSignatureLength * 2 + 1,
              typedDataSignature,
              typedDataSignatureLength);
    printf("WalletConnect 1.0 Signature %s\n", walletConnector1Dot0WalletSigningResult + 2);
    printf("Ours:                       %s\n", walletConnectSigHex);

    // Compare what we produced to against what WalletConnect 1.0 sample dApp verifies as correct
    hexDecode ((uint8_t*)&walletConnector1Dot0WalletSigningResultRsv,
               sizeof (walletConnector1Dot0WalletSigningResultRsv),
               walletConnector1Dot0WalletSigningResult + 2,
               strlen (walletConnector1Dot0WalletSigningResult) - 2);

    BREthereumSignatureRSV *ourResultRsv = (BREthereumSignatureRSV*)typedDataSignature;
    assert (0 == memcmp (walletConnector1Dot0WalletSigningResultRsv.r, ourResultRsv->r, 32));
    assert (0 == memcmp (walletConnector1Dot0WalletSigningResultRsv.s, ourResultRsv->s, 32));
    
    // WalletConnect 1.0 samples ignore the chainId both in generation (wallet) and confirmation (dApp)
    // and thus does not account for the EIP-155 treatment of 'v' value.
    // Signing on the wallet side is done via ethereumjs-util/dist/index.js -- exports.ecsign()
   // assert (walletConnector1Dot0WalletSigningResultRsv.v == ourResultRsv->v);

    free (digestData);
    free (typedDataSignature);

    wkWalletConnectorRelease(walletConnector);
}

static void
runSignTypedDataTest() {
    runSigningTypedDataTest();

    // Compare WalletKit output against WalletConnect 1.0 sample wallet
    runTypedDataSignatureTest();
}

/** Run all WalletConnector interface tests.
 *
 *
 *  The tests are formulated from the perspective of a user of 'WKWalletConnector.h'.
 *  As such certain basic initialization of the WalletConnector is required in order
 *  to run these tests, and certain validation information from vetted sources (core Demo App)
 *  is required to confirm outputs.
 */
extern void
runWalletConnectTests (void) {
    uint8_t *digest = NULL;
    uint8_t *signature = NULL;
    size_t  digestLength = 0;
    size_t  signatureLength = 0;
    uint8_t *transactionFromArguments = NULL;
    size_t  transactionFromArgumentsLength = 0;

    printf("Run WalletConnect 1.0 tests\n");

    // Create a key
    printf("Key generation test\n");
    runKeyCreationTest();

    // Produce signature of arbitrary data
    printf ("Signing tests\n");
    runSigningTest(&digest, &digestLength, &signature, &signatureLength);

    // Verify recovery of the public key
    printf ("Public Key recovery tests\n");
    runRecoverTest(digest, digestLength, signature, signatureLength);
    free (digest);
    free (signature);

    // Create a transaction from arguments
    printf ("Create transaction from arguments\n");
    runTransactionFromArgsTest();

    // Create a transaction from a previous transaction serialization
    printf ("Create transaction from transaction serialization\n");
    runTransactionFromTransactionDataTest();

    // Sign a transaction
    uint8_t *ignore = runSignTransactionTest(NULL);
    free (ignore);

    // Sign typed data
    printf ("Sign typed data\n");
    runSignTypedDataTest();

    printf("WalletConnect 1.0 Done\n");
}
