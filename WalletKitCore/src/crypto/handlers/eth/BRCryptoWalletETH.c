//
//  BRCryptoWalletETH.c
//  Core
//
//  Created by Ed Gamble on 05/07/2020.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "BRCryptoETH.h"
#include "crypto/BRCryptoAmountP.h"

#define DEFAULT_ETHER_GAS_LIMIT    21000ull

extern BRCryptoWalletETH
cryptoWalletCoerce (BRCryptoWallet wallet) {
    assert (CRYPTO_NETWORK_TYPE_ETH == wallet->type);
    return (BRCryptoWalletETH) wallet;
}

private_extern BRCryptoWallet
cryptoWalletCreateAsETH (BRCryptoUnit unit,
                         BRCryptoUnit unitForFee,
                         BREthereumToken   ethToken,
                         BREthereumAccount ethAccount) {
    BRCryptoWallet walletBase = cryptoWalletAllocAndInit (sizeof (struct BRCryptoWalletETHRecord),
                                                      CRYPTO_NETWORK_TYPE_ETH,
                                                      unit,
                                                      unitForFee,
                                                      NULL,
                                                      NULL);
    BRCryptoWalletETH wallet = cryptoWalletCoerce (walletBase);

    wallet->ethAccount  = ethAccount;
    wallet->ethToken    = ethToken;
    wallet->ethGasLimit = (NULL == ethToken
                           ? ethGasCreate (DEFAULT_ETHER_GAS_LIMIT)
                           : ethTokenGetGasLimit (ethToken));
    
    return walletBase;
}

static void
cryptoWalletReleaseETH (BRCryptoWallet wallet) {
}


static BRCryptoAddress
cryptoWalletGetAddressETH (BRCryptoWallet walletBase,
                           BRCryptoAddressScheme addressScheme) {
    BRCryptoWalletETH wallet = cryptoWalletCoerce(walletBase);
    BREthereumAddress ethAddress = ethAccountGetPrimaryAddress (wallet->ethAccount);

    return cryptoAddressCreateAsETH (ethAddress);
}

static bool
cryptoWalletHasAddressETH (BRCryptoWallet walletBase,
                           BRCryptoAddress address) {
    BRCryptoWalletETH wallet = cryptoWalletCoerce (walletBase);
    return (ETHEREUM_BOOLEAN_TRUE == ethAddressEqual (cryptoAddressAsETH (address),
                                                      ethAccountGetPrimaryAddress (wallet->ethAccount)));
}

extern size_t
cryptoWalletGetTransferAttributeCountETH (BRCryptoWallet wallet,
                                          BRCryptoAddress target) {
    return 0;
}

extern BRCryptoTransferAttribute
cryptoWalletGetTransferAttributeAtETH (BRCryptoWallet wallet,
                                       BRCryptoAddress target,
                                       size_t index) {
    return NULL;
}

extern BRCryptoTransferAttributeValidationError
cryptoWalletValidateTransferAttributeETH (BRCryptoWallet wallet,
                                          OwnershipKept BRCryptoTransferAttribute attribute,
                                          BRCryptoBoolean *validates) {
    *validates = CRYPTO_TRUE;
    return (BRCryptoTransferAttributeValidationError) 0;
}

static char *
cryptoTransferProvideOriginatingData (BREthereumTransferBasisType type,
                                      BREthereumAddress targetAddress,
                                      UInt256 value) {
    switch (type) {
        case TRANSFER_BASIS_TRANSACTION:
            return strdup ("");

        case TRANSFER_BASIS_LOG: {
            char address[ADDRESS_ENCODED_CHARS];
            ethAddressFillEncodedString (targetAddress, 0, address);

            // Data is a HEX ENCODED string
            return (char *) ethContractEncode (ethContractERC20, ethFunctionERC20Transfer,
                                               // Address
                                               (uint8_t *) &address[2], strlen(address) - 2,
                                               // Amount
                                               (uint8_t *) &value, sizeof (UInt256),
                                               NULL);
        }
    }
}

static BREthereumAddress
cryptoTransferProvideOriginatingTargetAddress (BREthereumTransferBasisType type,
                                               BREthereumAddress targetAddress,
                                               BREthereumToken   token) {
    switch (type) {
        case TRANSFER_BASIS_TRANSACTION:
            return targetAddress;
        case TRANSFER_BASIS_LOG:
            return ethTokenGetAddressRaw (token);
    }
}

static BREthereumEther
cryptoTransferProvideOriginatingAmount (BREthereumTransferBasisType type,
                                        UInt256 value) {
    switch (type) {
        case TRANSFER_BASIS_TRANSACTION:
            return ethEtherCreate(value);
        case TRANSFER_BASIS_LOG:
            return ethEtherCreateZero ();
    }
}

