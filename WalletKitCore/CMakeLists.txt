# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# now build app's shared lib
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

#
# sqlite3 build
#

add_library(sqlite3
            STATIC)

target_sources (sqlite3
                PRIVATE
                ${PROJECT_SOURCE_DIR}/vendor/sqlite3/sqlite3.c
                ${PROJECT_SOURCE_DIR}/vendor/sqlite3/sqlite3.h)

target_include_directories(sqlite3
                           INTERFACE
                           ${PROJECT_SOURCE_DIR}/vendor/sqlite3)

target_compile_definitions (sqlite3
                            PRIVATE
                            _HAVE_SQLITE_CONFIG_H=1)

#
# ed25519 build
#
add_library(ed25519
            STATIC)

target_sources (ed25519
                PRIVATE
                ${PROJECT_SOURCE_DIR}/vendor/ed25519/ed25519.h
                ${PROJECT_SOURCE_DIR}/vendor/ed25519/fe.c
                ${PROJECT_SOURCE_DIR}/vendor/ed25519/fe.h
                ${PROJECT_SOURCE_DIR}/vendor/ed25519/fixedint.h
                ${PROJECT_SOURCE_DIR}/vendor/ed25519/ge.c
                ${PROJECT_SOURCE_DIR}/vendor/ed25519/ge.h
                ${PROJECT_SOURCE_DIR}/vendor/ed25519/keypair.c
                ${PROJECT_SOURCE_DIR}/vendor/ed25519/precomp_data.h
                ${PROJECT_SOURCE_DIR}/vendor/ed25519/sc.c
                ${PROJECT_SOURCE_DIR}/vendor/ed25519/sc.h
                ${PROJECT_SOURCE_DIR}/vendor/ed25519/sha512.c
                ${PROJECT_SOURCE_DIR}/vendor/ed25519/sha512.h
                ${PROJECT_SOURCE_DIR}/vendor/ed25519/sign.c
                ${PROJECT_SOURCE_DIR}/vendor/ed25519/verify.c)

target_include_directories(ed25519
                           INTERFACE
                           ${PROJECT_SOURCE_DIR}/vendor/ed25519)

#
# blake2 build
#
add_library(blake2
            STATIC)

target_sources (blake2
                PRIVATE
                ${PROJECT_SOURCE_DIR}/vendor/blake2/blake2b.c
                ${PROJECT_SOURCE_DIR}/vendor/blake2/blake2b.h)

target_include_directories (blake2
                            INTERFACE
                            ${PROJECT_SOURCE_DIR}/vendor/blake2)

#
# crypto build
#

add_library( # Sets the name of the library.
             corecrypto
             # Sets the library as a shared library.
             SHARED)

# Support
target_sources (corecrypto
                PRIVATE
                ${PROJECT_SOURCE_DIR}/src/support/BROSCompat.c
                ${PROJECT_SOURCE_DIR}/src/support/BROSCompat.h
                ${PROJECT_SOURCE_DIR}/src/support/BRAddress.c
                ${PROJECT_SOURCE_DIR}/src/support/BRAddress.h
                ${PROJECT_SOURCE_DIR}/src/support/BRAssert.c
                ${PROJECT_SOURCE_DIR}/src/support/BRAssert.h
                ${PROJECT_SOURCE_DIR}/src/support/BRArray.h
                ${PROJECT_SOURCE_DIR}/src/support/BRBase.h
                ${PROJECT_SOURCE_DIR}/src/support/BRBIP32Sequence.c
                ${PROJECT_SOURCE_DIR}/src/support/BRBIP32Sequence.h
                ${PROJECT_SOURCE_DIR}/src/support/BRBIP39Mnemonic.c
                ${PROJECT_SOURCE_DIR}/src/support/BRBIP39Mnemonic.h
                ${PROJECT_SOURCE_DIR}/src/support/BRBIP39WordsEn.h
                ${PROJECT_SOURCE_DIR}/src/support/BRBase58.c
                ${PROJECT_SOURCE_DIR}/src/support/BRBase58.h
                ${PROJECT_SOURCE_DIR}/src/support/BRBech32.c
                ${PROJECT_SOURCE_DIR}/src/support/BRBech32.h
                ${PROJECT_SOURCE_DIR}/src/support/BRCrypto.c
                ${PROJECT_SOURCE_DIR}/src/support/BRCrypto.h
                ${PROJECT_SOURCE_DIR}/src/support/BRFileService.c
                ${PROJECT_SOURCE_DIR}/src/support/BRFileService.h
                ${PROJECT_SOURCE_DIR}/src/support/BRInt.h
                ${PROJECT_SOURCE_DIR}/src/support/BRKey.c
                ${PROJECT_SOURCE_DIR}/src/support/BRKey.h
                ${PROJECT_SOURCE_DIR}/src/support/BRKeyECIES.c
                ${PROJECT_SOURCE_DIR}/src/support/BRKeyECIES.h
                ${PROJECT_SOURCE_DIR}/src/support/BRSet.c
                ${PROJECT_SOURCE_DIR}/src/support/BRSet.h)

