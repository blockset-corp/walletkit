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

#define DEFAULT_ETHER_GAS_PRICE      2ull  // 2 GWEI
#define DEFAULT_ETHER_GAS_PRICE_UNIT GWEI

#define DEFAULT_ETHER_GAS_LIMIT    21000ull

extern BRCryptoWalletETH
cryptoWalletCoerce (BRCryptoWallet wallet) {
    assert (CRYPTO_NETWORK_TYPE_ETH == wallet->type);
    return (BRCryptoWalletETH) wallet;
}

typedef struct {
    BREthereumAccount ethAccount;
    BREthereumToken   ethToken;
    BREthereumGas ethGasLimit;
} BRCryptoWalletCreateContextETH;

static void
cryptoWalletCreateCallbackETH (BRCryptoWalletCreateContext context,
                               BRCryptoWallet wallet) {
    BRCryptoWalletCreateContextETH *contextETH = (BRCryptoWalletCreateContextETH*) context;
    BRCryptoWalletETH walletETH = cryptoWalletCoerce (wallet);

    walletETH->ethAccount  = contextETH->ethAccount;
    walletETH->ethToken    = contextETH->ethToken;
    walletETH->ethGasLimit = contextETH->ethGasLimit;
}

private_extern BRCryptoWallet
cryptoWalletCreateAsETH (BRCryptoWalletListener listener,
                         BRCryptoUnit unit,
                         BRCryptoUnit unitForFee,
                         BREthereumToken   ethToken,
                         BREthereumAccount ethAccount) {
    BREthereumFeeBasis ethDefaultFeeBasis =
    ethFeeBasisCreate ((NULL == ethToken
                        ? ethGasCreate (DEFAULT_ETHER_GAS_LIMIT)
                        : ethTokenGetGasLimit (ethToken)),
                       ethGasPriceCreate (ethEtherCreateNumber (DEFAULT_ETHER_GAS_PRICE,
                                                                DEFAULT_ETHER_GAS_PRICE_UNIT)));

    BRCryptoAmount minBalance = cryptoAmountCreateInteger(0, unit);

    BRCryptoWalletCreateContextETH contextETH = {
        ethAccount,
        ethToken,
        ethDefaultFeeBasis.u.gas.limit
    };
    
    BRCryptoWallet wallet = cryptoWalletAllocAndInit (sizeof (struct BRCryptoWalletETHRecord),
                                                      CRYPTO_NETWORK_TYPE_ETH,
                                                      listener,
                                                      unit,
                                                      unitForFee,
                                                      minBalance,
                                                      NULL,
                                                      cryptoFeeBasisCreateAsETH(unitForFee, ethDefaultFeeBasis),
                                                      &contextETH,
                                                      cryptoWalletCreateCallbackETH);

    cryptoAmountGive(minBalance);
    return wallet;
}

static void
cryptoWalletReleaseETH (BRCryptoWallet wallet) {
}


static BRCryptoAddress
cryptoWalletGetAddressETH (BRCryptoWallet wallet,
                           BRCryptoAddressScheme addressScheme) {
    BRCryptoWalletETH walletETH = cryptoWalletCoerce(wallet);
    BREthereumAddress ethAddress = ethAccountGetPrimaryAddress (walletETH->ethAccount);

    return cryptoAddressCreateAsETH (ethAddress);
}

