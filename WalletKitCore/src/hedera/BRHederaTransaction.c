//
//  BRHederaTransaction.c
//  Core
//
//  Created by Carl Cherry on Oct. 16, 2019.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "BRHederaTransaction.h"
#include "BRHederaCrypto.h"
#include "BRHederaSerialize.h"
#include "ed25519/ed25519.h"
#include "support/BRArray.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <string.h>
#include "support/BRInt.h"

// Here is the list of Hedera nodes used in production
static uint16_t nodes[10] = {
    3, 4, 5, 6, 7, 8, 9, 10, 11, 12
};
static uint16_t numNodes = sizeof(nodes) / sizeof(uint16_t);

// Forward declarations
int hederaTransactionGetHashCount (BRHederaTransaction transaction);
BRHederaTransactionHash hederaTransactionGetHashAtIndex (BRHederaTransaction transaction, int index);

struct BRHederaTransactionRecord {
    BRHederaAddress source;
    BRHederaAddress target;
    BRHederaUnitTinyBar amount;
    char * transactionId;
    BRHederaUnitTinyBar fee;
    uint8_t * serializedBytes;
    size_t serializedSize;
    BRHederaTransactionHash hash;
    BRHederaFeeBasis feeBasis;
    BRHederaTimeStamp timeStamp;
    uint64_t blockHeight;
    int error;
    char * memo;
    bool hashIsSet;
    BRArrayOf(BRHederaTransactionHash) hashes;
};

char * createTransactionID(BRHederaAddress address, BRHederaTimeStamp timeStamp)
{
    char buffer[128] = {0};
    const char * hederaAddress = hederaAddressAsString(address);
    sprintf(buffer, "%s-%" PRIi64 "-%" PRIi32, hederaAddress, timeStamp.seconds, timeStamp.nano);
    char * result = calloc(1, strlen(buffer) + 1);
    strncpy(result, buffer, strlen(buffer));
    return result;
}

extern BRHederaTransaction hederaTransactionCreateNew (BRHederaAddress source,
                                                       BRHederaAddress target,
                                                       BRHederaUnitTinyBar amount,
                                                       BRHederaFeeBasis feeBasis,
                                                       BRHederaTimeStamp *timeStamp)
{
    BRHederaTransaction transaction = calloc (1, sizeof(struct BRHederaTransactionRecord));
    transaction->source = hederaAddressClone (source);
    transaction->target = hederaAddressClone (target);
    transaction->amount = amount;
    transaction->feeBasis = feeBasis;
    transaction->fee = hederaFeeBasisGetFee (&transaction->feeBasis);
    if (timeStamp != NULL) {
        transaction->timeStamp = *timeStamp;
    } else {
        // Generate a timestamp - hedera uses the timestamp in the transaction ID
        // and includes seconds and nano-seconds
        transaction->timeStamp = hederaGenerateTimeStamp();
    }
    transaction->transactionId = createTransactionID(source, transaction->timeStamp);
    transaction->blockHeight = 0;
    transaction->error = 0;
    return transaction;
}

extern BRHederaTransaction hederaTransactionCreate (BRHederaAddress source,
                                                    BRHederaAddress target,
                                                    BRHederaUnitTinyBar amount,
                                                    BRHederaUnitTinyBar fee,
                                                    const char * txID,
                                                    BRHederaTransactionHash hash,
                                                    uint64_t timestamp, uint64_t blockHeight,
                                                    int error)
{
    // This is an existing transaction - it must have a transaction ID
    assert(source);
    assert(target);
    BRHederaTransaction transaction = calloc (1, sizeof(struct BRHederaTransactionRecord));
    transaction->source = hederaAddressClone (source);
    transaction->target = hederaAddressClone (target);
    transaction->amount = amount;
    transaction->feeBasis = (BRHederaFeeBasis) { fee, 1 };
    transaction->fee = hederaFeeBasisGetFee(&transaction->feeBasis);

    // Parse the transactionID
    if (txID && strlen(txID) > 1) {
        transaction->transactionId = (char*) calloc(1, strlen(txID) + 1);
        strcpy(transaction->transactionId, txID);
        // Get the timestamp from the transaction id
        transaction->timeStamp = hederaParseTimeStamp(txID);
    } else {
        // It would great to be able to get the timestamp from the txID - but I guess
        // we just have to use whatever came from blockset
        transaction->timeStamp.seconds = (int64_t) timestamp;
    }

    transaction->hash = hash;
    transaction->hashIsSet = true;
    transaction->blockHeight = blockHeight;
    transaction->error = error;

    return transaction;
}