# Support Tests
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_sources (corecrypto
                    PRIVATE
                    ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/support/testSup.c)
endif(CMAKE_BUILD_TYPE MATCHES Debug)

# Bitcoin
target_sources (corecrypto
                PRIVATE
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRBIP38Key.c
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRBIP38Key.h
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRBloomFilter.c
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRBloomFilter.h
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRChainParams.h
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRChainParams.c
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRMerkleBlock.c
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRMerkleBlock.h
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRPaymentProtocol.c
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRPaymentProtocol.h
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRPeer.c
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRPeer.h
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRPeerManager.c
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRPeerManager.h
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRSyncManager.c
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRSyncManager.h
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRTransaction.c
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRTransaction.h
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRWallet.c
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRWallet.h
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRWalletManager.c
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRWalletManager.h
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRWalletManagerEvent.c
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRWalletManagerPrivate.h)

# Bitcoin Tests
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_sources (corecrypto
                    PRIVATE
                    ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/bitcoin/test.c
                    ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/bitcoin/testBwm.c)
endif(CMAKE_BUILD_TYPE MATCHES Debug)

# BCash
target_sources (corecrypto
                PRIVATE
                ${PROJECT_SOURCE_DIR}/src/bcash/BRBCashAddr.c
                ${PROJECT_SOURCE_DIR}/src/bcash/BRBCashAddr.h
                ${PROJECT_SOURCE_DIR}/src/bcash/BRBCashParams.h
                ${PROJECT_SOURCE_DIR}/src/bcash/BRBCashParams.c)

