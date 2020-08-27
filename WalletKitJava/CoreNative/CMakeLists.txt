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
                src/main/cpp/core/vendor/sqlite3/sqlite3.c
                src/main/cpp/core/vendor/sqlite3/sqlite3.h)

target_include_directories(sqlite3
                           INTERFACE
                           ${PROJECT_SOURCE_DIR}/src/main/cpp/core/vendor/sqlite3)

target_compile_definitions (sqlite3
                            PRIVATE
                            _HAVE_SQLITE_CONFIG_H=1)

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
                src/main/cpp/core/src/support/BRAddress.c
                src/main/cpp/core/src/support/BRAddress.h
                src/main/cpp/core/src/support/BRAssert.c
                src/main/cpp/core/src/support/BRAssert.h
                src/main/cpp/core/src/support/BRArray.h
                src/main/cpp/core/src/support/BRBase.h
                src/main/cpp/core/src/support/BRBIP32Sequence.c
                src/main/cpp/core/src/support/BRBIP32Sequence.h
                src/main/cpp/core/src/support/BRBIP39Mnemonic.c
                src/main/cpp/core/src/support/BRBIP39Mnemonic.h
                src/main/cpp/core/src/support/BRBIP39WordsEn.h
                src/main/cpp/core/src/support/BRBase58.c
                src/main/cpp/core/src/support/BRBase58.h
                src/main/cpp/core/src/support/BRBech32.c
                src/main/cpp/core/src/support/BRBech32.h
                src/main/cpp/core/src/support/BRCrypto.c
                src/main/cpp/core/src/support/BRCrypto.h
                src/main/cpp/core/src/support/BRFileService.c
                src/main/cpp/core/src/support/BRFileService.h
                src/main/cpp/core/src/support/BRInt.h
                src/main/cpp/core/src/support/BRKey.c
                src/main/cpp/core/src/support/BRKey.h
                src/main/cpp/core/src/support/BRKeyECIES.c
                src/main/cpp/core/src/support/BRKeyECIES.h
                src/main/cpp/core/src/support/BRSet.c
                src/main/cpp/core/src/support/BRSet.h)

# Support Tests
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_sources (corecrypto
                    PRIVATE
                    src/main/cpp/core/src/support/testSup.c)
endif(CMAKE_BUILD_TYPE MATCHES Debug)

# Bitcoin
target_sources (corecrypto
                PRIVATE
                src/main/cpp/core/src/bitcoin/BRBIP38Key.c
                src/main/cpp/core/src/bitcoin/BRBIP38Key.h
                src/main/cpp/core/src/bitcoin/BRBloomFilter.c
                src/main/cpp/core/src/bitcoin/BRBloomFilter.h
                src/main/cpp/core/src/bitcoin/BRChainParams.h
                src/main/cpp/core/src/bitcoin/BRChainParams.c
                src/main/cpp/core/src/bitcoin/BRMerkleBlock.c
                src/main/cpp/core/src/bitcoin/BRMerkleBlock.h
                src/main/cpp/core/src/bitcoin/BRPaymentProtocol.c
                src/main/cpp/core/src/bitcoin/BRPaymentProtocol.h
                src/main/cpp/core/src/bitcoin/BRPeer.c
                src/main/cpp/core/src/bitcoin/BRPeer.h
                src/main/cpp/core/src/bitcoin/BRPeerManager.c
                src/main/cpp/core/src/bitcoin/BRPeerManager.h
                src/main/cpp/core/src/bitcoin/BRTransaction.c
                src/main/cpp/core/src/bitcoin/BRTransaction.h
                src/main/cpp/core/src/bitcoin/BRWallet.c
                src/main/cpp/core/src/bitcoin/BRWallet.h)

# Bitcoin Tests
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_sources (corecrypto
                    PRIVATE
                    src/main/cpp/core/src/bitcoin/test.c
                    src/main/cpp/core/src/bitcoin/testBwm.c)
endif(CMAKE_BUILD_TYPE MATCHES Debug)

# BCash
target_sources (corecrypto
                PRIVATE
                src/main/cpp/core/src/bcash/BRBCashAddr.c
                src/main/cpp/core/src/bcash/BRBCashAddr.h
                src/main/cpp/core/src/bcash/BRBCashParams.h
                src/main/cpp/core/src/bcash/BRBCashParams.c)