extern BRHederaTransaction
hederaTransactionClone (BRHederaTransaction transaction)
{
    assert(transaction);
    BRHederaTransaction newTransaction = calloc (1, sizeof(struct BRHederaTransactionRecord));
    newTransaction->source = hederaTransactionGetSource(transaction);
    newTransaction->target = hederaTransactionGetTarget(transaction);
    newTransaction->amount = hederaTransactionGetAmount(transaction);
    newTransaction->fee = transaction->fee;
    newTransaction->feeBasis = transaction->feeBasis;

    if (transaction->transactionId && strlen(transaction->transactionId) > 1) {
        newTransaction->transactionId = (char*) calloc(1, strlen(transaction->transactionId) + 1);
        strcpy(newTransaction->transactionId, transaction->transactionId);
    }

    newTransaction->hash = transaction->hash;
    newTransaction->blockHeight = transaction->blockHeight;
    newTransaction->timeStamp = transaction->timeStamp;

    if (transaction->serializedSize > 0 && transaction->serializedBytes) {
        newTransaction->serializedSize = transaction->serializedSize;
        newTransaction->serializedBytes = calloc(1, newTransaction->serializedSize);
        memcpy(newTransaction->serializedBytes, transaction->serializedBytes, newTransaction->serializedSize);
    }

    if (transaction->memo) {
        newTransaction->memo = strdup(transaction->memo);
    }

    if (transaction->hashes) {
        array_new(newTransaction->hashes, array_count(transaction->hashes));
        array_add_array(newTransaction->hashes, transaction->hashes, array_count(transaction->hashes));
    }

    newTransaction->hashIsSet = transaction->hashIsSet;

    return newTransaction;
}

extern void hederaTransactionFree (BRHederaTransaction transaction)
{
    assert (transaction);
    if (transaction->serializedBytes) free (transaction->serializedBytes);
    if (transaction->transactionId) free (transaction->transactionId);
    if (transaction->source) hederaAddressFree (transaction->source);
    if (transaction->target) hederaAddressFree (transaction->target);
    if (transaction->memo) free(transaction->memo);
    if (transaction->hashes) array_free(transaction->hashes);
    free (transaction);
}

extern void hederaTransactionSetMemo(BRHederaTransaction transaction, const char* memo)
{
    assert(transaction);
    assert(memo);
    transaction->memo = strdup(memo);
}

extern char * // Caller owns memory and must free calling "free"
hederaTransactionGetMemo(BRHederaTransaction transaction)
{
    assert(transaction);
    if (transaction->memo && strlen(transaction->memo) > 0) {
        return strdup(transaction->memo);
    }
    return NULL;
}

static uint8_t* hederaTransactionSignTransactionWithNode (BRHederaTransaction transaction,
                                                          BRKey publicKey,
                                                          const unsigned char * privateKey,
                                                          BRHederaAddress node,
                                                          BRHederaUnitTinyBar fee,
                                                          size_t *bufferSize)
{
    // First we need to serialize the body since it is the thing we sign
    size_t bodySize;
    uint8_t * body = hederaTransactionBodyPack (transaction->source,
                                                transaction->target,
                                                node,
                                                transaction->amount,
                                                transaction->timeStamp,
                                                fee,
                                                transaction->memo,
                                                &bodySize);

    // Create signature from the body bytes
    unsigned char signature[64];
    memset (signature, 0x00, 64);
    ed25519_sign(signature, body, bodySize, publicKey.pubKey, privateKey);

    // Serialize the full transaction including signature and public key
    uint8_t * serializedBytes = hederaTransactionPack (signature, 64,
                                                       publicKey.pubKey, 32,
                                                       body, bodySize,
                                                       bufferSize);
    free(body);
    return serializedBytes;
}