# Ethereum
target_sources (corecrypto
                PRIVATE
                ${PROJECT_SOURCE_DIR}/src/ethereum/BREthereum.h
                # Util
                ${PROJECT_SOURCE_DIR}/src/ethereum/util/BRKeccak.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/util/BRKeccak.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/util/BRUtil.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/util/BRUtilHex.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/util/BRUtilHex.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/util/BRUtilMath.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/util/BRUtilMath.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/util/BRUtilMathParse.c
                # RLP
                ${PROJECT_SOURCE_DIR}/src/ethereum/rlp/BRRlp.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/rlp/BRRlpCoder.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/rlp/BRRlpCoder.h
                # Event
                ${PROJECT_SOURCE_DIR}/src/ethereum/event/BREvent.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/event/BREvent.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/event/BREventAlarm.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/event/BREventAlarm.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/event/BREventQueue.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/event/BREventQueue.h
                # Base
                ${PROJECT_SOURCE_DIR}/src/ethereum/base/BREthereumAddress.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/base/BREthereumAddress.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/base/BREthereumBase.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/base/BREthereumEther.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/base/BREthereumEther.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/base/BREthereumGas.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/base/BREthereumGas.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/base/BREthereumFeeBasis.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/base/BREthereumFeeBasis.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/base/BREthereumHash.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/base/BREthereumHash.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/base/BREthereumData.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/base/BREthereumData.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/base/BREthereumLogic.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/base/BREthereumSignature.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/base/BREthereumSignature.h
                # Block Chain
                ${PROJECT_SOURCE_DIR}/src/ethereum/blockchain/BREthereumAccountState.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/blockchain/BREthereumAccountState.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/blockchain/BREthereumBlock.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/blockchain/BREthereumBlock.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/blockchain/BREthereumBlockChain.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/blockchain/BREthereumBloomFilter.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/blockchain/BREthereumBloomFilter.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/blockchain/BREthereumLog.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/blockchain/BREthereumLog.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/blockchain/BREthereumNetwork.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/blockchain/BREthereumNetwork.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/blockchain/BREthereumTransaction.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/blockchain/BREthereumTransaction.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/blockchain/BREthereumTransactionReceipt.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/blockchain/BREthereumTransactionReceipt.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/blockchain/BREthereumTransactionStatus.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/blockchain/BREthereumTransactionStatus.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/blockchain/BREthereumProofOfWork.c
                # Contract
                ${PROJECT_SOURCE_DIR}/src/ethereum/contract/BREthereumContract.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/contract/BREthereumContract.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/contract/BREthereumToken.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/contract/BREthereumToken.h
                # MPT
                ${PROJECT_SOURCE_DIR}/src/ethereum/mpt/BREthereumMPT.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/mpt/BREthereumMPT.h
                # LES Msg
                ${PROJECT_SOURCE_DIR}/src/ethereum/les/msg/BREthereumMessageDIS.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/les/msg/BREthereumMessageDIS.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/les/msg/BREthereumMessageETH.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/les/msg/BREthereumMessageLES.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/les/msg/BREthereumMessageLES.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/les/msg/BREthereumMessageP2P.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/les/msg/BREthereumMessageP2P.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/les/msg/BREthereumMessagePIP.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/les/msg/BREthereumMessagePIP.h
                # LES
                ${PROJECT_SOURCE_DIR}/src/ethereum/les/BREthereumLES.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/les/BREthereumLES.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/les/BREthereumLESBase.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/les/BREthereumLESFrameCoder.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/les/BREthereumLESFrameCoder.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/les/BREthereumLESRandom.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/les/BREthereumLESRandom.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/les/BREthereumMessage.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/les/BREthereumMessage.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/les/BREthereumNode.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/les/BREthereumNode.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/les/BREthereumNodeEndpoint.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/les/BREthereumNodeEndpoint.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/les/BREthereumProvision.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/les/BREthereumProvision.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/les/msg/BREthereumMessageP2P.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/les/msg/BREthereumMessageP2P.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/les/msg/BREthereumMessageDIS.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/les/msg/BREthereumMessageDIS.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/les/msg/BREthereumMessageETH.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/les/msg/BREthereumMessageLES.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/les/msg/BREthereumMessageLES.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/les/msg/BREthereumMessagePIP.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/les/msg/BREthereumMessagePIP.c
                # BCS
                ${PROJECT_SOURCE_DIR}/src/ethereum/bcs/BREthereumBCS.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/bcs/BREthereumBCS.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/bcs/BREthereumBCSEvent.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/bcs/BREthereumBCSPrivate.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/bcs/BREthereumBCSSync.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/bcs/BREthereumBlockChainSlice.h
                #EWM
                ${PROJECT_SOURCE_DIR}/src/ethereum/ewm/BREthereumBase.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/ewm/BREthereumAccount.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/ewm/BREthereumAccount.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/ewm/BREthereumAmount.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/ewm/BREthereumAmount.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/ewm/BREthereumTransfer.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/ewm/BREthereumTransfer.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/ewm/BREthereumWallet.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/ewm/BREthereumWallet.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/ewm/BREthereumClient.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/ewm/BREthereumEWM.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/ewm/BREthereumEWM.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/ewm/BREthereumEWMClient.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/ewm/BREthereumEWMEvent.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/ewm/BREthereumEWMPersist.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/ewm/BREthereumEWMPrivate.h)

# Ethereum Tests
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_sources (corecrypto
                    PRIVATE
                    ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/ethereum/testBase.c
                    ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/ethereum/testBc.c
                    ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/ethereum/testContract.c
                    ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/ethereum/testEvent.c
                    ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/ethereum/testEwm.c
                    ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/ethereum/testRlp.c
                    ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/ethereum/testLES.c
                    ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/ethereum/testUtil.c
                    ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/ethereum/test.c
                    ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/include/test.h)

    target_include_directories(corecrypto
            PRIVATE
            ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/include)
endif(CMAKE_BUILD_TYPE MATCHES Debug)

# Ripple
target_sources (corecrypto
                PRIVATE
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRipplePrivateStructs.h
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleBase.h
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRipple.h
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleAccount.c
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleAccount.h
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleAddress.c
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleAddress.h
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleFeeBasis.c
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleFeeBasis.h
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleSerialize.c
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleSerialize.h
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleSignature.c
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleSignature.h
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleTransaction.c
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleTransaction.h
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleTransfer.c
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleTransfer.h
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleUtils.c
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleUtils.h
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleWallet.c
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleWallet.h)

# Ripple Tests
if (CMAKE_BUILD_TYPE MATCHES Debug)
   target_sources (corecrypto
                   PRIVATE
                   ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/ripple/testRipple.c
                   ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/ripple/testRippleTxList1.h
                   ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/ripple/testRippleTxList2.h)