# Ethereum
target_sources (corecrypto
                PRIVATE
                # Util
                src/main/cpp/core/src/ethereum/util/BRKeccak.c
                src/main/cpp/core/src/ethereum/util/BRKeccak.h
                src/main/cpp/core/src/ethereum/util/BRUtil.h
                src/main/cpp/core/src/ethereum/util/BRUtilHex.c
                src/main/cpp/core/src/ethereum/util/BRUtilHex.h
                src/main/cpp/core/src/ethereum/util/BRUtilMath.c
                src/main/cpp/core/src/ethereum/util/BRUtilMath.h
                src/main/cpp/core/src/ethereum/util/BRUtilMathParse.c
                # RLP
                src/main/cpp/core/src/ethereum/rlp/BRRlp.h
                src/main/cpp/core/src/ethereum/rlp/BRRlpCoder.c
                src/main/cpp/core/src/ethereum/rlp/BRRlpCoder.h
                # Event
                src/main/cpp/core/src/ethereum/event/BREvent.c
                src/main/cpp/core/src/ethereum/event/BREvent.h
                src/main/cpp/core/src/ethereum/event/BREventAlarm.c
                src/main/cpp/core/src/ethereum/event/BREventAlarm.h
                src/main/cpp/core/src/ethereum/event/BREventQueue.c
                src/main/cpp/core/src/ethereum/event/BREventQueue.h
                # Base
                src/main/cpp/core/src/ethereum/base/BREthereumAddress.c
                src/main/cpp/core/src/ethereum/base/BREthereumAddress.h
                src/main/cpp/core/src/ethereum/base/BREthereumBase.h
                src/main/cpp/core/src/ethereum/base/BREthereumEther.c
                src/main/cpp/core/src/ethereum/base/BREthereumEther.h
                src/main/cpp/core/src/ethereum/base/BREthereumGas.c
                src/main/cpp/core/src/ethereum/base/BREthereumGas.h
                src/main/cpp/core/src/ethereum/base/BREthereumFeeBasis.c
                src/main/cpp/core/src/ethereum/base/BREthereumFeeBasis.h
                src/main/cpp/core/src/ethereum/base/BREthereumHash.c
                src/main/cpp/core/src/ethereum/base/BREthereumHash.h
                src/main/cpp/core/src/ethereum/base/BREthereumData.c
                src/main/cpp/core/src/ethereum/base/BREthereumData.h
                src/main/cpp/core/src/ethereum/base/BREthereumLogic.h
                src/main/cpp/core/src/ethereum/base/BREthereumSignature.c
                src/main/cpp/core/src/ethereum/base/BREthereumSignature.h
                # Block Chain
                src/main/cpp/core/ethereum/blockchain/BREthereumAccount.c
                src/main/cpp/core/ethereum/blockchain/BREthereumAccount.h
                src/main/cpp/core/src/ethereum/blockchain/BREthereumAccountState.c
                src/main/cpp/core/src/ethereum/blockchain/BREthereumAccountState.h
                src/main/cpp/core/src/ethereum/blockchain/BREthereumBlock.c
                src/main/cpp/core/src/ethereum/blockchain/BREthereumBlock.h
                src/main/cpp/core/src/ethereum/blockchain/BREthereumBlockChain.h
                src/main/cpp/core/src/ethereum/blockchain/BREthereumBloomFilter.c
                src/main/cpp/core/src/ethereum/blockchain/BREthereumBloomFilter.h
                src/main/cpp/core/src/ethereum/blockchain/BREthereumLog.c
                src/main/cpp/core/src/ethereum/blockchain/BREthereumLog.h
                src/main/cpp/core/src/ethereum/blockchain/BREthereumNetwork.c
                src/main/cpp/core/src/ethereum/blockchain/BREthereumNetwork.h
                src/main/cpp/core/src/ethereum/blockchain/BREthereumTransaction.c
                src/main/cpp/core/src/ethereum/blockchain/BREthereumTransaction.h
                src/main/cpp/core/src/ethereum/blockchain/BREthereumTransactionReceipt.c
                src/main/cpp/core/src/ethereum/blockchain/BREthereumTransactionReceipt.h
                src/main/cpp/core/src/ethereum/blockchain/BREthereumTransactionStatus.c
                src/main/cpp/core/src/ethereum/blockchain/BREthereumTransactionStatus.h
                src/main/cpp/core/src/ethereum/blockchain/BREthereumProofOfWork.c
                # Contract
                src/main/cpp/core/src/ethereum/contract/BREthereumContract.c
                src/main/cpp/core/src/ethereum/contract/BREthereumContract.h
                src/main/cpp/core/src/ethereum/contract/BREthereumToken.c
                src/main/cpp/core/src/ethereum/contract/BREthereumToken.h
                # MPT
                src/main/cpp/core/src/ethereum/mpt/BREthereumMPT.c
                src/main/cpp/core/src/ethereum/mpt/BREthereumMPT.h
                # LES Msg
                src/main/cpp/core/src/ethereum/les/msg/BREthereumMessageDIS.c
                src/main/cpp/core/src/ethereum/les/msg/BREthereumMessageDIS.h
                src/main/cpp/core/src/ethereum/les/msg/BREthereumMessageETH.h
                src/main/cpp/core/src/ethereum/les/msg/BREthereumMessageLES.c
                src/main/cpp/core/src/ethereum/les/msg/BREthereumMessageLES.h
                src/main/cpp/core/src/ethereum/les/msg/BREthereumMessageP2P.c
                src/main/cpp/core/src/ethereum/les/msg/BREthereumMessageP2P.h
                src/main/cpp/core/src/ethereum/les/msg/BREthereumMessagePIP.c
                src/main/cpp/core/src/ethereum/les/msg/BREthereumMessagePIP.h
                # LES
                src/main/cpp/core/src/ethereum/les/BREthereumLES.c
                src/main/cpp/core/src/ethereum/les/BREthereumLES.h
                src/main/cpp/core/src/ethereum/les/BREthereumLESBase.h
                src/main/cpp/core/src/ethereum/les/BREthereumLESFrameCoder.c
                src/main/cpp/core/src/ethereum/les/BREthereumLESFrameCoder.h
                src/main/cpp/core/src/ethereum/les/BREthereumLESRandom.c
                src/main/cpp/core/src/ethereum/les/BREthereumLESRandom.h
                src/main/cpp/core/src/ethereum/les/BREthereumMessage.c
                src/main/cpp/core/src/ethereum/les/BREthereumMessage.h
                src/main/cpp/core/src/ethereum/les/BREthereumNode.c
                src/main/cpp/core/src/ethereum/les/BREthereumNode.h
                src/main/cpp/core/src/ethereum/les/BREthereumNodeEndpoint.c
                src/main/cpp/core/src/ethereum/les/BREthereumNodeEndpoint.h
                src/main/cpp/core/src/ethereum/les/BREthereumProvision.c
                src/main/cpp/core/src/ethereum/les/BREthereumProvision.h
                src/main/cpp/core/src/ethereum/les/msg/BREthereumMessageP2P.h
                src/main/cpp/core/src/ethereum/les/msg/BREthereumMessageP2P.c
                src/main/cpp/core/src/ethereum/les/msg/BREthereumMessageDIS.h
                src/main/cpp/core/src/ethereum/les/msg/BREthereumMessageDIS.c
                src/main/cpp/core/src/ethereum/les/msg/BREthereumMessageETH.h
                src/main/cpp/core/src/ethereum/les/msg/BREthereumMessageLES.h
                src/main/cpp/core/src/ethereum/les/msg/BREthereumMessageLES.c
                src/main/cpp/core/src/ethereum/les/msg/BREthereumMessagePIP.h
                src/main/cpp/core/src/ethereum/les/msg/BREthereumMessagePIP.c
                # BCS
                src/main/cpp/core/src/ethereum/bcs/BREthereumBCS.c
                src/main/cpp/core/src/ethereum/bcs/BREthereumBCS.h
                src/main/cpp/core/src/ethereum/bcs/BREthereumBCSEvent.c
                src/main/cpp/core/src/ethereum/bcs/BREthereumBCSPrivate.h
                src/main/cpp/core/src/ethereum/bcs/BREthereumBCSSync.c
                src/main/cpp/core/src/ethereum/bcs/BREthereumBlockChainSlice.h)

