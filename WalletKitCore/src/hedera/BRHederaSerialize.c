//
//  BRHederaSerialize.c
//
//  Created by Carl Cherry on Oct. 17, 2019.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "BRHederaSerialize.h"
#include "BRHederaAddress.h"
#include "BRHederaToken.h"
#include "proto/Transaction.pb-c.h"
#include "proto/TransactionBody.pb-c.h"
#include "proto/TransactionContents.pb-c.h"
#include <stdlib.h>

const size_t max_memo_size = 100L;

Proto__AccountID * createAccountID (BRHederaAddress address)
{
    Proto__AccountID *protoAccountID = calloc(1, sizeof(Proto__AccountID));
    proto__account_id__init(protoAccountID);
    protoAccountID->shardnum = hederaAddressGetShard (address);
    protoAccountID->realmnum = hederaAddressGetRealm (address);
    protoAccountID->accountnum = hederaAddressGetAccount (address);
    return protoAccountID;
}

Proto__TokenID * createTokenID (BRHederaAddress address)
{
    Proto__TokenID *tokenId = calloc(1, sizeof(Proto__TokenID));
    proto__token_id__init(tokenId);
    tokenId->shardnum = hederaAddressGetShard (address);
    tokenId->realmnum = hederaAddressGetRealm (address);
    tokenId->tokennum = hederaAddressGetAccount (address);
    return tokenId;
}

Proto__Timestamp * createTimeStamp  (BRHederaTimeStamp timeStamp)
{
    Proto__Timestamp *ts = calloc(1, sizeof(Proto__Timestamp));
    proto__timestamp__init(ts);
    ts->seconds = timeStamp.seconds;
    ts->nanos = timeStamp.nano;
    return ts;
}

Proto__TransactionID * createProtoTransactionID (BRHederaAddress address, BRHederaTimeStamp timeStamp)
{
    Proto__TransactionID *txID = calloc(1, sizeof(Proto__TransactionID));
    proto__transaction_id__init(txID);
    txID->transactionvalidstart = createTimeStamp(timeStamp);
    txID->accountid = createAccountID(address);

    return txID;
}

Proto__Duration * createTransactionDuration(int64_t seconds)
{
    Proto__Duration * duration = calloc(1, sizeof(Proto__Duration));
    proto__duration__init(duration);
    duration->seconds = seconds;
    return duration;
}

Proto__AccountAmount * createAccountAmount (BRHederaAddress address, int64_t amount)
{
    Proto__AccountAmount * accountAmount = calloc(1, sizeof(Proto__AccountAmount));
    proto__account_amount__init(accountAmount);
    accountAmount->accountid = createAccountID(address);
    accountAmount->amount = amount;
    return accountAmount;
}

void addNativeTransfers(Proto__CryptoTransferTransactionBody * cryptoTransfer,
                        BRHederaAddress source,
                        BRHederaAddress target, BRHederaAmount amount)
{
    cryptoTransfer->transfers = calloc(1, sizeof(Proto__TransferList));
    proto__transfer_list__init(cryptoTransfer->transfers);

    // We are only supporting sending from A to B at this point - so create 2 transfers
    cryptoTransfer->transfers->n_accountamounts = 2;
    cryptoTransfer->transfers->accountamounts = calloc(2, sizeof(Proto__AccountAmount*));
    // NOTE - the amounts in the transfer MUST add up to 0
    // Also note we don't add in FEE transfers, the Hedera server does that
    cryptoTransfer->transfers->accountamounts[0] = createAccountAmount(source, -(amount));
    cryptoTransfer->transfers->accountamounts[1] = createAccountAmount(target, amount);
}

void addTokenTransfers(Proto__CryptoTransferTransactionBody * cryptoTransfer,
                       BRHederaAddress tokenAddress,
                       BRHederaAddress source,
                       BRHederaAddress target,
                       BRHederaAmount amount)
{
    // Create the token transfer list - the protocol support an array of token transfer list
    // since it would be possible to send different token types in the same transaction
    // We are only supporting a single token per transation so the list size is 1
    cryptoTransfer->n_tokentransfers = 1;

    // Create the one and only transfer list
    cryptoTransfer->tokentransfers = calloc(1, sizeof(Proto__TokenTransferList*));
    cryptoTransfer->tokentransfers[0] = calloc(1, sizeof(Proto__TokenTransferList));
    proto__token_transfer_list__init(cryptoTransfer->tokentransfers[0]);

    // Now add the token, and transfers to our one and only list
    cryptoTransfer->tokentransfers[0]->n_transfers = 2; // Always 2 for from and to addresses
    cryptoTransfer->tokentransfers[0]->token = createTokenID(tokenAddress);
    cryptoTransfer->tokentransfers[0]->transfers = calloc(2, sizeof(Proto__AccountAmount*));
    // NOTE - the amounts in the transfer MUST add up to 0
    cryptoTransfer->tokentransfers[0]->transfers[0] = createAccountAmount(source, -(amount));
    cryptoTransfer->tokentransfers[0]->transfers[1] = createAccountAmount(target, amount);
}

