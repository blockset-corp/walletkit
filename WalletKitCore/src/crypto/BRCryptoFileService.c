//
//  BRCryptoFileService.c
//  Core
//
//  Created by Ehsan Rezaie on 2020-05-14.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "BRCryptoFileService.h"

private_extern UInt256
fileServiceTypeTransferV1Identifier (BRFileServiceContext context,
                                     BRFileService fs,
                                     const void *entity) {
#ifdef REFACTOR
    BRGenericTransfer transfer = (BRGenericTransfer) entity;
    BRGenericHash     hash     = genTransferGetHash(transfer);

    assert (hash.bytesCount >= sizeof(UInt256));
    UInt256 *result = (UInt256*) hash.bytes;

    return *result;
#endif
    return UINT256_ZERO;
}

private_extern void *
fileServiceTypeTransferV1Reader (BRFileServiceContext context,
                                 BRFileService fs,
                                 uint8_t *bytes,
                                 uint32_t bytesCount) {
#ifdef REFACTOR
    BRGenericManager gwm = (BRGenericManager) context;

    BRRlpCoder coder = rlpCoderCreate();
    BRRlpData  data  = (BRRlpData) { bytesCount, bytes };
    BRRlpItem  item  = rlpDataGetItem (coder, data);

    size_t itemsCount;
    const BRRlpItem *items = rlpDecodeList (coder, item, &itemsCount);
    assert (9 == itemsCount);

    BRRlpData hashData = rlpDecodeBytes(coder, items[0]);
    char *strUids   = rlpDecodeString  (coder, items[1]);
    char *strSource = rlpDecodeString  (coder, items[2]);
    char *strTarget = rlpDecodeString  (coder, items[3]);
    UInt256 amount  = rlpDecodeUInt256 (coder, items[4], 0);
    char *currency  = rlpDecodeString  (coder, items[5]);
    BRGenericFeeBasis feeBasis = genFeeBasisDecode (items[6], coder);
    BRGenericTransferState state = genTransferStateDecode (items[7], coder);
    BRArrayOf(BRGenericTransferAttribute) attributes = genTransferAttributesDecode(items[8], coder);

    BRGenericHash hash = genericHashCreate(hashData.bytesCount, hashData.bytes);

    char *strHash   = genericHashAsString (hash);
    char *strAmount = uint256CoerceString (amount, 10);

    int overflow = 0;
    UInt256 fee = genFeeBasisGetFee (&feeBasis, &overflow);
    assert (!overflow);
    char *strFee = uint256CoerceString (fee,    10);

    uint64_t timestamp = (GENERIC_TRANSFER_STATE_INCLUDED == state.type
                          ? state.u.included.timestamp
                          : GENERIC_TRANSFER_TIMESTAMP_UNKNOWN);

    uint64_t blockHeight = (GENERIC_TRANSFER_STATE_INCLUDED == state.type
                            ? state.u.included.blockNumber
                            : GENERIC_TRANSFER_BLOCK_NUMBER_UNKNOWN);

    // Derive `wallet` from currency
    BRGenericWallet  wallet = genManagerGetPrimaryWallet (gwm);

    BRGenericTransfer transfer = genManagerRecoverTransfer (gwm, wallet, strHash,
                                                            strUids,
                                                            strSource,
                                                            strTarget,
                                                            strAmount,
                                                            currency,
                                                            strFee,
                                                            timestamp,
                                                            blockHeight,
                                                            GENERIC_TRANSFER_STATE_ERRORED == state.type);

    // Set the transfer's `state` and `attributes` from the read values.  For`state`, this will
    // overwrite what `genManagerRecoverTransfer()` assigned but will be correct with the saved
    // values.  Later, perhaps based on a BlocksetDB query, the state change to 'included error'.
    genTransferSetState (transfer, state);
    genTransferSetAttributes (transfer, attributes);

    genTransferAttributeReleaseAll(attributes);
    free (strFee);
    free (strAmount);
    free (strHash);
    free (currency);
    free (strTarget);
    free (strSource);
    free (strUids);

    rlpItemRelease (coder, item);
    rlpCoderRelease(coder);

    return transfer;
#endif
    return NULL;
}

private_extern uint8_t *
fileServiceTypeTransferWriter (BRFileServiceContext context,
                               BRFileService fs,
                               const void* entity,
                               uint32_t *bytesCount,
                               BRGenericFileServiceTransferVersion version) {
#ifdef REFACTOR
    BRGenericTransfer transfer = (BRGenericTransfer) entity;

    BRGenericHash    hash   = genTransferGetHash (transfer);
    BRGenericAddress source = genTransferGetSourceAddress (transfer);
    BRGenericAddress target = genTransferGetTargetAddress (transfer);
    UInt256 amount = genTransferGetAmount (transfer);
    BRGenericFeeBasis feeBasis = genTransferGetFeeBasis(transfer);
    BRGenericTransferState state = genTransferGetState (transfer);

    // Code it Up!
    BRRlpCoder coder = rlpCoderCreate();

    char *strSource = genAddressAsString(source);
    char *strTarget = genAddressAsString(target);

    BRGenericTransferStateEncodeVersion stateEncodeVersion =
        (GENERIC_TRANSFER_VERSION_1 == version ? GEN_TRANSFER_STATE_ENCODE_V1
         : (GENERIC_TRANSFER_VERSION_2 == version ? GEN_TRANSFER_STATE_ENCODE_V2
            : GEN_TRANSFER_STATE_ENCODE_V1));

    BRRlpItem item = rlpEncodeList (coder, 9,
                                    rlpEncodeBytes (coder, hash.bytes, hash.bytesCount),
                                    rlpEncodeString (coder, transfer->uids),
                                    rlpEncodeString (coder, strSource),
                                    rlpEncodeString (coder, strTarget),
                                    rlpEncodeUInt256 (coder, amount, 0),
                                    rlpEncodeString (coder, cryptoNetworkCanonicalTypeGetCurrencyCode(transfer->type)),
                                    genFeeBasisEncode (feeBasis, coder),
                                    genTransferStateEncode (state, stateEncodeVersion, coder),
                                    genTransferAttributesEncode (transfer->attributes, coder));

    BRRlpData data = rlpItemGetData (coder, item);

    rlpItemRelease (coder, item);
    rlpCoderRelease (coder);

    free (strSource); genAddressRelease (source);
    free (strTarget); genAddressRelease (target);

    *bytesCount = (uint32_t) data.bytesCount;
    return data.bytes;
#endif
    return NULL;
}

private_extern uint8_t *
fileServiceTypeTransferV1Writer (BRFileServiceContext context,
                                 BRFileService fs,
                                 const void* entity,
                                 uint32_t *bytesCount) {
    return fileServiceTypeTransferWriter (context, fs, entity, bytesCount, GENERIC_TRANSFER_VERSION_1);
}

private_extern uint8_t *
fileServiceTypeTransferV2Writer (BRFileServiceContext context,
                                 BRFileService fs,
                                 const void* entity,
                                 uint32_t *bytesCount) {
    return fileServiceTypeTransferWriter (context, fs, entity, bytesCount, GENERIC_TRANSFER_VERSION_2);
}