endif(CMAKE_BUILD_TYPE MATCHES Debug)

# Hedera
target_sources (corecrypto
                PRIVATE
                ${PROJECT_SOURCE_DIR}/src/hedera/BRHederaBase.h
                ${PROJECT_SOURCE_DIR}/src/hedera/BRHedera.h
                ${PROJECT_SOURCE_DIR}/src/hedera/BRHederaAccount.c
                ${PROJECT_SOURCE_DIR}/src/hedera/BRHederaAccount.h
                ${PROJECT_SOURCE_DIR}/src/hedera/BRHederaAddress.c
                ${PROJECT_SOURCE_DIR}/src/hedera/BRHederaAddress.h
                ${PROJECT_SOURCE_DIR}/src/hedera/BRHederaCrypto.c
                ${PROJECT_SOURCE_DIR}/src/hedera/BRHederaCrypto.h
                ${PROJECT_SOURCE_DIR}/src/hedera/BRHederaFeeBasis.c
                ${PROJECT_SOURCE_DIR}/src/hedera/BRHederaFeeBasis.h
                ${PROJECT_SOURCE_DIR}/src/hedera/BRHederaSerialize.c
                ${PROJECT_SOURCE_DIR}/src/hedera/BRHederaSerialize.h
                ${PROJECT_SOURCE_DIR}/src/hedera/BRHederaTransaction.c
                ${PROJECT_SOURCE_DIR}/src/hedera/BRHederaTransaction.h
                ${PROJECT_SOURCE_DIR}/src/hedera/BRHederaWallet.h
                ${PROJECT_SOURCE_DIR}/src/hedera/BRHederaWallet.c
                ${PROJECT_SOURCE_DIR}/src/hedera/proto/BasicTypes.pb-c.c
                ${PROJECT_SOURCE_DIR}/src/hedera/proto/BasicTypes.pb-c.h
                ${PROJECT_SOURCE_DIR}/src/hedera/proto/CryptoTransfer.pb-c.c
                ${PROJECT_SOURCE_DIR}/src/hedera/proto/CryptoTransfer.pb-c.h
                ${PROJECT_SOURCE_DIR}/src/hedera/proto/Duration.pb-c.c
                ${PROJECT_SOURCE_DIR}/src/hedera/proto/Duration.pb-c.h
                ${PROJECT_SOURCE_DIR}/src/hedera/proto/protobuf-c.c
                ${PROJECT_SOURCE_DIR}/src/hedera/proto/protobuf-c.h
                ${PROJECT_SOURCE_DIR}/src/hedera/proto/Timestamp.pb-c.c
                ${PROJECT_SOURCE_DIR}/src/hedera/proto/Timestamp.pb-c.h
                ${PROJECT_SOURCE_DIR}/src/hedera/proto/Transaction.pb-c.c
                ${PROJECT_SOURCE_DIR}/src/hedera/proto/Transaction.pb-c.h
                ${PROJECT_SOURCE_DIR}/src/hedera/proto/TransactionBody.pb-c.c
                ${PROJECT_SOURCE_DIR}/src/hedera/proto/TransactionBody.pb-c.h)

# Hedera Tests
if (CMAKE_BUILD_TYPE MATCHES Debug)
   target_sources (corecrypto
                   PRIVATE
                   ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/hedera/testHedera.c)
endif(CMAKE_BUILD_TYPE MATCHES Debug)

# Tezos
target_sources (corecrypto
        PRIVATE
        ${PROJECT_SOURCE_DIR}/src/tezos/BRTezos.h
        ${PROJECT_SOURCE_DIR}/src/tezos/BRTezosAccount.c
        ${PROJECT_SOURCE_DIR}/src/tezos/BRTezosAccount.h
        ${PROJECT_SOURCE_DIR}/src/tezos/BRTezosAddress.c
        ${PROJECT_SOURCE_DIR}/src/tezos/BRTezosAddress.h
        ${PROJECT_SOURCE_DIR}/src/tezos/BRTezosBase.h
        ${PROJECT_SOURCE_DIR}/src/tezos/BRTezosEncoder.c
        ${PROJECT_SOURCE_DIR}/src/tezos/BRTezosEncoder.h
        ${PROJECT_SOURCE_DIR}/src/tezos/BRTezosFeeBasis.c
        ${PROJECT_SOURCE_DIR}/src/tezos/BRTezosFeeBasis.h
        ${PROJECT_SOURCE_DIR}/src/tezos/BRTezosTransaction.c
        ${PROJECT_SOURCE_DIR}/src/tezos/BRTezosTransaction.h
        ${PROJECT_SOURCE_DIR}/src/tezos/BRTezosTransfer.c
        ${PROJECT_SOURCE_DIR}/src/tezos/BRTezosTransfer.h
        ${PROJECT_SOURCE_DIR}/src/tezos/BRTezosWallet.c
        ${PROJECT_SOURCE_DIR}/src/tezos/BRTezosWallet.h)