uint8_t * hederaTransactionBodyPack (BRHederaAddress source,
                                     BRHederaAddress target,
                                     BRHederaAddress nodeAddress,
                                     BRHederaAmount amount,
                                     BRHederaTimeStamp timeStamp,
                                     BRHederaUnitTinyBar fee,
                                     const char * memo,
                                     BRHederaToken token,
                                     size_t *size)
{
    Proto__TransactionBody *body = calloc(1, sizeof(Proto__TransactionBody));
    proto__transaction_body__init(body);

    // Create a transaction ID
    body->transactionid = createProtoTransactionID(source, timeStamp);
    body->nodeaccountid = createAccountID(nodeAddress);
    body->transactionfee = (uint64_t)fee; // The max we are willing to pay

    // Docs say the limit of 100 is enforced. The max size of not defined
    // in the .proto file so I guess we just have to trust that it is string with max 100 chars
    if (memo) body->memo = strndup(memo, max_memo_size);

    // Set the duration
    // *** NOTE 1 *** if the transaction is unable to be verified in this
    // duration then it will fail. The default value in the Hedera Java SDK
    // is 120. I have set ours to 180 since it requires a couple of extra hops
    body->transactionvalidduration = createTransactionDuration(180);

    // We are creating a "Cryto Transfer" transaction which has a transfer list
    body->data_case =  PROTO__TRANSACTION_BODY__DATA_CRYPTO_TRANSFER;
    body->cryptotransfer = calloc(1, sizeof(Proto__CryptoTransferTransactionBody));
    proto__crypto_transfer_transaction_body__init(body->cryptotransfer);

    // We only support native OR token - never both in the same transaction
    if (token) {
        addTokenTransfers(body->cryptotransfer, hederaTokenGetAddress(token), source, target, amount);
    } else {
        addNativeTransfers(body->cryptotransfer, source, target, amount);
    }

    // Serialize the transaction body
    *size = proto__transaction_body__get_packed_size(body);
    uint8_t * buffer = calloc(1, *size);
    proto__transaction_body__pack(body, buffer);

    // Free the body object now that we have serialized to bytes
    proto__transaction_body__free_unpacked(body, NULL);

    return buffer;
}

Proto__SignatureMap * createSigMap(uint8_t *signature, uint8_t * publicKey)
{
    Proto__SignatureMap * sigMap = calloc(1, sizeof(Proto__SignatureMap));
    proto__signature_map__init(sigMap);
    sigMap->sigpair = calloc(1, sizeof(Proto__SignaturePair*)); // A single signature
    sigMap->sigpair[0] = calloc(1, sizeof(Proto__SignaturePair));
    proto__signature_pair__init(sigMap->sigpair[0]);
    sigMap->sigpair[0]->signature_case = PROTO__SIGNATURE_PAIR__SIGNATURE_ED25519;

    // We need to take copies of the the following fields or else we
    // get a double free assert.  They will get freed later in the call
    // to proto__transaction__free_unpacked
    uint8_t * pubKeyBuffer = calloc(1, 32);
    memcpy(pubKeyBuffer, publicKey, 32);
    uint8_t * sigCopy = calloc(1, 64);
    memcpy(sigCopy, signature, 64);

    sigMap->sigpair[0]->pubkeyprefix.data = pubKeyBuffer;
    sigMap->sigpair[0]->pubkeyprefix.len = 32;
    sigMap->sigpair[0]->ed25519.data = sigCopy;
    sigMap->sigpair[0]->ed25519.len = 64;
    sigMap->n_sigpair = 1;

    return sigMap;
}

uint8_t * hederaTransactionPack (uint8_t * signature, size_t signatureSize,
                                      uint8_t * publicKey, size_t publicKeySize,
                                      uint8_t * body, size_t bodySize,
                                      size_t * serializedSize)
{
    // Create a sigmap object for this transaction
    Proto__SignatureMap * sigMap = createSigMap(signature, publicKey);

    Proto__SignedTransaction * signedTx = calloc(1, sizeof(struct Proto__SignedTransaction));
    proto__signed_transaction__init(signedTx);
    signedTx->sigmap = sigMap;

    // The call to free_unpacked below will delete the .data field, so take
    // a copy here so we don't get a double free
    uint8_t * bodyCopy = calloc(1, bodySize);
    memcpy(bodyCopy, body, bodySize);

    signedTx->bodybytes.data = bodyCopy;
    signedTx->bodybytes.len = bodySize;

    // Get the packed bytes
    *serializedSize = proto__signed_transaction__get_packed_size(signedTx);
    uint8_t * signedBytes = calloc(1, *serializedSize);
    proto__signed_transaction__pack(signedTx, signedBytes);

    // Free the transaction now that we have serialized to bytes
    proto__signed_transaction__free_unpacked(signedTx, NULL);

    return signedBytes;
}
