//
//  BRCryptoWalletManagerClient.c
//  BRCrypto
//
//  Created by Michael Carrara on 6/19/19.
//  Copyright © 2019 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "BRCryptoClientP.h"

#include <errno.h>
#include <math.h>  // round()
#include <stdbool.h>

static void
cryptoClientP2PManagerSync (BRCryptoClientP2PManager p2p, BRCryptoSyncDepth depth);

static void
cryptoClientP2PManagerSend (BRCryptoClientP2PManager p2p, BRCryptoTransfer transfer);

static void
cryptoClientQRYManagerSync (BRCryptoClientQRYManager qry, BRCryptoSyncDepth depth);

static void
cryptoClientQRYManagerSend (BRCryptoClientQRYManager qry, BRCryptoTransfer transfer);


// MARK: Client Sync

extern void
cryptoClientSync (BRCryptoClientSync sync, BRCryptoSyncDepth depth) {
    switch (sync.type) {
        case CRYPTO_CLIENT_P2P_MANAGER_TYPE:
            cryptoClientP2PManagerSync (sync.u.p2pManager, depth);
            break;
        case CRYPTO_CLIENT_QRY_MANAGER_TYPE:
            cryptoClientQRYManagerSync (sync.u.qryManager, depth);
            break;
    }
}

// MARK: Client Send

extern void
cryptoClientSend (BRCryptoClientSend send, BRCryptoTransfer transfer) {
    switch (send.type) {
        case CRYPTO_CLIENT_P2P_MANAGER_TYPE:
            cryptoClientP2PManagerSend (send.u.p2pManager, transfer);
            break;
        case CRYPTO_CLIENT_QRY_MANAGER_TYPE:
            cryptoClientQRYManagerSend (send.u.qryManager, transfer);
            break;
    }
}

// MARK: Client P2P (Peer-to-Peer)

extern BRCryptoClientP2PManager
cryptoClientP2PManagerCreate (size_t sizeInBytes,
                              BRCryptoBlockChainType type,
                              BRCryptoClientP2PManagerReleaseHandler releaseHandler,
                              BRCryptoClientP2PManagerSyncHandler syncHandler,
                              BRCryptoClientP2PManagerSendHandler sendHandler) {
    assert (sizeInBytes >= sizeof (struct BRCryptoClientP2PManagerRecord));

    BRCryptoClientP2PManager p2pManager = calloc (1, sizeInBytes);

    p2pManager->type           = type;
    p2pManager->sizeInBytes    = sizeInBytes;
    p2pManager->releaseHandler = releaseHandler;
    p2pManager->syncHandler    = syncHandler;
    p2pManager->sendHandler    = sendHandler;

    return p2pManager;
}

extern void
cryptoClientP2PManagerRelease (BRCryptoClientP2PManager p2p) {
    p2p->releaseHandler (p2p);
    memset (p2p, 0, p2p->sizeInBytes);
    free (p2p);
}

static void
cryptoClientP2PManagerSync (BRCryptoClientP2PManager p2p, BRCryptoSyncDepth depth) {
    p2p->syncHandler (p2p, depth);
}

static void
cryptoClientP2PManagerSend (BRCryptoClientP2PManager p2p, BRCryptoTransfer transfer) {
    p2p->sendHandler (p2p, transfer);
}

// MARK: Client QRY (QueRY)

extern BRCryptoClientQRYManager
cryptoClientQRYManagerCreate (BRCryptoClient client,
                              BRCryptoWalletManager manager) {
    BRCryptoClientQRYManager qryManager = calloc (1, sizeof (struct BRCryptoClientQRYManagerRecord));

    qryManager->client  = client;
    qryManager->manager = manager;

    return qryManager;
}

extern void
cryptoClientQRYManagerRelease (BRCryptoClientQRYManager qry) {
    memset (qry, 0, sizeof(*qry));
    free (qry);
}

static void
cryptoClientQRYManagerSync (BRCryptoClientQRYManager qry, BRCryptoSyncDepth depth) {

}

static void
cryptoClientQRYManagerSend (BRCryptoClientQRYManager qry, BRCryptoTransfer transfer) {
//    typedef void
//    (*BRCryptoClientSubmitTransactionCallback) (BRCryptoClientContext context,
//                                                OwnershipGiven BRCryptoWalletManager manager,
//                                                OwnershipGiven BRCryptoClientCallbackState callbackState,
//                                                OwnershipKept const uint8_t *transaction,
//                                                size_t transactionLength,
//                                                OwnershipKept const char *hashAsHex);

//    qry->client.funcSubmitTransaction (qry->client.context,
//                                       cwm,
//             );
    
}