# Tezos Tests
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_sources (corecrypto
            PRIVATE
            ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/tezos/testTezos.c)
endif(CMAKE_BUILD_TYPE MATCHES Debug)

# Generic
target_sources (corecrypto
                PRIVATE
                ${PROJECT_SOURCE_DIR}/src/generic/BRGenericBase.h
                ${PROJECT_SOURCE_DIR}/src/generic/BRGeneric.c
                ${PROJECT_SOURCE_DIR}/src/generic/BRGeneric.h
                ${PROJECT_SOURCE_DIR}/src/generic/BRGenericClient.c
                ${PROJECT_SOURCE_DIR}/src/generic/BRGenericClient.h
                ${PROJECT_SOURCE_DIR}/src/generic/BRGenericHandlers.c
                ${PROJECT_SOURCE_DIR}/src/generic/BRGenericHandlers.h
                ${PROJECT_SOURCE_DIR}/src/generic/BRGenericManager.c
                ${PROJECT_SOURCE_DIR}/src/generic/BRGenericRipple.h
                ${PROJECT_SOURCE_DIR}/src/generic/BRGenericRipple.c
                ${PROJECT_SOURCE_DIR}/src/generic/BRGenericHedera.h
                ${PROJECT_SOURCE_DIR}/src/generic/BRGenericHedera.c
                ${PROJECT_SOURCE_DIR}/src/generic/BRGenericTezos.h
                ${PROJECT_SOURCE_DIR}/src/generic/BRGenericTezos.c)