# Ethereum Tests
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_sources (corecrypto
                    PRIVATE
                    src/main/cpp/core/src/ethereum/base/testBase.c
                    src/main/cpp/core/src/ethereum/blockchain/testBc.c
                    src/main/cpp/core/src/ethereum/contract/testContract.c
                    src/main/cpp/core/src/ethereum/event/testEvent.c
                    src/main/cpp/core/src/ethereum/ewm/testEwm.c
                    src/main/cpp/core/src/ethereum/rlp/testRlp.c
                    src/main/cpp/core/src/ethereum/les/testLES.c
                    src/main/cpp/core/src/ethereum/util/testUtil.c
                    src/main/cpp/core/src/ethereum/test.c)
endif(CMAKE_BUILD_TYPE MATCHES Debug)

# Ripple
target_sources (corecrypto
                PRIVATE
                src/main/cpp/core/src/ripple/BRRipplePrivateStructs.h
                src/main/cpp/core/src/ripple/BRRippleBase.h
                src/main/cpp/core/src/ripple/BRRipple.h
                src/main/cpp/core/src/ripple/BRRippleAccount.c
                src/main/cpp/core/src/ripple/BRRippleAccount.h
                src/main/cpp/core/src/ripple/BRRippleAddress.c
                src/main/cpp/core/src/ripple/BRRippleAddress.h
                src/main/cpp/core/src/ripple/BRRippleBase58.c
                src/main/cpp/core/src/ripple/BRRippleBase58.h
                src/main/cpp/core/src/ripple/BRRippleFeeBasis.c
                src/main/cpp/core/src/ripple/BRRippleFeeBasis.h
                src/main/cpp/core/src/ripple/BRRippleSerialize.c
                src/main/cpp/core/src/ripple/BRRippleSerialize.h
                src/main/cpp/core/src/ripple/BRRippleSignature.c
                src/main/cpp/core/src/ripple/BRRippleSignature.h
                src/main/cpp/core/src/ripple/BRRippleTransaction.c
                src/main/cpp/core/src/ripple/BRRippleTransaction.h
                src/main/cpp/core/src/ripple/BRRippleTransfer.c
                src/main/cpp/core/src/ripple/BRRippleTransfer.h
                src/main/cpp/core/src/ripple/BRRippleUtils.c
                src/main/cpp/core/src/ripple/BRRippleUtils.h
                src/main/cpp/core/src/ripple/BRRippleWallet.c
                src/main/cpp/core/src/ripple/BRRippleWallet.h)

