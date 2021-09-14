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

static const char* testPK = "5Kb8kLf9zgWQnogidDA76MzPL6TsZZY36hWXMssSzNydYXYB9KF";

/** WalletConnector 1.0 Ethereum handlers
 *
 */
extern WKWalletConnectorHandlers wkWalletConnectorHandlersETH;

/** Create a connector for the tests skipping the normal
 *  system setup.
 */
WKWalletConnector createTestConnector(WKWalletManager manager) {
   // WKWalletConnector ethConnector = malloc (sizeof (struct WKWalletConnectorRecord));
   // assert (NULL != ethConnector);

  //  ethConnector->type = WK_NETWORK_TYPE_ETH;
  //  ethConnector->handlers = &wkWalletConnectorHandlersETH;
  //  ethConnector->sizeInBytes = 0;

 //   WKWalletManager manager = malloc (sizeof (struct WKWalletManagerRecord));
//    assert (NULL != manager);

  //  manager->type = WK_NETWORK_TYPE_ETH;
   // ethConnector->manager = manager;

    WKWalletConnector connector = wkWalletConnectorCreate(manager);
    assert (NULL != connector);
    printf ("Created Connector!\n");
    return connector;
}


/** Signing Test:
 *
 *     Akin to API layer sign(byte[] data, Key key, boolean prefix)
 *
 *     To be used in response to 'eth_sign'
 *
 *     Confirms wkWalletConnectorGetDigest()
 *     Confirms wkWalletConnectorSignData()
 */
void runSigningTest(WKWalletConnector connector) {

    WKBoolean                   addPrefix = WK_TRUE;
    WKWalletConnectorStatus     status = WK_WALLET_CONNECTOR_STATUS_OK;

    // From sample dapp. Unfortunately direct comparison with sample dapp
    // output is not possible to confirm this test because it does not implement
    // eth_sign per the WC 1.0 spec
    // "My email is john@doe.com - Tue, 14 Sep 2021 13:47:59 GMT";
    const char* testSignMessageTestStringHex = "0x4d7920656d61696c206973206a6f686e40646f652e636f6d202d205475652c2031342053657020323032312031333a34373a353920474d54";
    size_t msgLen = strlen(testSignMessageTestStringHex) - 2;
    uint8_t* dataToSignArrayifyed = malloc (msgLen); // Drop "0x"
    
    int pos = 0;
    char* p = (char*)(dataToSignArrayifyed + 2); // Drop "0x"
    while (*p != '\0') {
        uint8_t a;
        sscanf (p, "%2hhx", &a);
        dataToSignArrayifyed[pos] = a;
        p += 2;
        pos++;
    }

    size_t digestLength = 0;
    uint8_t* digest = wkWalletConnectorGetDigest(connector,
                                                 dataToSignArrayifyed,
                                                 msgLen,
                                                 addPrefix,
                                                 &digestLength,
                                                 &status);
    assert (WK_WALLET_CONNECTOR_STATUS_OK == status);
    assert (digestLength > 0); // Note: could be confirmed to a precise length if the API exposed it

    size_t signatureLength = 0;
    WKKey key = wkKeyCreateFromStringPrivate(testPK);
    uint8_t* signature = wkWalletConnectorSignData(connector,
                                                   digest,
                                                   digestLength,
                                                   key,
                                                   &signatureLength,
                                                   &status);
    assert (WK_WALLET_CONNECTOR_STATUS_OK == status);
    assert (signatureLength > 0);

    free (dataToSignArrayifyed);
    free (signature);
}

void runConnectorTests (WKWalletConnector connector) {
    runSigningTest(connector);
}

#define WALLET_CONNECT_SUPPORTED_NETS   (1) //  WalletConnect 1.0: Ethereum
                                            //  WalletConnect 2.0: ...

extern void
runWalletConnectTests (void) {

    WKNetworkType testedNets[WALLET_CONNECT_SUPPORTED_NETS] = {WK_NETWORK_TYPE_ETH};
    printf("Run WalletConnect 1.0 tests\n");
    
    for (int net=0; net < WALLET_CONNECT_SUPPORTED_NETS; net++) {
        WKWalletManager manager = malloc (sizeof (struct WKWalletManagerRecord));
        assert (NULL != manager);

        manager->type = testedNets[net];

        // WalletConnector is currently stateless, so its OK to
        // share it among multiple tests
        WKWalletConnector connector = createTestConnector(manager);

        printf ("----- %s Tests ------\n", wkNetworkTypeGetCurrencyCode(manager->type));
        runConnectorTests(connector);

        wkWalletConnectorRelease (connector);
    }

    printf("WalletConnect 1.0 Done\n");
    
    return;
}


