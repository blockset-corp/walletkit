//
//  BRStellarOperation.h
//  WalletKitCore
//
//  Created by Carl Cherry on 6/12/2019.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "BRStellarOperation.h"
#include <stdlib.h>

extern BRStellarOperation
stellarOperationCreatePayment(struct BRStellarAddressRecord *destination, BRStellarAsset asset, BRStellarAmount amount)
{
    BRStellarOperation op;
    memset(&op, 0x00, sizeof(BRStellarOperation));
    op.type = ST_OP_PAYMENT;
    op.operation.payment.destination.accountType = PUBLIC_KEY_TYPE_ED25519;
    memcpy(op.operation.payment.destination.accountID, destination->bytes, STELLAR_ADDRESS_BYTES);
    op.operation.payment.amount = amount;
    op.operation.payment.asset = asset;
    return op;
}

extern BRStellarAsset stellarAssetCreateAsset(const char* assetCode,
                                              struct BRStellarAddressRecord *issuer)
{
    BRStellarAsset asset;
    memset(&asset, 0x00, sizeof(BRStellarAsset));

    // https://www.stellar.org/developers/guides/concepts/assets.html
    // Assets come in 3 forms
    // 1. Native XML
    // 2. An asset code that is up to 4 ASCII characters
    // 3. An asset code that is between 5 and 12 characters
    if (0 == strcmp("XML", assetCode)) {
        asset.type = ASSET_TYPE_NATIVE;
    } else {
        if (strlen(assetCode) <= 4) {
            asset.type = ASSET_TYPE_CREDIT_ALPHANUM4;
            strncpy(asset.assetCode, assetCode, strlen(assetCode));
            // We need an issuer in this case
            asset.issuer.accountType = PUBLIC_KEY_TYPE_ED25519;
            memcpy(asset.issuer.accountID, issuer->bytes, STELLAR_ADDRESS_BYTES);
        } else {
            // If the caller passes in a code larger that 12 chacters just copy
            // the first 12 and let the Stellar server respond with an error if
            // the asset type is unknown
            size_t amountToCopy = strlen(assetCode) <= 12 ? strlen(assetCode) : 12;
            asset.type = ASSET_TYPE_CREDIT_ALPHANUM12;
            strncpy(asset.assetCode, assetCode, amountToCopy);
            asset.issuer.accountType = PUBLIC_KEY_TYPE_ED25519;
            memcpy(asset.issuer.accountID, issuer->bytes, STELLAR_ADDRESS_BYTES);
        }
    }
    return asset;
}