# Crypto
target_sources (corecrypto
                PRIVATE
                #  Dependency Order
                ${PROJECT_SOURCE_DIR}/include/BRCryptoBase.h
                ${PROJECT_SOURCE_DIR}/src/crypto/BRCryptoBaseP.h

                ${PROJECT_SOURCE_DIR}/src/version/BRCryptoVersion.c

                ${PROJECT_SOURCE_DIR}/include/BRCryptoStatus.h
                ${PROJECT_SOURCE_DIR}/src/crypto/BRCryptoStatusP.h
                ${PROJECT_SOURCE_DIR}/src/crypto/BRCryptoStatus.c

                ${PROJECT_SOURCE_DIR}/include/BRCryptoHash.h
                ${PROJECT_SOURCE_DIR}/src/crypto/BRCryptoHashP.h
                ${PROJECT_SOURCE_DIR}/src/crypto/BRCryptoHash.c

                ${PROJECT_SOURCE_DIR}/include/BRCryptoCurrency.h
                ${PROJECT_SOURCE_DIR}/src/crypto/BRCryptoCurrency.c

                ${PROJECT_SOURCE_DIR}/include/BRCryptoUnit.h
                ${PROJECT_SOURCE_DIR}/src/crypto/BRCryptoUnit.c

                ${PROJECT_SOURCE_DIR}/include/BRCryptoAmount.h
                ${PROJECT_SOURCE_DIR}/src/crypto/BRCryptoAmountP.h
                ${PROJECT_SOURCE_DIR}/src/crypto/BRCryptoAmount.c

                ${PROJECT_SOURCE_DIR}/include/BRCryptoKey.h
                ${PROJECT_SOURCE_DIR}/src/crypto/BRCryptoKeyP.h
                ${PROJECT_SOURCE_DIR}/src/crypto/BRCryptoKey.c

                ${PROJECT_SOURCE_DIR}/include/BRCryptoAccount.h
                ${PROJECT_SOURCE_DIR}/src/crypto/BRCryptoAccountP.h
                ${PROJECT_SOURCE_DIR}/src/crypto/BRCryptoAccount.c

                ${PROJECT_SOURCE_DIR}/include/BRCryptoFeeBasis.h
                ${PROJECT_SOURCE_DIR}/src/crypto/BRCryptoFeeBasisP.h
                ${PROJECT_SOURCE_DIR}/src/crypto/BRCryptoFeeBasis.c

                ${PROJECT_SOURCE_DIR}/include/BRCryptoNetwork.h
                ${PROJECT_SOURCE_DIR}/src/crypto/BRCryptoNetworkP.h
                ${PROJECT_SOURCE_DIR}/src/crypto/BRCryptoNetwork.c

                ${PROJECT_SOURCE_DIR}/include/BRCryptoAddress.h
                ${PROJECT_SOURCE_DIR}/src/crypto/BRCryptoAddressP.h
                ${PROJECT_SOURCE_DIR}/src/crypto/BRCryptoAddress.c

                ${PROJECT_SOURCE_DIR}/include/BRCryptoPeer.h
                ${PROJECT_SOURCE_DIR}/src/crypto/BRCryptoPeer.c

                ${PROJECT_SOURCE_DIR}/include/BRCryptoTransfer.h
                ${PROJECT_SOURCE_DIR}/src/crypto/BRCryptoTransferP.h
                ${PROJECT_SOURCE_DIR}/src/crypto/BRCryptoTransfer.c

                ${PROJECT_SOURCE_DIR}/include/BRCryptoPayment.h
                ${PROJECT_SOURCE_DIR}/src/crypto/BRCryptoPaymentP.h
                ${PROJECT_SOURCE_DIR}/src/crypto/BRCryptoPayment.c

                ${PROJECT_SOURCE_DIR}/include/BRCryptoWallet.h
                ${PROJECT_SOURCE_DIR}/src/crypto/BRCryptoWalletP.h
                ${PROJECT_SOURCE_DIR}/src/crypto/BRCryptoWallet.c

                ${PROJECT_SOURCE_DIR}/include/BRCryptoSync.h

                ${PROJECT_SOURCE_DIR}/include/BRCryptoWalletManagerClient.h
                ${PROJECT_SOURCE_DIR}/src/crypto/BRCryptoWalletManagerClient.c

                ${PROJECT_SOURCE_DIR}/include/BRCryptoWalletManager.h
                ${PROJECT_SOURCE_DIR}/src/crypto/BRCryptoWalletManagerP.h
                ${PROJECT_SOURCE_DIR}/src/crypto/BRCryptoWalletManager.c

                ${PROJECT_SOURCE_DIR}/include/BRCryptoCipher.h
                ${PROJECT_SOURCE_DIR}/src/crypto/BRCryptoCipher.c

                ${PROJECT_SOURCE_DIR}/include/BRCryptoCoder.h
                ${PROJECT_SOURCE_DIR}/src/crypto/BRCryptoCoder.c

                ${PROJECT_SOURCE_DIR}/include/BRCryptoHasher.h
                ${PROJECT_SOURCE_DIR}/src/crypto/BRCryptoHasher.c

                ${PROJECT_SOURCE_DIR}/include/BRCryptoSigner.h
                ${PROJECT_SOURCE_DIR}/src/crypto/BRCryptoSigner.c
                )

# Crypto Tests
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_sources (corecrypto
                    PRIVATE
                    ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/crypto/testCrypto.c)
endif(CMAKE_BUILD_TYPE MATCHES Debug)

# Setup all the include paths
target_include_directories (corecrypto
                            PUBLIC
                            ${PROJECT_SOURCE_DIR}/include
                            ${PROJECT_SOURCE_DIR}/src
                            ${PROJECT_SOURCE_DIR}/src/support
                            ${PROJECT_SOURCE_DIR}
                            ${PROJECT_SOURCE_DIR}/vendor
                            ${PROJECT_SOURCE_DIR}/vendor/secp256k1)

# Set the DEBUG define when building DEBUG builds
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(corecrypto
                               PUBLIC
                               "DEBUG"
                               "BITCOIN_DEBUG")
endif (CMAKE_BUILD_TYPE MATCHES Debug)


# Link in the static sqlite3 library
target_link_libraries (corecrypto
                       sqlite3
                       ed25519
                       blake2)

# On Android, link in the log library
if (ANDROID)
    find_library (log-lib
                  log)
    target_link_libraries (corecrypto
                           ${log-lib})
endif(ANDROID)

# On Apple (iOS/macOS), link in the resolv library and Security framework
if (APPLE)
    find_library (log-lib
                  log)
    target_link_libraries (corecrypto
                           "resolv"
                           "-framework Security")
endif(APPLE)
