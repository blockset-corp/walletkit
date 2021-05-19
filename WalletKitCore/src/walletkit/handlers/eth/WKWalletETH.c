//
//  WKWalletETH.c
//  WalletKitCore
//
//  Created by Ed Gamble on 05/07/2020.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKETH.h"
#include "walletkit/WKAmountP.h"

#define DEFAULT_ETHER_GAS_PRICE      2ull  // 2 GWEI
#define DEFAULT_ETHER_GAS_PRICE_UNIT GWEI

#define DEFAULT_ETHER_GAS_LIMIT    21000ull

extern WKWalletETH
wkWalletCoerce (WKWallet wallet) {
    assert (WK_NETWORK_TYPE_ETH == wallet->type);
    return (WKWalletETH) wallet;
}

typedef struct {
    BREthereumAccount ethAccount;
    BREthereumToken   ethToken;
    BREthereumGas ethGasLimit;
} WKWalletCreateContextETH;

static void
wkWalletCreateCallbackETH (WKWalletCreateContext context,
                               WKWallet wallet) {
    WKWalletCreateContextETH *contextETH = (WKWalletCreateContextETH*) context;
    WKWalletETH walletETH = wkWalletCoerce (wallet);

    walletETH->ethAccount  = contextETH->ethAccount;
    walletETH->ethToken    = contextETH->ethToken;
    walletETH->ethGasLimit = contextETH->ethGasLimit;
}

private_extern WKWallet
wkWalletCreateAsETH (WKWalletListener listener,
                         WKUnit unit,
                         WKUnit unitForFee,
                         BREthereumToken   ethToken,
                         BREthereumAccount ethAccount) {
    BREthereumFeeBasis ethDefaultFeeBasis =
    ethFeeBasisCreate ((NULL == ethToken
                        ? ethGasCreate (DEFAULT_ETHER_GAS_LIMIT)
                        : ethTokenGetGasLimit (ethToken)),
                       ethGasPriceCreate (ethEtherCreateNumber (DEFAULT_ETHER_GAS_PRICE,
                                                                DEFAULT_ETHER_GAS_PRICE_UNIT)));

    WKAmount minBalance = wkAmountCreateInteger(0, unit);

    WKWalletCreateContextETH contextETH = {
        ethAccount,
        ethToken,
        ethDefaultFeeBasis.u.gas.limit
    };
    
    WKWallet wallet = wkWalletAllocAndInit (sizeof (struct WKWalletETHRecord),
                                                      WK_NETWORK_TYPE_ETH,
                                                      listener,
                                                      unit,
                                                      unitForFee,
                                                      minBalance,
                                                      NULL,
                                                      wkFeeBasisCreateAsETH(unitForFee, ethDefaultFeeBasis),
                                                      &contextETH,
                                                      wkWalletCreateCallbackETH);

    wkAmountGive(minBalance);
    return wallet;
}

static void
wkWalletReleaseETH (WKWallet wallet) {
}


static WKAddress
wkWalletGetAddressETH (WKWallet wallet,
                           WKAddressScheme addressScheme) {
    WKWalletETH walletETH = wkWalletCoerce(wallet);
    BREthereumAddress ethAddress = ethAccountGetPrimaryAddress (walletETH->ethAccount);

    return wkAddressCreateAsETH (ethAddress);
}

static bool
wkWalletHasAddressETH (WKWallet wallet,
                           WKAddress address) {
    WKWalletETH walletETH = wkWalletCoerce (wallet);
    return (ETHEREUM_BOOLEAN_TRUE == ethAddressEqual (wkAddressAsETH (address),
                                                      ethAccountGetPrimaryAddress (walletETH->ethAccount)));
}

extern size_t
wkWalletGetTransferAttributeCountETH (WKWallet wallet,
                                          WKAddress target) {
    return 0;
}

extern WKTransferAttribute
wkWalletGetTransferAttributeAtETH (WKWallet wallet,
                                       WKAddress target,
                                       size_t index) {
    return NULL;
}

extern WKTransferAttributeValidationError
wkWalletValidateTransferAttributeETH (WKWallet wallet,
                                          OwnershipKept WKTransferAttribute attribute,
                                          WKBoolean *validates) {
    *validates = WK_TRUE;
    return (WKTransferAttributeValidationError) 0;
}

