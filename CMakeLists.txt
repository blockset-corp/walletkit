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
                ${PROJECT_SOURCE_DIR}/WalletKitCore/vendor/sqlite3/sqlite3.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/vendor/sqlite3/sqlite3.h)

target_include_directories(sqlite3
                           INTERFACE
                           ${PROJECT_SOURCE_DIR}/WalletKitCore/vendor/sqlite3)

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
                ${PROJECT_SOURCE_DIR}/WalletKitCore/vendor/ed25519/ed25519.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/vendor/ed25519/fe.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/vendor/ed25519/fe.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/vendor/ed25519/fixedint.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/vendor/ed25519/ge.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/vendor/ed25519/ge.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/vendor/ed25519/keypair.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/vendor/ed25519/precomp_data.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/vendor/ed25519/sc.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/vendor/ed25519/sc.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/vendor/ed25519/sha512.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/vendor/ed25519/sha512.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/vendor/ed25519/sign.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/vendor/ed25519/verify.c)

target_include_directories(ed25519
                           INTERFACE
                           ${PROJECT_SOURCE_DIR}/WalletKitCore/vendor/ed25519)


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
                ${PROJECT_SOURCE_DIR}/WalletKitCore/support/BROSCompat.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/support/BROSCompat.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/support/BRAddress.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/support/BRAddress.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/support/BRAssert.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/support/BRAssert.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/support/BRArray.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/support/BRBase.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/support/BRBIP32Sequence.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/support/BRBIP32Sequence.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/support/BRBIP39Mnemonic.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/support/BRBIP39Mnemonic.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/support/BRBIP39WordsEn.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/support/BRBase58.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/support/BRBase58.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/support/BRBech32.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/support/BRBech32.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/support/BRCrypto.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/support/BRCrypto.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/support/BRFileService.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/support/BRFileService.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/support/BRInt.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/support/BRKey.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/support/BRKey.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/support/BRKeyECIES.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/support/BRKeyECIES.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/support/BRSet.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/support/BRSet.h)

# Support Tests
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_sources (corecrypto
                    PRIVATE
                    ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/support/testSup.c)
endif(CMAKE_BUILD_TYPE MATCHES Debug)

# Bitcoin
target_sources (corecrypto
                PRIVATE
                ${PROJECT_SOURCE_DIR}/WalletKitCore/bitcoin/BRBIP38Key.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/bitcoin/BRBIP38Key.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/bitcoin/BRBloomFilter.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/bitcoin/BRBloomFilter.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/bitcoin/BRChainParams.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/bitcoin/BRChainParams.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/bitcoin/BRMerkleBlock.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/bitcoin/BRMerkleBlock.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/bitcoin/BRPaymentProtocol.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/bitcoin/BRPaymentProtocol.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/bitcoin/BRPeer.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/bitcoin/BRPeer.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/bitcoin/BRPeerManager.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/bitcoin/BRPeerManager.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/bitcoin/BRSyncManager.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/bitcoin/BRSyncManager.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/bitcoin/BRTransaction.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/bitcoin/BRTransaction.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/bitcoin/BRWallet.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/bitcoin/BRWallet.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/bitcoin/BRWalletManager.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/bitcoin/BRWalletManager.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/bitcoin/BRWalletManagerEvent.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/bitcoin/BRWalletManagerPrivate.h)

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
                ${PROJECT_SOURCE_DIR}/WalletKitCore/bcash/BRBCashAddr.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/bcash/BRBCashAddr.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/bcash/BRBCashParams.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/bcash/BRBCashParams.c)