static size_t
hederaTransactionSignMultipleSerializations (BRHederaTransaction transaction, BRKey publicKey,
                                             const unsigned char *privateKey, BRHederaUnitTinyBar fee)
{
    // Create a serialization for all the known nodes - currently 3 through 12
    // defined in the "nodes" array
    if (transaction->hashes) array_free(transaction->hashes);
    array_new(transaction->hashes, numNodes);
    transaction->serializedSize = 0;
    uint8_t * pSerializedBytes = NULL;
    for (uint16_t i = 0; i < numNodes; i++) {
        size_t size = 0;
        BRHederaAddress node = hederaAddressCreate(0, 0, (int64_t)nodes[i]);
        uint8_t * signedBytes = hederaTransactionSignTransactionWithNode(transaction, publicKey, privateKey,
                                                                         node, fee, &size);

        // Store the hash for this serialization
        BRHederaTransactionHash hash;
        BRSHA384(hash.bytes, signedBytes, size);
        array_add(transaction->hashes, hash);

        if (i == 0) { // First serialization
            // We now have an idea of how much memory is needed for a single serialization
            // - 3 bytes for the header
            // - enough room for NUM_NODES (6 bytes for a header plus serialization)
            // - plus some padding just in case the other serializations are larger - will truncate later
            transaction->serializedBytes = calloc(1, 3 + ((size + 6) * numNodes) + 1024);
            pSerializedBytes = transaction->serializedBytes;

            // Add the header info - version plus the number of serializations
            *pSerializedBytes = (uint8_t)1; // Version 1 of the protocol
            pSerializedBytes++;
            UInt16SetBE(pSerializedBytes, numNodes);
            pSerializedBytes += 2; // Pointer to after the header
        }
        // Add the node number, the size of the serialization, then the
        // serialized bytes for this node
        UInt16SetBE(pSerializedBytes, nodes[i]);
        UInt32SetBE(pSerializedBytes + 2, (uint32_t)size);
        memcpy(pSerializedBytes + 6, signedBytes, size);
        pSerializedBytes += (6 + size);

        free(signedBytes);
        hederaAddressFree(node);
    }

    // Calculate the size using pointer arithmetic
    transaction->serializedSize = (unsigned long)(pSerializedBytes - transaction->serializedBytes);
    // Truncate the buffer to the actual size
    transaction->serializedBytes = realloc(transaction->serializedBytes, transaction->serializedSize);
    return transaction->serializedSize;
}

static size_t
hederaTransactionSignTransactionV0 (BRHederaTransaction transaction,
                                    BRKey publicKey,
                                    const unsigned char *privateKey,
                                    BRHederaUnitTinyBar fee,
                                    BRHederaAddress nodeAddress)
{
    assert (transaction);

    // First we need to serialize the body since it is the thing we sign
    size_t bodySize;
    uint8_t * body = hederaTransactionBodyPack (transaction->source,
                                                transaction->target,
                                                nodeAddress,
                                                transaction->amount,
                                                transaction->timeStamp,
                                                fee,
                                                transaction->memo,
                                                &bodySize);

    // Create signature from the body bytes
    unsigned char signature[64];
    memset (signature, 0x00, 64);
    ed25519_sign(signature, body, bodySize, publicKey.pubKey, privateKey);

    // Serialize the full transaction including signature and public key
    uint8_t * serializedBytes = hederaTransactionPack (signature, 64,
                                                       publicKey.pubKey, 32,
                                                       body, bodySize,
                                                       &transaction->serializedSize);

    // Create the hash from the serialized bytes
    BRSHA384(transaction->hash.bytes, serializedBytes, transaction->serializedSize);

    // We are now done with the body - it was copied to the serialized bytes so we
    // must clean up it now.
    free (body);

    // Due to how the Hedera API works the "node account id" of the server we will submit to
    // is included in the signing data. For V0 of our solution we prepend the node information
    // to the payload so that Blockset will know which node to use when submitting.
    // Serialize the node address
    uint8_t nodeAccountId[HEDERA_ADDRESS_SERIALIZED_SIZE];
    hederaAddressSerialize (nodeAddress, nodeAccountId, HEDERA_ADDRESS_SERIALIZED_SIZE);

    // The buffer has to hold the nodeAccountId and the serialized bytes
    transaction->serializedBytes = calloc(1, transaction->serializedSize + HEDERA_ADDRESS_SERIALIZED_SIZE);

    // First copy the nodeAccountId,
    memcpy(transaction->serializedBytes, nodeAccountId, HEDERA_ADDRESS_SERIALIZED_SIZE);

    // Now copy the serialized transaction bytes after the node account id
    memcpy(transaction->serializedBytes + HEDERA_ADDRESS_SERIALIZED_SIZE, serializedBytes, transaction->serializedSize);

    // Cleanup temporary buffers
    free (serializedBytes);

    // Create the transaction id
    transaction->transactionId = createTransactionID(transaction->source, transaction->timeStamp);

    transaction->serializedSize += HEDERA_ADDRESS_SERIALIZED_SIZE; // This will be our new size of serialized bytes
    return transaction->serializedSize;
}