static bool
cryptoWalletHasAddressETH (BRCryptoWallet wallet,
                           BRCryptoAddress address) {
    BRCryptoWalletETH walletETH = cryptoWalletCoerce (wallet);
    return (ETHEREUM_BOOLEAN_TRUE == ethAddressEqual (cryptoAddressAsETH (address),
                                                      ethAccountGetPrimaryAddress (walletETH->ethAccount)));
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

/** An Ethereum transaction's data depends on if the transfer is for ETH or ERC20 */
static char *
cryptoTransferProvideOriginatingData (BREthereumToken token,
                                      BREthereumAddress targetAddress,
                                      UInt256 value) {
    if (NULL == token) return strdup ("");

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

/** An Ethereum transaction's target address depends on if the transfer is for ETH or ERC20 */
static BREthereumAddress
cryptoTransferProvideOriginatingTargetAddress (BREthereumToken   token,
                                               BREthereumAddress targetAddress) {
    return (NULL == token
            ? targetAddress
            : ethTokenGetAddressRaw (token));
}

/** An Ethereum transaction's amount depends on if the transfer is for ETH or ERC20 */
static BREthereumEther
cryptoTransferProvideOriginatingAmount (BREthereumToken token,
                                        UInt256 value) {
    return (NULL == token
            ? ethEtherCreate(value)
            : ethEtherCreateZero ());
}

extern BRCryptoTransfer
cryptoWalletCreateTransferETH (BRCryptoWallet  wallet,
                               BRCryptoAddress target,
                               BRCryptoAmount  amount,
                               BRCryptoFeeBasis estimatedFeeBasis,
                               size_t attributesCount,
                               OwnershipKept BRCryptoTransferAttribute *attributes,
                               BRCryptoCurrency currency,
                               BRCryptoUnit unit,
                               BRCryptoUnit unitForFee) {
    BRCryptoWalletETH walletETH = cryptoWalletCoerce (wallet);
    assert (cryptoWalletGetType(wallet) == cryptoAddressGetType(target));
    assert (cryptoAmountHasCurrency (amount, currency));

    BREthereumToken    ethToken         = walletETH->ethToken;
    BREthereumFeeBasis ethFeeBasis      = cryptoFeeBasisAsETH (estimatedFeeBasis);
    BREthereumAddress  ethSourceAddress = ethAccountGetPrimaryAddress (walletETH->ethAccount);
    BREthereumAddress  ethTargetAddress = cryptoAddressAsETH (target);

    UInt256 value = cryptoAmountGetValue (amount);
    char   *data  = cryptoTransferProvideOriginatingData (ethToken, ethTargetAddress, value);

    // When creating an BREthereumTransaction, we'll apply margin to the gasLimit in `ethFeeBasis`.
    // This helps to ensure that the transaction will be accepted into the blockchain rather than
    // be rejected with 'not enough gas'.  We apply this no matter the transaction type, for ETH or
    // TOK.  With an ETH transaction the target address might be a 'Smart Contract'.
    BREthereumTransaction ethTransaction =
    transactionCreate (ethSourceAddress,
                       cryptoTransferProvideOriginatingTargetAddress (ethToken, ethTargetAddress),
                       cryptoTransferProvideOriginatingAmount (ethToken, value),
                       ethFeeBasisGetGasPrice(ethFeeBasis),
                       gasApplyLimitMargin (ethFeeBasisGetGasLimit(ethFeeBasis)),
                       data,
                       TRANSACTION_NONCE_IS_NOT_ASSIGNED);

    free (data);

    BRCryptoAddress  source   = cryptoAddressCreateAsETH  (ethSourceAddress);

    BRCryptoTransferState state = cryptoTransferStateInit(CRYPTO_TRANSFER_STATE_CREATED);

    BRCryptoTransfer transfer = cryptoTransferCreateAsETH (wallet->listenerTransfer,
                                                           NULL,
                                                           unit,
                                                           unitForFee,
                                                           estimatedFeeBasis,  // w/o margin
                                                           amount,
                                                           source,
                                                           target,
                                                           state,
                                                           walletETH->ethAccount,
                                                           TRANSACTION_NONCE_IS_NOT_ASSIGNED,
                                                           ethTransaction);
    cryptoTransferSetAttributes (transfer, attributesCount, attributes);
    cryptoTransferStateGive (state);

    cryptoAddressGive(source);

    return transfer;
}

static BRCryptoTransfer
cryptoWalletCreateTransferMultipleETH (BRCryptoWallet wallet,
                                       size_t outputsCount,
                                       BRCryptoTransferOutput *outputs,
                                       BRCryptoFeeBasis estimatedFeeBasis,
                                       BRCryptoCurrency currency,
                                       BRCryptoUnit unit,
                                       BRCryptoUnit unitForFee) {
    BRCryptoWalletETH walletETH = cryptoWalletCoerce (wallet);
    (void) walletETH;

    if (0 == outputsCount) return NULL;

    return NULL;
}

static OwnershipGiven BRSetOf(BRCryptoAddress)
cryptoWalletGetAddressesForRecoveryETH (BRCryptoWallet wallet) {
    BRSetOf(BRCryptoAddress) addresses = cryptoAddressSetCreate (1);
    BRSetAdd (addresses, cryptoWalletGetAddressETH (wallet, CRYPTO_ADDRESS_SCHEME_NATIVE));
    return addresses;
}

extern BRCryptoTransferETH
cryptoWalletLookupTransferByIdentifier (BRCryptoWalletETH walletETH,
                                        BREthereumHash hash) {
    if (ETHEREUM_BOOLEAN_IS_TRUE (ethHashEqual (hash, EMPTY_HASH_INIT))) return NULL;

    for (int i = 0; i < array_count(walletETH->base.transfers); i++) {
        BRCryptoTransferETH transferETH = cryptoTransferCoerceETH (walletETH->base.transfers[i]);
        BREthereumHash identifier = cryptoTransferGetIdentifierETH(transferETH);
        if (ETHEREUM_BOOLEAN_IS_TRUE (ethHashEqual (hash, identifier)))
            return transferETH;
    }
    return NULL;
}

extern BRCryptoTransferETH
cryptoWalletLookupTransferByOriginatingHash (BRCryptoWalletETH walletETH,
                                             BREthereumHash hash) {
    if (ETHEREUM_BOOLEAN_IS_TRUE (ethHashEqual (hash, EMPTY_HASH_INIT))) return NULL;

    for (int i = 0; i < array_count(walletETH->base.transfers); i++) {
        BRCryptoTransferETH transferETH = cryptoTransferCoerceETH (walletETH->base.transfers[i]);
        BREthereumTransaction transaction = transferETH->originatingTransaction;
        if (NULL != transaction && ETHEREUM_BOOLEAN_IS_TRUE (ethHashEqual (hash, transactionGetHash (transaction))))
            return transferETH;
    }
    return NULL;
}

static void
cryptoWalletAnnounceTransferETH (BRCryptoWallet wallet,
                                 BRCryptoTransfer transfer,
                                 BRCryptoWalletEventType type) {
    BRCryptoWalletETH walletETH = cryptoWalletCoerce (wallet);

    // We are only interested in updating the accounts nonce; therefore token wallets are ignored.
    if (NULL != walletETH->ethToken) return;

    uint64_t nonce = TRANSACTION_NONCE_IS_NOT_ASSIGNED;

    // Ignore `type`; just iterate over all transfers.
    for (size_t index = 0; index < array_count(wallet->transfers); index++) {
        BRCryptoTransferETH transferETH = cryptoTransferCoerceETH (wallet->transfers[index]);

        if (CRYPTO_TRANSFER_RECEIVED != transferETH->base.direction &&
            TRANSACTION_NONCE_IS_NOT_ASSIGNED != transferETH->nonce)
            nonce = (TRANSACTION_NONCE_IS_NOT_ASSIGNED == nonce
                     ? transferETH->nonce
                     : MAX (nonce, transferETH->nonce));
    }

    ethAccountSetAddressNonce (walletETH->ethAccount,
                               ethAccountGetPrimaryAddress(walletETH->ethAccount),
                               (TRANSACTION_NONCE_IS_NOT_ASSIGNED == nonce ? 0 : (nonce + 1)),
                               ETHEREUM_BOOLEAN_TRUE);
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
    cryptoWalletAnnounceTransferETH,
    cryptoWalletIsEqualETH
};