# Ethereum
target_sources (corecrypto
                PRIVATE
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/BREthereum.h
                # Util
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/util/BRKeccak.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/util/BRKeccak.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/util/BRUtil.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/util/BRUtilHex.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/util/BRUtilHex.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/util/BRUtilMath.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/util/BRUtilMath.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/util/BRUtilMathParse.c
                # RLP
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/rlp/BRRlp.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/rlp/BRRlpCoder.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/rlp/BRRlpCoder.h
                # Event
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/event/BREvent.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/event/BREvent.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/event/BREventAlarm.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/event/BREventAlarm.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/event/BREventQueue.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/event/BREventQueue.h
                # Base
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/base/BREthereumAddress.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/base/BREthereumAddress.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/base/BREthereumBase.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/base/BREthereumEther.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/base/BREthereumEther.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/base/BREthereumGas.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/base/BREthereumGas.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/base/BREthereumFeeBasis.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/base/BREthereumFeeBasis.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/base/BREthereumHash.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/base/BREthereumHash.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/base/BREthereumData.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/base/BREthereumData.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/base/BREthereumLogic.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/base/BREthereumSignature.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/base/BREthereumSignature.h
                # Block Chain
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/blockchain/BREthereumAccountState.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/blockchain/BREthereumAccountState.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/blockchain/BREthereumBlock.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/blockchain/BREthereumBlock.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/blockchain/BREthereumBlockChain.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/blockchain/BREthereumBloomFilter.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/blockchain/BREthereumBloomFilter.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/blockchain/BREthereumLog.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/blockchain/BREthereumLog.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/blockchain/BREthereumNetwork.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/blockchain/BREthereumNetwork.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/blockchain/BREthereumTransaction.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/blockchain/BREthereumTransaction.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/blockchain/BREthereumTransactionReceipt.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/blockchain/BREthereumTransactionReceipt.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/blockchain/BREthereumTransactionStatus.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/blockchain/BREthereumTransactionStatus.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/blockchain/BREthereumProofOfWork.c
                # Contract
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/contract/BREthereumContract.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/contract/BREthereumContract.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/contract/BREthereumToken.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/contract/BREthereumToken.h
                # MPT
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/mpt/BREthereumMPT.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/mpt/BREthereumMPT.h
                # LES Msg
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/les/msg/BREthereumMessageDIS.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/les/msg/BREthereumMessageDIS.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/les/msg/BREthereumMessageETH.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/les/msg/BREthereumMessageLES.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/les/msg/BREthereumMessageLES.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/les/msg/BREthereumMessageP2P.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/les/msg/BREthereumMessageP2P.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/les/msg/BREthereumMessagePIP.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/les/msg/BREthereumMessagePIP.h
                # LES
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/les/BREthereumLES.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/les/BREthereumLES.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/les/BREthereumLESBase.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/les/BREthereumLESFrameCoder.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/les/BREthereumLESFrameCoder.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/les/BREthereumLESRandom.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/les/BREthereumLESRandom.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/les/BREthereumMessage.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/les/BREthereumMessage.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/les/BREthereumNode.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/les/BREthereumNode.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/les/BREthereumNodeEndpoint.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/les/BREthereumNodeEndpoint.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/les/BREthereumProvision.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/les/BREthereumProvision.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/les/msg/BREthereumMessageP2P.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/les/msg/BREthereumMessageP2P.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/les/msg/BREthereumMessageDIS.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/les/msg/BREthereumMessageDIS.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/les/msg/BREthereumMessageETH.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/les/msg/BREthereumMessageLES.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/les/msg/BREthereumMessageLES.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/les/msg/BREthereumMessagePIP.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/les/msg/BREthereumMessagePIP.c
                # BCS
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/bcs/BREthereumBCS.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/bcs/BREthereumBCS.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/bcs/BREthereumBCSEvent.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/bcs/BREthereumBCSPrivate.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/bcs/BREthereumBCSSync.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/bcs/BREthereumBlockChainSlice.h
                #EWM
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/ewm/BREthereumBase.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/ewm/BREthereumAccount.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/ewm/BREthereumAccount.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/ewm/BREthereumAmount.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/ewm/BREthereumAmount.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/ewm/BREthereumTransfer.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/ewm/BREthereumTransfer.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/ewm/BREthereumWallet.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/ewm/BREthereumWallet.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/ewm/BREthereumClient.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/ewm/BREthereumEWM.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/ewm/BREthereumEWM.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/ewm/BREthereumEWMClient.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/ewm/BREthereumEWMEvent.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/ewm/BREthereumEWMPersist.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum/ewm/BREthereumEWMPrivate.h)

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
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ripple/BRRipplePrivateStructs.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ripple/BRRippleBase.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ripple/BRRipple.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ripple/BRRippleAccount.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ripple/BRRippleAccount.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ripple/BRRippleAddress.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ripple/BRRippleAddress.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ripple/BRRippleBase58.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ripple/BRRippleBase58.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ripple/BRRippleFeeBasis.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ripple/BRRippleFeeBasis.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ripple/BRRippleSerialize.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ripple/BRRippleSerialize.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ripple/BRRippleSignature.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ripple/BRRippleSignature.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ripple/BRRippleTransaction.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ripple/BRRippleTransaction.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ripple/BRRippleTransfer.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ripple/BRRippleTransfer.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ripple/BRRippleUtils.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ripple/BRRippleUtils.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ripple/BRRippleWallet.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/ripple/BRRippleWallet.h)

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
                ${PROJECT_SOURCE_DIR}/WalletKitCore/hedera/BRHederaBase.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/hedera/BRHedera.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/hedera/BRHederaAccount.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/hedera/BRHederaAccount.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/hedera/BRHederaAddress.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/hedera/BRHederaAddress.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/hedera/BRHederaCrypto.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/hedera/BRHederaCrypto.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/hedera/BRHederaFeeBasis.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/hedera/BRHederaFeeBasis.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/hedera/BRHederaSerialize.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/hedera/BRHederaSerialize.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/hedera/BRHederaTransaction.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/hedera/BRHederaTransaction.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/hedera/BRHederaWallet.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/hedera/BRHederaWallet.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/hedera/proto/BasicTypes.pb-c.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/hedera/proto/BasicTypes.pb-c.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/hedera/proto/CryptoTransfer.pb-c.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/hedera/proto/CryptoTransfer.pb-c.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/hedera/proto/Duration.pb-c.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/hedera/proto/Duration.pb-c.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/hedera/proto/protobuf-c.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/hedera/proto/protobuf-c.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/hedera/proto/Timestamp.pb-c.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/hedera/proto/Timestamp.pb-c.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/hedera/proto/Transaction.pb-c.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/hedera/proto/Transaction.pb-c.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/hedera/proto/TransactionBody.pb-c.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/hedera/proto/TransactionBody.pb-c.h)