extern size_t
hederaTransactionSignTransaction (BRHederaTransaction transaction,
                                  BRKey publicKey,
                                  UInt512 seed,
                                  BRHederaAddress nodeAddress)
{
    assert (transaction);

    // If previously signed - delete and resign
    if (transaction->serializedBytes) {
        free (transaction->serializedBytes);
        transaction->serializedSize = 0;
    }

    // Generate the private key from the seed
    BRKey key = hederaKeyCreate (seed);
    unsigned char privateKey[64] = {0};
    unsigned char temp[32] = {0}; // Use the public key that is sent in instead
    ed25519_create_keypair (temp, privateKey, key.secret.u8);
    BRHederaUnitTinyBar fee = hederaFeeBasisGetFee(&transaction->feeBasis);

    if (nodeAddress != NULL) {
        // Create a single sign payload for the specified Hedera node
        return hederaTransactionSignTransactionV0(transaction, publicKey, privateKey, fee, nodeAddress);
    } else {
        // Create a payload with signed serializations for all the (knonw) nodes
        return hederaTransactionSignMultipleSerializations(transaction, publicKey, privateKey, fee);
    }
}

extern uint8_t * hederaTransactionSerialize (BRHederaTransaction transaction, size_t *size)
{
    assert(transaction);
    assert(size);
    if (transaction->serializedBytes) {
        *size = transaction->serializedSize;
        uint8_t * buffer = calloc(1, transaction->serializedSize);
        memcpy(buffer, transaction->serializedBytes, transaction->serializedSize);
        return buffer;
    } else {
        *size = 0;
        return NULL;
    }
}

extern BRHederaTransactionHash hederaTransactionGetHash(BRHederaTransaction transaction)
{
    assert(transaction);
    return transaction->hash;
}

extern char * hederaTransactionGetTransactionId(BRHederaTransaction transaction)
{
    assert(transaction);
    if (transaction->transactionId) {
        return strdup(transaction->transactionId);
    }
    return NULL;
}

extern BRHederaFeeBasis hederaTransactionGetFeeBasis (BRHederaTransaction transaction) {
    assert (transaction);
    return transaction->feeBasis;;
}

extern BRHederaUnitTinyBar hederaTransactionGetFee(BRHederaTransaction transaction)
{
    assert(transaction);
    return transaction->fee;
}

extern BRHederaUnitTinyBar hederaTransactionGetAmount(BRHederaTransaction transaction)
{
    assert(transaction);
    return transaction->amount;
}

extern BRHederaUnitTinyBar hederaTransactionGetAmountDirected (BRHederaTransaction transfer,
                                                               BRHederaAddress address,
                                                               int *negative) {
    BRHederaUnitTinyBar fee    = hederaTransactionGetFee(transfer);
    BRHederaUnitTinyBar amount = (hederaTransactionHasError(transfer)
                                  ? 0
                                  : hederaTransactionGetAmount(transfer));
    
    int isSource = hederaTransactionHasSource (transfer, address);
    int isTarget = hederaTransactionHasTarget (transfer, address);
    
    if (isSource && isTarget) {
        *negative = 1;
        return fee;
    }
    else if (isSource) {
        *negative = 1;
        return amount + fee;
    }
    else if (isTarget) {
        *negative = 0;
        return amount;
    }
    else {
        assert (0);
    }
}