# Ripple Tests
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_sources (corecrypto
                    PRIVATE
                    src/main/cpp/core/src/ripple/testRipple.c
                    src/main/cpp/core/src/ripple/testRippleTxList1.h
                    src/main/cpp/core/src/ripple/testRippleTxList2.h)
endif(CMAKE_BUILD_TYPE MATCHES Debug)

# Crypto
target_sources (corecrypto
                PRIVATE
                #  Dependency Order
                src/main/cpp/core/include/BRCryptoBase.h
                src/main/cpp/core/src/crypto/BRCryptoBaseP.h

                src/main/cpp/core/src/crypto/handlers/BRCryptoHandlersExport.h
                src/main/cpp/core/src/crypto/BRCryptoHandlersP.h
                src/main/cpp/core/src/crypto/BRCryptoHandlers.c

                src/main/cpp/core/include/BRCryptoStatus.h
                src/main/cpp/core/src/crypto/BRCryptoStatusP.h
                src/main/cpp/core/src/crypto/BRCryptoStatus.c

                src/main/cpp/core/include/BRCryptoHash.h
                src/main/cpp/core/src/crypto/BRCryptoHashP.h
                src/main/cpp/core/src/crypto/BRCryptoHash.c

                src/main/cpp/core/include/BRCryptoCurrency.h
                src/main/cpp/core/src/crypto/BRCryptoCurrency.c

                src/main/cpp/core/include/BRCryptoUnit.h
                src/main/cpp/core/src/crypto/BRCryptoUnit.c

                src/main/cpp/core/include/BRCryptoAmount.h
                src/main/cpp/core/src/crypto/BRCryptoAmountP.h
                src/main/cpp/core/src/crypto/BRCryptoAmount.c

                src/main/cpp/core/include/BRCryptoKey.h
                src/main/cpp/core/src/crypto/BRCryptoKeyP.h
                src/main/cpp/core/src/crypto/BRCryptoKey.c

                src/main/cpp/core/include/BRCryptoAccount.h
                src/main/cpp/core/src/crypto/BRCryptoAccountP.h
                src/main/cpp/core/src/crypto/BRCryptoAccount.c

                src/main/cpp/core/include/BRCryptoFeeBasis.h
                src/main/cpp/core/src/crypto/BRCryptoFeeBasisP.h
                src/main/cpp/core/src/crypto/BRCryptoFeeBasis.c

                src/main/cpp/core/include/BRCryptoNetwork.h
                src/main/cpp/core/src/crypto/BRCryptoNetworkP.h
                src/main/cpp/core/src/crypto/BRCryptoNetwork.c

                src/main/cpp/core/include/BRCryptoAddress.h
                src/main/cpp/core/src/crypto/BRCryptoAddressP.h
                src/main/cpp/core/src/crypto/BRCryptoAddress.c

                src/main/cpp/core/include/BRCryptoPeer.h
                src/main/cpp/core/src/crypto/BRCryptoPeer.c

                src/main/cpp/core/include/BRCryptoTransfer.h
                src/main/cpp/core/src/crypto/BRCryptoTransferP.h
                src/main/cpp/core/src/crypto/BRCryptoTransfer.c

                src/main/cpp/core/include/BRCryptoPayment.h
                src/main/cpp/core/src/crypto/BRCryptoPaymentP.h
                src/main/cpp/core/src/crypto/BRCryptoPayment.c

                src/main/cpp/core/include/BRCryptoWallet.h
                src/main/cpp/core/src/crypto/BRCryptoWalletP.h
                src/main/cpp/core/src/crypto/BRCryptoWallet.c

                src/main/cpp/core/include/BRCryptoSync.h

                src/main/cpp/core/include/BRCryptoClient.h
                src/main/cpp/core/src/crypto/BRCryptoClientP.h
                src/main/cpp/core/src/crypto/BRCryptoClient.c

                src/main/cpp/core/src/crypto/BRCryptoFileService.h
                src/main/cpp/core/src/crypto/BRCryptoFileService.c

                src/main/cpp/core/include/BRCryptoWalletManager.h
                src/main/cpp/core/src/crypto/BRCryptoWalletManagerP.h
                src/main/cpp/core/src/crypto/BRCryptoWalletManager.c

                src/main/cpp/core/include/BRCryptoCipher.h
                src/main/cpp/core/src/crypto/BRCryptoCipher.c

                src/main/cpp/core/include/BRCryptoCoder.h
                src/main/cpp/core/src/crypto/BRCryptoCoder.c

                src/main/cpp/core/include/BRCryptoHasher.h
                src/main/cpp/core/src/crypto/BRCryptoHasher.c

                src/main/cpp/core/include/BRCryptoSigner.h
                src/main/cpp/core/crypto/BRCryptoSigner.c

                src/main/cpp/core/include/BRCryptoWalletSweeper.h
                src/main/cpp/core/src/crypto/BRCryptoWalletSweeperP.h
                src/main/cpp/core/src/crypto/BRCryptoWalletSweeper.c
                )

