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
                                                      NULL,
                                                      NULL,
                                                      cryptoFeeBasisCreateAsETH(unitForFee, ethDefaultFeeBasis),
                                                      &contextETH,
                                                      cryptoWalletCreateCallbackETH);

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

        case TRANSFER_BASIS_EXCHANGE:
            assert (false);
            return NULL;
    }
}

/** An Ethereum transaction's target address depends on if the transfer is for ETH or ERC20 */
static BREthereumAddress
cryptoTransferProvideOriginatingTargetAddress (BREthereumTransferBasisType type,
                                               BREthereumAddress targetAddress,
                                               BREthereumToken   token) {
    switch (type) {
        case TRANSFER_BASIS_TRANSACTION:
            return targetAddress;
        case TRANSFER_BASIS_LOG:
            return ethTokenGetAddressRaw (token);
        case TRANSFER_BASIS_EXCHANGE:
            assert (false);
            return EMPTY_ADDRESS_INIT;
    }
}

/** An Ethereum transaction's amount depends on if the transfer is for ETH or ERC20 */
static BREthereumEther
cryptoTransferProvideOriginatingAmount (BREthereumTransferBasisType type,
                                        UInt256 value) {
    switch (type) {
        case TRANSFER_BASIS_TRANSACTION:
            return ethEtherCreate(value);
        case TRANSFER_BASIS_LOG:
            return ethEtherCreateZero ();
        case TRANSFER_BASIS_EXCHANGE:
            assert (false);
            return ethEtherCreateZero();
    }
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

    // When creating an BREthereumTransaction, we'll apply margin to the `ethFeeBasis` if and only
    // if this is not the primary wallet (which is determined by unit and unitForFee compatibility).
    assert (FEE_BASIS_GAS == ethFeeBasis.type);
    ethFeeBasis.u.gas.limit = (CRYPTO_FALSE == cryptoUnitIsCompatible (unit, unitForFee)
                               ? gasApplyLimitMargin (ethFeeBasis.u.gas.limit)
                               : ethFeeBasis.u.gas.limit);
    cryptoFeeBasisGive (estimatedFeeBasis);
    estimatedFeeBasis = cryptoFeeBasisCreateAsETH (unitForFee, ethFeeBasis);

    BREthereumAddress  ethSourceAddress = ethAccountGetPrimaryAddress (walletETH->ethAccount);
    BREthereumAddress  ethTargetAddress = cryptoAddressAsETH (target);

    BREthereumTransferBasisType type = (NULL == ethToken ? TRANSFER_BASIS_TRANSACTION : TRANSFER_BASIS_LOG);

    UInt256 value = cryptoAmountGetValue (amount);
    char   *data  = cryptoTransferProvideOriginatingData (type, ethTargetAddress, value);

    BREthereumTransaction ethTransaction =
    transactionCreate (ethSourceAddress,
                       cryptoTransferProvideOriginatingTargetAddress (type, ethTargetAddress, ethToken),
                       cryptoTransferProvideOriginatingAmount (type, value),
                       ethFeeBasisGetGasPrice(ethFeeBasis),
                       ethFeeBasisGetGasLimit(ethFeeBasis),     // margin applied
                       data,
                       TRANSACTION_NONCE_IS_NOT_ASSIGNED);

    free (data);

    BRCryptoTransferDirection direction = (ETHEREUM_BOOLEAN_TRUE == ethAccountHasAddress (walletETH->ethAccount, ethTargetAddress)
                                           ? CRYPTO_TRANSFER_RECOVERED
                                           : CRYPTO_TRANSFER_SENT);

    BRCryptoAddress  source   = cryptoAddressCreateAsETH  (ethSourceAddress);

    BRCryptoTransferState transferState = { CRYPTO_TRANSFER_STATE_CREATED };

    // We don't/can't create TRANSFER_BASIS_EXCHANGE.
    BREthereumTransferBasis basis = (NULL == ethToken
                                     ? ((BREthereumTransferBasis) { TRANSFER_BASIS_TRANSACTION })
                                     : ((BREthereumTransferBasis) { TRANSFER_BASIS_LOG }));

    BRCryptoTransfer transfer = cryptoTransferCreateAsETH (wallet->listenerTransfer,
                                                           unit,
                                                           unitForFee,
                                                           estimatedFeeBasis,   // margin applied
                                                           amount,
                                                           direction,
                                                           source,
                                                           target,
                                                           transferState,
                                                           walletETH->ethAccount,
                                                           basis,
                                                           ethTransaction);

    if (NULL != transfer && attributesCount > 0)
        cryptoTransferSetAttributes (transfer, attributesCount, attributes);

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
    BRSetAdd (addresses, cryptoWalletGetAddressETH (wallet, CRYPTO_ADDRESS_SCHEME_ETH_DEFAULT));
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

static bool
cryptoWalletIsEqualETH (BRCryptoWallet wb1, BRCryptoWallet wb2) {
    return wb1 == wb2;
}


#ifdef REFACTOR
private_extern void
walletUpdateBalance (BREthereumWallet wallet) {
    int overflow = 0, negative = 0, fee_overflow = 0;

    UInt256 recv = UINT256_ZERO;
    UInt256 sent = UINT256_ZERO;
    UInt256 fees = UINT256_ZERO;

    for (size_t index = 0; index < array_count (wallet->transfers); index++) {
        BREthereumTransfer transfer = wallet->transfers[index];
        BREthereumAmount   amount = transferGetAmount(transfer);
        assert (ethAmountGetType(wallet->balance) == ethAmountGetType(amount));
        UInt256 value = (AMOUNT_ETHER == ethAmountGetType(amount)
                         ? ethAmountGetEther(amount).valueInWEI
                         : ethAmountGetTokenQuantity(amount).valueAsInteger);

        // Will be ZERO if transfer is not for ETH
        BREthereumEther fee = transferGetFee(transfer, &fee_overflow);

        BREthereumBoolean isSend = ethAddressEqual(wallet->address, transferGetSourceAddress(transfer));
        BREthereumBoolean isRecv = ethAddressEqual(wallet->address, transferGetTargetAddress(transfer));

        if (ETHEREUM_BOOLEAN_IS_TRUE (isSend)) {
            sent = uint256Add_Overflow(sent, value, &overflow);
            assert (!overflow);

            fees = uint256Add_Overflow(fees, fee.valueInWEI, &fee_overflow);
            assert (!fee_overflow);
        }

        if (ETHEREUM_BOOLEAN_IS_TRUE (isRecv)) {
            recv = uint256Add_Overflow(recv, value, &overflow);
            assert (!overflow);
        }
    }

    // A wallet balance can never be negative; however, as transfers arrive in a sporadic manner,
    // the balance could be negative until all transfers arrive, eventually.  If negative, we'll
    // set the balance to zero.
    UInt256 balance = uint256Sub_Negative(recv, sent, &negative);

    // If we are going to be changing the balance here then 1) shouldn't we call walletSetBalance()
    // and shouldn't we also ensure that an event is generated (like all calls to
    // walletSetBalance() ensure)?

    if (AMOUNT_ETHER == ethAmountGetType(wallet->balance)) {
        balance = uint256Sub_Negative(balance, fees, &negative);
        if (negative) balance = UINT256_ZERO;
        wallet->balance = ethAmountCreateEther (ethEtherCreate(balance));
    }
    else {
        if (negative) balance = UINT256_ZERO;
        wallet->balance = ethAmountCreateToken (ethTokenQuantityCreate(ethAmountGetToken (wallet->balance), balance));
    }
}

extern uint64_t
walletGetTransferCountAsSource (BREthereumWallet wallet) {
    unsigned int count = 0;

    for (int i = 0; i < array_count(wallet->transfers); i++)
         if (ETHEREUM_BOOLEAN_IS_TRUE(ethAddressEqual(wallet->address, transferGetSourceAddress(wallet->transfers[i]))))
             count += 1;

    return count;
}

extern uint64_t
walletGetTransferNonceMaximumAsSource (BREthereumWallet wallet) {
    uint64_t nonce = TRANSACTION_NONCE_IS_NOT_ASSIGNED;

#define MAX(x,y)    ((x) >= (y) ? (x) : (y))
    for (int i = 0; i < array_count(wallet->transfers); i++)
        if (ETHEREUM_BOOLEAN_IS_TRUE(ethAddressEqual(wallet->address, transferGetSourceAddress(wallet->transfers[i])))) {
            uint64_t newNonce = (unsigned int) transferGetNonce(wallet->transfers[i]);
            // wallet->transfers can have a newly created transfer that does not yet have
            // an assigned nonce - avoid such a transfer.
            if ( TRANSACTION_NONCE_IS_NOT_ASSIGNED != newNonce  &&
                (TRANSACTION_NONCE_IS_NOT_ASSIGNED == nonce     || newNonce > nonce))
                nonce = (unsigned int) newNonce;
        }
#undef MAX
    return nonce;
}

#endif

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
    NULL,
    cryptoWalletIsEqualETH
};