#if 0
static void
transferProvideOriginatingTransaction (BREthereumTransfer transfer) {
    if (NULL != transfer->originatingTransaction)
        transactionRelease (transfer->originatingTransaction);

    char *data = transferProvideOriginatingTransactionData(transfer);

    transfer->originatingTransaction =
    transactionCreate (transfer->sourceAddress,
                       transferProvideOriginatingTransactionTargetAddress (transfer),
                       transferProvideOriginatingTransactionAmount (transfer),
                       ethFeeBasisGetGasPrice(transfer->feeBasis),
                       ethFeeBasisGetGasLimit(transfer->feeBasis),
                       data,
                       TRANSACTION_NONCE_IS_NOT_ASSIGNED);
    free (data);
}
#endif
extern BRCryptoTransfer
cryptoWalletCreateTransferETH (BRCryptoWallet  walletBase,
                               BRCryptoAddress target,
                               BRCryptoAmount  amount,
                               BRCryptoFeeBasis estimatedFeeBasis,
                               size_t attributesCount,
                               OwnershipKept BRCryptoTransferAttribute *attributes,
                               BRCryptoCurrency currency,
                               BRCryptoUnit unit,
                               BRCryptoUnit unitForFee) {
    BRCryptoWalletETH wallet = cryptoWalletCoerce (walletBase);
    assert (cryptoWalletGetType(walletBase) == cryptoAddressGetType(target));
    assert (cryptoAmountHasCurrency (amount, currency));

    BREthereumToken    ethToken         = wallet->ethToken;
    BREthereumFeeBasis ethFeeBasis      = cryptoFeeBasisAsETH (estimatedFeeBasis);

    BREthereumAddress  ethSourceAddress = ethAccountGetPrimaryAddress (wallet->ethAccount);
    BREthereumAddress  ethTargetAddress = cryptoAddressAsETH (target);

    BREthereumTransferBasisType type = (NULL == ethToken ? TRANSFER_BASIS_TRANSACTION : TRANSFER_BASIS_LOG);

    UInt256 value = cryptoAmountGetValue (amount);
    char   *data  = cryptoTransferProvideOriginatingData (type, ethTargetAddress, value);

    BREthereumTransaction ethTransaction =
    transactionCreate (ethSourceAddress,
                       cryptoTransferProvideOriginatingTargetAddress (type, ethTargetAddress, ethToken),
                       cryptoTransferProvideOriginatingAmount (type, value),
                       ethFeeBasisGetGasPrice(ethFeeBasis),
                       ethFeeBasisGetGasLimit(ethFeeBasis),
                       data,
                       TRANSACTION_NONCE_IS_NOT_ASSIGNED);

    free (data);

    BRCryptoTransferDirection direction = (ETHEREUM_BOOLEAN_TRUE == ethAccountHasAddress (wallet->ethAccount, ethTargetAddress)
                                           ? CRYPTO_TRANSFER_RECOVERED
                                           : CRYPTO_TRANSFER_SENT);

    BRCryptoAddress  source   = cryptoAddressCreateAsETH  (ethSourceAddress);
    BRCryptoTransfer transfer = cryptoTransferCreateAsETH (unit,
                                                           unitForFee,
                                                           estimatedFeeBasis,
                                                           amount,
                                                           direction,
                                                           source,
                                                           target,
                                                           wallet->ethAccount,
                                                           type,
                                                           ethTransaction);

    transfer->sourceAddress = cryptoAddressCreateAsETH (ethSourceAddress);
    transfer->targetAddress = cryptoAddressCreateAsETH (ethTargetAddress);
    transfer->feeBasisEstimated = cryptoFeeBasisCreateAsETH (unitForFee, transactionGetFeeBasisLimit(ethTransaction));

    if (NULL != transfer && attributesCount > 0) {
        BRArrayOf (BRCryptoTransferAttribute) transferAttributes;
        array_new (transferAttributes, attributesCount);
        array_add_array (transferAttributes, attributes, attributesCount);
        cryptoTransferSetAttributes (transfer, transferAttributes);
        array_free (transferAttributes);
    }

    cryptoAddressGive(source);

    return transfer;
}

static BRCryptoTransfer
cryptoWalletCreateTransferMultipleETH (BRCryptoWallet walletBase,
                                       size_t outputsCount,
                                       BRCryptoTransferOutput *outputs,
                                       BRCryptoFeeBasis estimatedFeeBasis,
                                       BRCryptoCurrency currency,
                                       BRCryptoUnit unit,
                                       BRCryptoUnit unitForFee) {
    BRCryptoWalletETH wallet = cryptoWalletCoerce (walletBase);
    (void) wallet;

    if (0 == outputsCount) return NULL;

    return NULL;
}

static OwnershipGiven BRSetOf(BRCryptoAddress)
cryptoWalletGetAddressesForRecoveryETH (BRCryptoWallet walletBase) {
    BRSetOf(BRCryptoAddress) addresses = cryptoAddressSetCreate (1);
    BRSetAdd (addresses, cryptoWalletGetAddressETH (walletBase, CRYPTO_ADDRESS_SCHEME_ETH_DEFAULT));
    return addresses;
}

static bool
cryptoWalletIsEqualETH (BRCryptoWallet wb1, BRCryptoWallet wb2) {
    return wb1 == wb2;
}

BRCryptoWalletHandlers cryptoWalletHandlersETH = {
    cryptoWalletReleaseETH,
    cryptoWalletGetAddressETH,
    cryptoWalletHasAddressETH,
    cryptoWalletGetTransferAttributeCountETH,
    cryptoWalletGetTransferAttributeAtETH,
    cryptoWalletValidateTransferAttributeETH,
    cryptoWalletCreateTransferETH,
    cryptoWalletCreateTransferMultipleETH,
    cryptoWalletGetAddressesForRecoveryETH,
    cryptoWalletIsEqualETH
};