# Crypto BTC
target_sources (corecrypto
                PRIVATE
                src/main/cpp/core/src/crypto/handlers/btc/BRCryptoBTC.h
                src/main/cpp/core/src/crypto/handlers/btc/BRCryptoSupportBTC.c
                src/main/cpp/core/src/crypto/handlers/btc/BRCryptoNetworkBTC.c
                src/main/cpp/core/src/crypto/handlers/btc/BRCryptoAddressBTC.c
                src/main/cpp/core/src/crypto/handlers/btc/BRCryptoTransferBTC.c
                src/main/cpp/core/src/crypto/handlers/btc/BRCryptoWalletBTC.c
                src/main/cpp/core/src/crypto/handlers/btc/BRCryptoWalletSweeperBTC.c
                src/main/cpp/core/src/crypto/handlers/btc/BRCryptoWalletManagerBTC.c
                )

# Crypto ETH
target_sources (corecrypto
                PRIVATE
                src/main/cpp/core/src/crypto/handlers/eth/BRCryptoETH.h
                src/main/cpp/core/src/crypto/handlers/eth/BRCryptoSupportETH.c
                src/main/cpp/core/src/crypto/handlers/eth/BRCryptoNetworkETH.c
                src/main/cpp/core/src/crypto/handlers/eth/BRCryptoAddressETH.c
                src/main/cpp/core/src/crypto/handlers/eth/BRCryptoTransferETH.c
                src/main/cpp/core/src/crypto/handlers/eth/BRCryptoWalletETH.c
                src/main/cpp/core/src/crypto/handlers/eth/BRCryptoWalletManagerPersistETH.c
                src/main/cpp/core/src/crypto/handlers/eth/BRCryptoWalletManagerETH.c
                )

