//
//  BREthereumSignature.c
//  WalletKitCore
//
//  Created by Ed Gamble on 5/17/18.
//  Copyright © 2018-2019 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include <assert.h>
#include "support/BRCrypto.h"
#include "BREthereumSignature.h"

//
// Signature
//
extern BREthereumSignature
ethSignatureCreateFromDigest (BREthereumSignatureType type,
                              BREthereumHash digest,
                              BRKey privateKeyUncompressed) {
    UInt256 digestAsUInt256;
    memcpy (digestAsUInt256.u8, digest.bytes, 32);

    BREthereumSignature signature;

    // Save the type.
    signature.type = type;

    switch (type) {
        case SIGNATURE_TYPE_RECOVERABLE_VRS_EIP: {
            // Determine the signature length
            size_t signatureLen = BRKeyCompactSign (&privateKeyUncompressed,
                                                    NULL, 0,
                                                    digestAsUInt256);

            // Fill the signature
            uint8_t signatureBytes[signatureLen];
            signatureLen = BRKeyCompactSign (&privateKeyUncompressed,
                                             signatureBytes, signatureLen,
                                             digestAsUInt256);
            assert (65 == signatureLen);

            // The actual 'signature' is one byte added to secp256k1_ecdsa_recoverable_signature
            // and secp256k1_ecdsa_recoverable_signature is 64 bytes as {r[32], s32]}

            // Extract V, R, and S
            signature.sig.vrs.v = signatureBytes[0];
            memcpy(signature.sig.vrs.r, &signatureBytes[ 1], 32);
            memcpy(signature.sig.vrs.s, &signatureBytes[33], 32);

            // TODO: Confirm signature
            // assigns pubKey recovered from compactSig to key and returns true on success
            // int BRKeyRecoverPubKey(BRKey *key, UInt256 md, const void *compactSig, size_t sigLen)

            break;
        }

        case SIGNATURE_TYPE_RECOVERABLE_RSV: {
            // Determine the signature length
            size_t signatureLen = BRKeyCompactSignEthereum (&privateKeyUncompressed,
                                                            NULL, 0,
                                                            digestAsUInt256);

            // Fill the signature
            uint8_t signatureBytes[signatureLen];
            signatureLen = BRKeyCompactSignEthereum (&privateKeyUncompressed,
                                                     signatureBytes, signatureLen,
                                                     digestAsUInt256);
            assert (65 == signatureLen);

            // The actual 'signature' is one byte added to secp256k1_ecdsa_recoverable_signature
            // and secp256k1_ecdsa_recoverable_signature is 64 bytes as {r[32], s32]}

            // Extract V, R, and S
            memcpy(signature.sig.rsv.r, &signatureBytes[ 0], 32);
            memcpy(signature.sig.rsv.s, &signatureBytes[32], 32);
            signature.sig.rsv.v = signatureBytes[64];

            break;
        }

        case SIGNATURE_TYPE_RECOVERABLE_RSV_ECDSA: {


            // Determine the signature length
            size_t signatureLen = BRKeyCompactSignEthereumEcsign (&privateKeyUncompressed,
                                                                  NULL, 0,
                                                                  digestAsUInt256);
            // Fill the signature
            uint8_t signatureBytes[signatureLen];
            signatureLen = BRKeyCompactSignEthereumEcsign (&privateKeyUncompressed,
                                                           signatureBytes, signatureLen,
                                                           digestAsUInt256);
            assert (65 == signatureLen);

            // The actual 'signature' is one byte added to secp256k1_ecdsa_recoverable_signature
            // and secp256k1_ecdsa_recoverable_signature is 64 bytes as {r[32], s32]}

            // Extract V, R, and S
            memcpy(signature.sig.rsv.r, &signatureBytes[ 0], 32);
            memcpy(signature.sig.rsv.s, &signatureBytes[32], 32);
            signature.sig.rsv.v = signatureBytes[64];

            break;
        }
    }

    return signature;
}

extern BREthereumSignature
ethSignatureCreate(BREthereumSignatureType type,
                   const uint8_t *bytes,
                   size_t bytesCount,
                   BRKey privateKeyUncompressed,
                   BREthereumHash *digestRef) {

    // Hash with the required Keccak-256
    BREthereumHash digest = ethHashCreateFromBytes (bytes, bytesCount);
    if (NULL != digestRef) *digestRef = digest;
    return ethSignatureCreateFromDigest (type, digest, privateKeyUncompressed);
}

extern BREthereumBoolean
ethSignatureEqual (BREthereumSignature s1, BREthereumSignature s2) {
    return (s1.type == s2.type &&
            s1.sig.vrs.v == s2.sig.vrs.v &&
            0 == memcmp (s1.sig.vrs.r, s2.sig.vrs.r, 32) &&
            0 == memcmp (s1.sig.vrs.s, s2.sig.vrs.s, 32)
            ? ETHEREUM_BOOLEAN_TRUE
            : ETHEREUM_BOOLEAN_FALSE);
}

extern BREthereumAddress
ethSignatureExtractAddress(const BREthereumSignature signature,
                           const uint8_t *bytes,
                           size_t bytesCount,
                           int *success) {
    assert (NULL != success);

    UInt256 digest;
    BRKeccak256 (&digest, bytes, bytesCount);

    BRKey key;

    switch (signature.type) {
        case SIGNATURE_TYPE_RECOVERABLE_VRS_EIP:
            *success = BRKeyRecoverPubKey (&key, digest,
                                           &signature.sig.vrs,
                                           sizeof (signature.sig.vrs));
            break;
        case SIGNATURE_TYPE_RECOVERABLE_RSV:
        case SIGNATURE_TYPE_RECOVERABLE_RSV_ECDSA:
            *success = BRKeyRecoverPubKeyEthereum (&key, digest,
                                                   &signature.sig.rsv,
                                                   sizeof (signature.sig.rsv));
            break;
    }

    return (0 == *success
            ? (BREthereumAddress) ETHEREUM_EMPTY_ADDRESS_INIT
            : ethAddressCreateKey(&key));
}

extern void
ethSignatureClear (BREthereumSignature *s,
                   BREthereumSignatureType type) {
    memset (s, 0, sizeof (BREthereumSignature));
    s->type = type;
}