extern BRHederaAddress hederaTransactionGetSource(BRHederaTransaction transaction)
{
    assert(transaction);
    return hederaAddressClone (transaction->source);
}

extern BRHederaAddress hederaTransactionGetTarget(BRHederaTransaction transaction)
{
    assert(transaction);
    return hederaAddressClone (transaction->target);
}

extern int hederaTransactionHasSource (BRHederaTransaction tranaction, BRHederaAddress address) {
    return hederaAddressEqual (tranaction->source, address);
}

extern int hederaTransactionHasTarget (BRHederaTransaction tranaction, BRHederaAddress address) {
    return hederaAddressEqual (tranaction->target, address);
}

extern int hederaTransactionHasError (BRHederaTransaction transaction) {
    assert (transaction);
    return transaction->error;
}

extern BRHederaTimeStamp hederaTransactionGetTimestamp (BRHederaTransaction transaction) {
    assert (transaction);
    return transaction->timeStamp;
}

extern uint64_t hederaTransactionGetBlockheight (BRHederaTransaction transaction) {
    assert (transaction);
    return transaction->blockHeight;
}

extern bool hederaTransactionEqual (BRHederaTransaction t1, BRHederaTransaction t2)
{
    assert(t1);
    assert(t2);
    // Equal means the same transaction id, source, target
    bool result = false;
    // First check the transaction ID
    if (t1->transactionId && t2->transactionId) {
        if (0 == strcmp(t1->transactionId, t2->transactionId)) {
            result = true;
        }
    } else {
        // Transaction IDs are not available - use the hash
        BRHederaTransactionHash hash1 = hederaTransactionGetHash(t1);
        BRHederaTransactionHash hash2 = hederaTransactionGetHash(t2);
        if (memcmp(hash1.bytes, hash2.bytes, sizeof(hash1.bytes)) == 0) {
            // Hash is the same - compare the source
            BRHederaAddress source1 = hederaTransactionGetSource(t1);
            BRHederaAddress source2 = hederaTransactionGetSource(t2);
            if (1 == hederaAddressEqual(source1, source2)) {
                // OK - compare the target
                BRHederaAddress target1 = hederaTransactionGetTarget(t1);
                BRHederaAddress target2 = hederaTransactionGetTarget(t2);
                if (1 == hederaAddressEqual(target1, target2)) {
                    result = true;
                }
                hederaAddressFree(target1);
                hederaAddressFree(target2);
            }
            hederaAddressFree (source1);
            hederaAddressFree (source2);
        }
    }
    return result;
}

static int findHashInList (BRHederaTransactionHash *hash, BRArrayOf(BRHederaTransactionHash) hashes)
{
    for (int i = 0; i < array_count(hashes); i++) {
        if (0 == memcmp(hash, &hashes[i], sizeof(BRHederaTransactionHash))) {
            return 1;
        }
    }
    return 0;
}