# Crypto XRP
target_sources (corecrypto
                PRIVATE
                src/main/cpp/core/src/crypto/handlers/xrp/handlers/BRCryptoXRP.h
                src/main/cpp/core/src/crypto/handlers/xrp/BRCryptoSupportXRP.c
                src/main/cpp/core/src/crypto/handlers/xrp/BRCryptoNetworkXRP.c
                src/main/cpp/core/src/crypto/handlers/xrp/BRCryptoAddressXRP.c
                src/main/cpp/core/src/crypto/handlers/xrp/BRCryptoTransferXRP.c
                src/main/cpp/core/src/crypto/handlers/xrp/BRCryptoWalletXRP.c
                src/main/cpp/core/src/crypto/handlers/xrp/BRCryptoWalletManagerXRP.c
                )

# Crypto HBAR
target_sources (corecrypto
                PRIVATE
                src/main/cpp/core/src/crypto/handlers/hbar/handlers/BRCryptoHBAR.h
                src/main/cpp/core/src/crypto/handlers/hbar/BRCryptoSupportHBAR.c
                src/main/cpp/core/src/crypto/handlers/hbar/BRCryptoNetworkHBAR.c
                src/main/cpp/core/src/crypto/handlers/hbar/BRCryptoAddressHBAR.c
                src/main/cpp/core/src/crypto/handlers/hbar/BRCryptoTransferHBAR.c
                src/main/cpp/core/src/crypto/handlers/hbar/BRCryptoWalletHBAR.c
                src/main/cpp/core/src/crypto/handlers/hbar/BRCryptoWalletManagerHBAR.c
                )


# Crypto Tests
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_sources (corecrypto
                    PRIVATE
                    src/main/cpp/core/src/crypto/testCrypto.c)
endif(CMAKE_BUILD_TYPE MATCHES Debug)

# Setup all the include paths
target_include_directories (corecrypto
                            PUBLIC
                            ${PROJECT_SOURCE_DIR}/src/main/cpp/core
                            ${PROJECT_SOURCE_DIR}/src/main/cpp/core/include
                            ${PROJECT_SOURCE_DIR}/src/main/cpp/core/src/support
                            ${PROJECT_SOURCE_DIR}/src/main/cpp/core/src/bitcoin
                            ${PROJECT_SOURCE_DIR}/src/main/cpp/core/src/bcash
                            ${PROJECT_SOURCE_DIR}/src/main/cpp/core/src/ethereum
                            ${PROJECT_SOURCE_DIR}/src/main/cpp/core/vendor/secp256k1
                            ${PROJECT_SOURCE_DIR}/src/main/cpp/core/src/crypto)

# Set the DEBUG define when building DEBUG builds
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(corecrypto
                               PUBLIC
                               "DEBUG"
                               "BITCOIN_DEBUG")
endif (CMAKE_BUILD_TYPE MATCHES Debug)


# Link in the static sqlite3 library
target_link_libraries (corecrypto
                       sqlite3)

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