# Hedera Tests
if (CMAKE_BUILD_TYPE MATCHES Debug)
   target_sources (corecrypto
                   PRIVATE
                   ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/hedera/testHedera.c)
endif(CMAKE_BUILD_TYPE MATCHES Debug)

# Generic
target_sources (corecrypto
                PRIVATE
                ${PROJECT_SOURCE_DIR}/WalletKitCore/generic/BRGenericBase.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/generic/BRGeneric.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/generic/BRGeneric.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/generic/BRGenericClient.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/generic/BRGenericClient.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/generic/BRGenericHandlers.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/generic/BRGenericHandlers.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/generic/BRGenericManager.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/generic/BRGenericRipple.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/generic/BRGenericRipple.c
                ${PROJECT_SOURCE_DIR}/WalletKitCore/generic/BRGenericHedera.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/generic/BRGenericHedera.c)

# Crypto
target_sources (corecrypto
                PRIVATE
                #  Dependency Order
                ${PROJECT_SOURCE_DIR}/WalletKitCore/include/BRCryptoBase.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto/BRCryptoBaseP.h

                ${PROJECT_SOURCE_DIR}/WalletKitCore/version/BRCryptoVersion.c

                ${PROJECT_SOURCE_DIR}/WalletKitCore/include/BRCryptoStatus.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto/BRCryptoStatusP.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto/BRCryptoStatus.c

                ${PROJECT_SOURCE_DIR}/WalletKitCore/include/BRCryptoHash.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto/BRCryptoHashP.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto/BRCryptoHash.c

                ${PROJECT_SOURCE_DIR}/WalletKitCore/include/BRCryptoCurrency.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto/BRCryptoCurrency.c

                ${PROJECT_SOURCE_DIR}/WalletKitCore/include/BRCryptoUnit.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto/BRCryptoUnit.c

                ${PROJECT_SOURCE_DIR}/WalletKitCore/include/BRCryptoAmount.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto/BRCryptoAmountP.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto/BRCryptoAmount.c

                ${PROJECT_SOURCE_DIR}/WalletKitCore/include/BRCryptoKey.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto/BRCryptoKeyP.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto/BRCryptoKey.c

                ${PROJECT_SOURCE_DIR}/WalletKitCore/include/BRCryptoAccount.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto/BRCryptoAccountP.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto/BRCryptoAccount.c

                ${PROJECT_SOURCE_DIR}/WalletKitCore/include/BRCryptoFeeBasis.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto/BRCryptoFeeBasisP.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto/BRCryptoFeeBasis.c

                ${PROJECT_SOURCE_DIR}/WalletKitCore/include/BRCryptoNetwork.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto/BRCryptoNetworkP.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto/BRCryptoNetwork.c

                ${PROJECT_SOURCE_DIR}/WalletKitCore/include/BRCryptoAddress.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto/BRCryptoAddressP.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto/BRCryptoAddress.c

                ${PROJECT_SOURCE_DIR}/WalletKitCore/include/BRCryptoPeer.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto/BRCryptoPeer.c

                ${PROJECT_SOURCE_DIR}/WalletKitCore/include/BRCryptoTransfer.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto/BRCryptoTransferP.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto/BRCryptoTransfer.c

                ${PROJECT_SOURCE_DIR}/WalletKitCore/include/BRCryptoPayment.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto/BRCryptoPaymentP.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto/BRCryptoPayment.c

                ${PROJECT_SOURCE_DIR}/WalletKitCore/include/BRCryptoWallet.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto/BRCryptoWalletP.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto/BRCryptoWallet.c

                ${PROJECT_SOURCE_DIR}/WalletKitCore/include/BRCryptoSync.h

                ${PROJECT_SOURCE_DIR}/WalletKitCore/include/BRCryptoWalletManagerClient.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto/BRCryptoWalletManagerClient.c

                ${PROJECT_SOURCE_DIR}/WalletKitCore/include/BRCryptoWalletManager.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto/BRCryptoWalletManagerP.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto/BRCryptoWalletManager.c

                ${PROJECT_SOURCE_DIR}/WalletKitCore/include/BRCryptoCipher.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto/BRCryptoCipher.c

                ${PROJECT_SOURCE_DIR}/WalletKitCore/include/BRCryptoCoder.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto/BRCryptoCoder.c

                ${PROJECT_SOURCE_DIR}/WalletKitCore/include/BRCryptoHasher.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto/BRCryptoHasher.c

                ${PROJECT_SOURCE_DIR}/WalletKitCore/include/BRCryptoSigner.h
                ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto/BRCryptoSigner.c
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
                            ${PROJECT_SOURCE_DIR}/WalletKitCore
                            ${PROJECT_SOURCE_DIR}/WalletKitCore/include
                            ${PROJECT_SOURCE_DIR}/WalletKitCore/support
                            ${PROJECT_SOURCE_DIR}/WalletKitCore/bitcoin
                            ${PROJECT_SOURCE_DIR}/WalletKitCore/bcash
                            ${PROJECT_SOURCE_DIR}/WalletKitCore/ethereum
                            ${PROJECT_SOURCE_DIR}/WalletKitCore/vendor/secp256k1
                            ${PROJECT_SOURCE_DIR}/WalletKitCore/crypto)

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
                       ed25519)

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
