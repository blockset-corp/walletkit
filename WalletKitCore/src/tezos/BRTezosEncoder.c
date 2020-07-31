//
//  BRTezosEncoder.c
//  Core
//
//  Created by Ehsan Rezaie on 2020-07-22.
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include <stdint.h>
#include "BRTezosEncoder.h"
#include "BRTezosAddress.h"
#include "BRTezosTransaction.h"
#include "ethereum/util/BRUtilMath.h"
#include "support/BRBase58.h"
#include <stdio.h>
#include <assert.h>


static BRCryptoData
encodeAddress (BRTezosAddress address) {
    // address bytes with 3-byte TZx prefix replaced with a 1-byte prefix
    
    assert (address);
    assert (tezosAddressIsImplicit (address));
    
    size_t len = tezosAddressGetRawSize (address);
    uint8_t bytes[len];
    tezosAddressGetRawBytes (address, bytes, len);
    
    uint8_t prefix;
    
    if (0 == memcmp(bytes, TZ1_PREFIX, sizeof(TZ1_PREFIX)))
        prefix = 0x00;
    else if (0 == memcmp(bytes, TZ2_PREFIX, sizeof(TZ2_PREFIX)))
        prefix = 0x01;
    else if (0 == memcmp(bytes, TZ3_PREFIX, sizeof(TZ1_PREFIX)))
        prefix = 0x02;
    else
        assert(0);
    
    uint8_t encoded[len-2];
    memcpy(&encoded[0], &prefix, 1);
    memcpy(&encoded[1], &bytes[3], len-3);
    
    return cryptoDataCopy (encoded, sizeof(encoded));
}

static BRCryptoData
encodeSource (BRTezosAddress address) {
    // same as address encoding with a 0x0 byte prepended
    BRCryptoData encodedAddress = encodeAddress (address);
    
    uint8_t prefix = 0x00;
    
    uint8_t encoded[encodedAddress.size + 1];
    memcpy(&encoded[0], &prefix, 1);
    memcpy(&encoded[1], encodedAddress.bytes, encodedAddress.size);
    free (encodedAddress.bytes);
    
    return cryptoDataCopy (encoded, sizeof(encoded));
}

static BRCryptoData
encodePublicKey (uint8_t * pubKey) {
    /*
     std::array<uint8_t, 4> prefix = {13, 15, 37, 217};
     auto data = Data(prefix.begin(), prefix.end());
     auto bytes = Data(publicKey.bytes.begin(), publicKey.bytes.end());
     append(data, bytes);

     auto pk = Base58::bitcoin.encodeCheck(data);
     auto decoded = "00" + base58ToHex(pk, 4, prefix.data());
     return parse_hex(decoded);
     */
    //TODO:TEZOS
}

extern BRCryptoData
encodeZarith (int64_t value) {
    assert (value >= 0);
    uint8_t result[32] = {0};
    size_t resultSize = 0;
    
    uint64_t input = (uint64_t)value;
    while (input >= 0x80) {
        result[resultSize++] = (uint8_t)((input & 0xff) | 0x80);
        input >>= 7;
    }
    result[resultSize++] = (uint8_t)input;
    
    return cryptoDataCopy(&result[0], resultSize);
}

static BRCryptoData
encodeBool (bool value) {
    BRCryptoData encoded = cryptoDataNew(1);
    encoded.bytes[0] = value ? 0xff : 0x00;
    return encoded;
}

static BRCryptoData
encodeOperationKind (BRTezosOperationKind kind) {
    uint8_t bytes[1] = { kind };
    return cryptoDataCopy (bytes, 1);
}

extern BRCryptoData
tezosSerializeTransaction (BRTezosTransaction tx) {
    assert (tx);
    
    BRTezosOperationData opData = tezosTransactionGetOperationData(tx);
    
    BRTezosAddress source = tezosTransactionGetSource(tx);
    BRTezosFeeBasis feeBasis = tezosTransactionGetFeeBasis(tx);
    
    size_t maxFields = 9;
    BRCryptoData fields[maxFields];
    size_t numFields = 0;
    fields[numFields++] = encodeOperationKind(opData.kind);
    fields[numFields++] = encodeAddress(source);
    fields[numFields++] = encodeZarith(tezosTransactionGetFee(tx));
    fields[numFields++] = encodeZarith(tezosTransactionGetCounter(tx));
    fields[numFields++] = encodeZarith(feeBasis.gasLimit);
    fields[numFields++] = encodeZarith(feeBasis.storageLimit);
    
    if (TEZOS_OP_TRANSACTION == opData.kind) {
        fields[numFields++] = encodeZarith(opData.u.transaction.amount);
        //TODO:TEZOS support sending to KT addresses
        fields[numFields++] = encodeBool(false); // is originated (KT) address
        fields[numFields++] = encodeAddress(opData.u.transaction.target);
        fields[numFields++] = encodeBool(false); // contract execution params (0x0 for no params)
    } else if (TEZOS_OP_DELEGATION == opData.kind) {
        if (NULL != opData.u.delegation.target) {
            fields[numFields++] = encodeBool(true); // set delegate
            fields[numFields++] = encodeAddress(opData.u.delegation.target);
        } else {
            fields[numFields++] = encodeBool(false); // remove delegate
        }
    } else if (TEZOS_OP_REVEAL == opData.kind) {
        fields[numFields++] = encodePublicKey(opData.u.reveal.publicKey);
    } else {
        // unsupported
        assert(0);
    }
    
    BRCryptoData serialized = cryptoDataConcat(fields, numFields);
    
    tezosAddressFree (source);
    tezosTransactionFreeOperationData(opData);
    for (int i=0; i < numFields; i++) {
        cryptoDataFree(fields[i]);
    }
    
    return serialized;
}