extern bool hederaTransactionHashEqual (BRHederaTransaction t1, BRHederaTransaction t2)
{
    // When we submit a transfer to Blockset for Hedera - we are forced to send
    // multiple serializations where each one has a different hash - so we might
    // have to check out all the hashes if applicable
    assert(t1);
    assert(t2);

    // Go through the hash list and find the match
    int hashCount1 = hederaTransactionGetHashCount(t1);
    int hashCount2 = hederaTransactionGetHashCount(t2);

    if (hashCount1 == 0 || hashCount2 == 0) {
        return false;
    } else if (hashCount1 == 1 && hashCount2 == 1) {
        // Just compare the single hash
        BRHederaTransactionHash hash1 = hederaTransactionGetHash(t1);
        BRHederaTransactionHash hash2 = hederaTransactionGetHash(t2);
        return (0 == memcmp(hash1.bytes, hash2.bytes, sizeof(hash1.bytes)));
    } else if (hashCount1 == 1) {
        BRHederaTransactionHash hash = hederaTransactionGetHash(t1);
        return (1 == findHashInList(&hash, t2->hashes));
    } else if (hashCount2 == 1) {
        BRHederaTransactionHash hash = hederaTransactionGetHash(t2);
        return (1 == findHashInList(&hash, t1->hashes));
    } else {
        // Both have multi hash lists - should never happen
        for (int i = 0; i < hashCount1 - 1; i++) {
            BRHederaTransactionHash hash1 = hederaTransactionGetHashAtIndex(t1, i);
            for (int j = 0; j < hashCount2 - 1; j++) {
                BRHederaTransactionHash hash2 = hederaTransactionGetHashAtIndex(t2, j);
                if (memcmp(hash1.bytes, hash2.bytes, sizeof(hash1.bytes)) == 0) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

extern BRHederaTimeStamp hederaGenerateTimeStamp(void)
{
    BRHederaTimeStamp ts;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    ts.seconds = tv.tv_sec;
    ts.nano = tv.tv_usec;
    return ts;
}

BRHederaTimeStamp hederaParseTimeStamp(const char* transactionID)
{
    BRHederaTimeStamp ts;

    // The transaction ID looks like this:
    // block-chain:shard.realm.account-seconds-nanos-index

    // Copy the transactionID and then use strtok to parse
    char * txID = strdup(transactionID);
    const char * searchTokens = ":.";
    strtok(txID, searchTokens); // blockchain-id
    strtok(NULL, searchTokens); // shard
    strtok(NULL, searchTokens); // realm
    char * accountAndTS = strtok(NULL, searchTokens); // account plus rest of the string

    // Now start over and search for the "-" the string now looks like this:
    // account-seconds-nanos-index-whatever
    strtok(accountAndTS, "-"); // Account number
    char * secondsStr = strtok(NULL, "-"); // Seconds
    char * nanosStr = strtok(NULL, "-"); // Nanos

    //sscanf(transactionID, "%s:%lld.%lld.%lld-%lld-%d-", blockchain, &shard, &realm, &account, &seconds, &nanos);
    sscanf(secondsStr, "%" PRIi64, &ts.seconds);
    sscanf(nanosStr, "%" PRIi32, &ts.nano);

    free (txID);
    return ts;
}

int hederaTransactionGetHashCount (BRHederaTransaction transaction) {
    assert(transaction);
    if (transaction->hashIsSet) {
        return 1;
    } else if (transaction->hashes) {
        return (int)array_count(transaction->hashes);
    } else {
        return 0;
    }
}

BRHederaTransactionHash hederaTransactionGetHashAtIndex (BRHederaTransaction transaction, int index) {
    assert(transaction);
    assert(index >= 0 && index < numNodes);
    if (transaction->hashes) {
        return transaction->hashes[index];
    } else {
        return transaction->hash;
    }
}

extern int hederaTransactionUpdateHash (BRHederaTransaction transaction, BRHederaTransactionHash hash) {
    assert(transaction);
    // The hash updating ONLY happens once when we have a SENT transaction waiting to find
    // out which hash was used when submitting - so if we don't have multiple hashes then ignore
    if (transaction->hashes) {
        for (int i = 0; i < array_count(transaction->hashes); i++) {
            if (memcmp(transaction->hashes[i].bytes, hash.bytes, sizeof(hash.bytes)) == 0) {
                // Copy the real hash into our internal hash value
                transaction->hash = hash;
                transaction->hashIsSet = true;

                // Don't need the hashes anymore
                array_free(transaction->hashes);
                transaction->hashes = NULL;

                // If the hash is being updated then that means the transfer was successful
                // and the upper layers have matched up this transfer to the correct hash
                // so we can't send this one again - since it has 10 serializtions it would be
                // better to clean up that memory
                if (transaction->serializedBytes != NULL) {
                    free(transaction->serializedBytes);
                    transaction->serializedBytes = NULL;
                    transaction->serializedSize = 0;
                }
                return 1; // Found it - updated - cleaned up - DONE
            }
        }
    }
    return 0;
}