/** An Ethereum transaction's data depends on if the transfer is for ETH or ERC20 */
static char *
wkTransferProvideOriginatingData (BREthereumToken token,
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
wkTransferProvideOriginatingTargetAddress (BREthereumToken   token,
                                               BREthereumAddress targetAddress) {
    return (NULL == token
            ? targetAddress
            : ethTokenGetAddressRaw (token));
}

/** An Ethereum transaction's amount depends on if the transfer is for ETH or ERC20 */
static BREthereumEther
wkTransferProvideOriginatingAmount (BREthereumToken token,
                                        UInt256 value) {
    return (NULL == token
            ? ethEtherCreate(value)
            : ethEtherCreateZero ());
}

extern WKTransfer
wkWalletCreateTransferETH (WKWallet  wallet,
                               WKAddress target,
                               WKAmount  amount,
                               WKFeeBasis estimatedFeeBasis,
                               size_t attributesCount,
                               OwnershipKept WKTransferAttribute *attributes,
                               WKCurrency currency,
                               WKUnit unit,
                               WKUnit unitForFee) {
    WKWalletETH walletETH = wkWalletCoerce (wallet);
    assert (wkWalletGetType(wallet) == wkAddressGetType(target));
    assert (wkAmountHasCurrency (amount, currency));

    BREthereumToken    ethToken         = walletETH->ethToken;
    BREthereumFeeBasis ethFeeBasis      = wkFeeBasisAsETH (estimatedFeeBasis);
    BREthereumAddress  ethSourceAddress = ethAccountGetPrimaryAddress (walletETH->ethAccount);
    BREthereumAddress  ethTargetAddress = wkAddressAsETH (target);

    UInt256 value = wkAmountGetValue (amount);
    char   *data  = wkTransferProvideOriginatingData (ethToken, ethTargetAddress, value);

    uint64_t nonce = ETHEREUM_TRANSACTION_NONCE_IS_NOT_ASSIGNED;

    // When creating an BREthereumTransaction, we'll apply margin to the gasLimit in `ethFeeBasis`.
    // This helps to ensure that the transaction will be accepted into the blockchain rather than
    // be rejected with 'not enough gas'.  We apply this no matter the transaction type, for ETH or
    // TOK.  With an ETH transaction the target address might be a 'Smart Contract'.
    //
    // The created `ethTransaction` is unsigned and thus doesn't have a hash.
    BREthereumTransaction ethTransaction =
    ethTransactionCreate (ethSourceAddress,
                       wkTransferProvideOriginatingTargetAddress (ethToken, ethTargetAddress),
                       wkTransferProvideOriginatingAmount (ethToken, value),
                       ethFeeBasisGetGasPrice(ethFeeBasis),
                       gasApplyLimitMargin (ethFeeBasisGetGasLimit(ethFeeBasis)),
                       data,
                       nonce);

    free (data);

    WKAddress  source   = wkAddressCreateAsETH  (ethSourceAddress);

    WKTransferState state = wkTransferStateInit(WK_TRANSFER_STATE_CREATED);

    WKTransfer transfer = wkTransferCreateAsETH (wallet->listenerTransfer,
                                                           NULL, // w/o hash
                                                           unit,
                                                           unitForFee,
                                                           estimatedFeeBasis,  // w/o margin
                                                           amount,
                                                           source,
                                                           target,
                                                           state,
                                                           walletETH->ethAccount,
                                                           nonce,
                                                           ethTransaction);
    wkTransferSetAttributes (transfer, attributesCount, attributes);
    wkTransferStateGive (state);

    wkAddressGive(source);

    return transfer;
}

static WKTransfer
wkWalletCreateTransferMultipleETH (WKWallet wallet,
                                       size_t outputsCount,
                                       WKTransferOutput *outputs,
                                       WKFeeBasis estimatedFeeBasis,
                                       WKCurrency currency,
                                       WKUnit unit,
                                       WKUnit unitForFee) {
    WKWalletETH walletETH = wkWalletCoerce (wallet);
    (void) walletETH;

    if (0 == outputsCount) return NULL;

    return NULL;
}

static OwnershipGiven BRSetOf(WKAddress)
wkWalletGetAddressesForRecoveryETH (WKWallet wallet) {
    BRSetOf(WKAddress) addresses = wkAddressSetCreate (1);
    BRSetAdd (addresses, wkWalletGetAddressETH (wallet, WK_ADDRESS_SCHEME_NATIVE));
    return addresses;
}

extern WKTransferETH
wkWalletLookupTransferByIdentifier (WKWalletETH walletETH,
                                        BREthereumHash hash) {
    if (ETHEREUM_BOOLEAN_IS_TRUE (ethHashEqual (hash, ETHEREUM_EMPTY_HASH_INIT))) return NULL;

    for (int i = 0; i < array_count(walletETH->base.transfers); i++) {
        WKTransferETH transferETH = wkTransferCoerceETH (walletETH->base.transfers[i]);
        BREthereumHash identifier = wkTransferGetIdentifierETH(transferETH);
        if (ETHEREUM_BOOLEAN_IS_TRUE (ethHashEqual (hash, identifier)))
            return transferETH;
    }
    return NULL;
}

extern WKTransferETH
wkWalletLookupTransferByOriginatingHash (WKWalletETH walletETH,
                                             BREthereumHash hash) {
    if (ETHEREUM_BOOLEAN_IS_TRUE (ethHashEqual (hash, ETHEREUM_EMPTY_HASH_INIT))) return NULL;

    for (int i = 0; i < array_count(walletETH->base.transfers); i++) {
        WKTransferETH transferETH = wkTransferCoerceETH (walletETH->base.transfers[i]);
        BREthereumTransaction transaction = transferETH->originatingTransaction;
        if (NULL != transaction && ETHEREUM_BOOLEAN_IS_TRUE (ethHashEqual (hash, ethTransactionGetHash (transaction))))
            return transferETH;
    }
    return NULL;
}

static void
wkWalletAnnounceTransferETH (WKWallet wallet,
                                 WKTransfer transfer,
                                 WKWalletEventType type) {
    WKWalletETH walletETH = wkWalletCoerce (wallet);

    // We are only interested in updating the accounts nonce; therefore token wallets are ignored.
    if (NULL != walletETH->ethToken) return;

    uint64_t nonce = ETHEREUM_TRANSACTION_NONCE_IS_NOT_ASSIGNED;

    // Ignore `type`; just iterate over all transfers.
    for (size_t index = 0; index < array_count(wallet->transfers); index++) {
        WKTransferETH transferETH = wkTransferCoerceETH (wallet->transfers[index]);
        uint64_t transferNonce = wkTransferGetNonceETH(transferETH);

        if (WK_TRANSFER_RECEIVED                       != transferETH->base.direction &&
            ETHEREUM_TRANSACTION_NONCE_IS_NOT_ASSIGNED != transferNonce               &&
            WK_TRANSFER_STATE_ERRORED                  != wkTransferGetStateType (wallet->transfers[index]))
            nonce = (ETHEREUM_TRANSACTION_NONCE_IS_NOT_ASSIGNED == nonce
                     ? transferNonce
                     : MAX (nonce, transferNonce));
    }

    ethAccountSetAddressNonce (walletETH->ethAccount,
                               ethAccountGetPrimaryAddress(walletETH->ethAccount),
                               (ETHEREUM_TRANSACTION_NONCE_IS_NOT_ASSIGNED == nonce ? 0 : (nonce + 1)),
                               ETHEREUM_BOOLEAN_TRUE);
}

static bool
wkWalletIsEqualETH (WKWallet wb1, WKWallet wb2) {
    return wb1 == wb2;
}

WKWalletHandlers wkWalletHandlersETH = {
    wkWalletReleaseETH,
    wkWalletGetAddressETH,
    wkWalletHasAddressETH,
    wkWalletGetTransferAttributeCountETH,
    wkWalletGetTransferAttributeAtETH,
    wkWalletValidateTransferAttributeETH,
    wkWalletCreateTransferETH,
    wkWalletCreateTransferMultipleETH,
    wkWalletGetAddressesForRecoveryETH,
    wkWalletAnnounceTransferETH,
    wkWalletIsEqualETH
};
