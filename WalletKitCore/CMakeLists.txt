# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.10.2)

# Change the default RELEASE and DEBUG flags.
# Even for RELEASE build, always use DEBUG and never NDEBUG (to enable assert())
set(CMAKE_C_FLAGS_DEBUG   "-DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-Oz")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

#
# Core Crypto Library
#

add_library( # Sets the name of the library.
             corecrypto
             # Sets the library as a shared library.
             SHARED)

# Setup all the include paths
target_include_directories (corecrypto
                            PUBLIC
                            ${PROJECT_SOURCE_DIR}/include
                            ${PROJECT_SOURCE_DIR}/src
                            ${PROJECT_SOURCE_DIR}/src/support
                            ${PROJECT_SOURCE_DIR}
                            ${PROJECT_SOURCE_DIR}/vendor
                            ${PROJECT_SOURCE_DIR}/vendor/secp256k1)

# specify compiler warnings
target_compile_options (corecrypto
	   	        PUBLIC
		        "-Wall"
		        "-Wconversion"
		        "-Wsign-conversion"
		        "-Wparentheses"
		        "-Wswitch"
		        "-Wno-implicit-int-conversion"
		        "-Wno-missing-braces")

# Set BITCOIN_DEBUG define when building DEBUG builds
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(corecrypto
                               PUBLIC
                               "BITCOIN_DEBUG")
endif (CMAKE_BUILD_TYPE MATCHES Debug)

#
# Core Crypto Test Library
#
if (CMAKE_BUILD_TYPE MATCHES Debug)
   add_library(corecryptotest
	       STATIC)
   target_include_directories (corecryptotest
                               PUBLIC
                            ${PROJECT_SOURCE_DIR}/include
                            ${PROJECT_SOURCE_DIR}/src
                            ${PROJECT_SOURCE_DIR})

endif(CMAKE_BUILD_TYPE MATCHES Debug)
	     
#
# sqlite3 build
#

add_library(sqlite3
            STATIC)

target_sources (sqlite3
                PRIVATE
                ${PROJECT_SOURCE_DIR}/vendor/sqlite3/sqlite3.c
                ${PROJECT_SOURCE_DIR}/vendor/sqlite3/sqlite3.h)

target_include_directories (sqlite3
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

target_include_directories (ed25519
                            INTERFACE
                            ${PROJECT_SOURCE_DIR}/vendor/ed25519)

#
# blake2 build
#
add_library (blake2
             STATIC)

target_sources (blake2
                PRIVATE
                ${PROJECT_SOURCE_DIR}/vendor/blake2/blake2b.c
                ${PROJECT_SOURCE_DIR}/vendor/blake2/blake2b.h)

target_include_directories (blake2
                            INTERFACE
                            ${PROJECT_SOURCE_DIR}/vendor/blake2)

#
# hedera proto build
#
add_library (hederaproto
	     STATIC)

# sources added in 'hedera'
	    
#
# crypto sources
#

# Support
target_sources (corecrypto
                PRIVATE
                ${PROJECT_SOURCE_DIR}/src/support/BRAddress.c
                ${PROJECT_SOURCE_DIR}/src/support/BRAddress.h
                ${PROJECT_SOURCE_DIR}/src/support/BRArray.h
                ${PROJECT_SOURCE_DIR}/src/support/BRAssert.c
                ${PROJECT_SOURCE_DIR}/src/support/BRAssert.h
                ${PROJECT_SOURCE_DIR}/src/support/BRBase.h
                ${PROJECT_SOURCE_DIR}/src/support/BRBase58.c
                ${PROJECT_SOURCE_DIR}/src/support/BRBase58.h
                ${PROJECT_SOURCE_DIR}/src/support/BRBech32.c
                ${PROJECT_SOURCE_DIR}/src/support/BRBech32.h
                ${PROJECT_SOURCE_DIR}/src/support/BRBIP32Sequence.c
                ${PROJECT_SOURCE_DIR}/src/support/BRBIP32Sequence.h
                ${PROJECT_SOURCE_DIR}/src/support/BRBIP39Mnemonic.c
                ${PROJECT_SOURCE_DIR}/src/support/BRBIP39Mnemonic.h
                ${PROJECT_SOURCE_DIR}/src/support/BRBIP39WordsEn.h
                ${PROJECT_SOURCE_DIR}/src/support/BRCrypto.c
                ${PROJECT_SOURCE_DIR}/src/support/BRCrypto.h
                ${PROJECT_SOURCE_DIR}/src/support/BRFileService.c
                ${PROJECT_SOURCE_DIR}/src/support/BRFileService.h
                ${PROJECT_SOURCE_DIR}/src/support/BRBIP38Key.c
                ${PROJECT_SOURCE_DIR}/src/support/BRBIP38Key.h
                ${PROJECT_SOURCE_DIR}/src/support/BRInt.h
                ${PROJECT_SOURCE_DIR}/src/support/BRKey.c
                ${PROJECT_SOURCE_DIR}/src/support/BRKey.h
                ${PROJECT_SOURCE_DIR}/src/support/BRKeyECIES.c
                ${PROJECT_SOURCE_DIR}/src/support/BRKeyECIES.h
                ${PROJECT_SOURCE_DIR}/src/support/BROSCompat.c
                ${PROJECT_SOURCE_DIR}/src/support/BROSCompat.h
                ${PROJECT_SOURCE_DIR}/src/support/BRSet.c
                ${PROJECT_SOURCE_DIR}/src/support/BRSet.h
                # RLP
                ${PROJECT_SOURCE_DIR}/src/support/rlp/BRRlp.h
                ${PROJECT_SOURCE_DIR}/src/support/rlp/BRRlpCoder.c
                ${PROJECT_SOURCE_DIR}/src/support/rlp/BRRlpCoder.h
                # Event
                ${PROJECT_SOURCE_DIR}/src/support/event/BREvent.c
                ${PROJECT_SOURCE_DIR}/src/support/event/BREvent.h
                ${PROJECT_SOURCE_DIR}/src/support/event/BREventAlarm.c
                ${PROJECT_SOURCE_DIR}/src/support/event/BREventAlarm.h
                ${PROJECT_SOURCE_DIR}/src/support/event/BREventQueue.c
                ${PROJECT_SOURCE_DIR}/src/support/event/BREventQueue.h
		# Util
		${PROJECT_SOURCE_DIR}/src/support/util/BRUtil.h
                ${PROJECT_SOURCE_DIR}/src/support/util/BRUtilMath.c
                ${PROJECT_SOURCE_DIR}/src/support/util/BRUtilMath.h
                ${PROJECT_SOURCE_DIR}/src/support/util/BRUtilMathParse.c
                ${PROJECT_SOURCE_DIR}/src/support/util/BRHex.c
                ${PROJECT_SOURCE_DIR}/src/support/util/BRHex.h)

# Support Tests
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_sources (corecryptotest
                    PRIVATE
                    ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/support/testSup.c)
endif(CMAKE_BUILD_TYPE MATCHES Debug)

# Bitcoin
target_sources (corecrypto
                PRIVATE
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRBitcoinBloomFilter.c
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRBitcoinBloomFilter.h
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRBitcoinChainParams.h
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRBitcoinChainParams.c
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRBitcoinMerkleBlock.c
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRBitcoinMerkleBlock.h
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRBitcoinPaymentProtocol.c
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRBitcoinPaymentProtocol.h
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRBitcoinPeer.c
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRBitcoinPeer.h
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRBitcoinPeerManager.c
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRBitcoinPeerManager.h
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRBitcoinTransaction.c
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRBitcoinTransaction.h
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRBitcoinWallet.c
                ${PROJECT_SOURCE_DIR}/src/bitcoin/BRBitcoinWallet.h)

# Bitcoin Tests
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_sources (corecryptotest
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
# BSV
target_sources (corecrypto
                PRIVATE
                ${PROJECT_SOURCE_DIR}/src/bsv/BRBSVParams.h
                ${PROJECT_SOURCE_DIR}/src/bsv/BRBSVParams.c)
                
# Litecoin
target_sources (corecrypto
                PRIVATE
                ${PROJECT_SOURCE_DIR}/src/litecoin/BRLitecoinParams.h
                ${PROJECT_SOURCE_DIR}/src/litecoin/BRLitecoinParams.c)

# Dogecoin
target_sources (corecrypto
                PRIVATE
                ${PROJECT_SOURCE_DIR}/src/dogecoin/BRDogecoinParams.h
                ${PROJECT_SOURCE_DIR}/src/dogecoin/BRDogecoinParams.c)
                
# Ethereum
target_sources (corecrypto
                PRIVATE
                # Base
                ${PROJECT_SOURCE_DIR}/src/ethereum/base/BREthereumAddress.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/base/BREthereumAddress.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/base/BREthereumBase.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/base/BREthereumData.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/base/BREthereumData.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/base/BREthereumEther.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/base/BREthereumEther.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/base/BREthereumFeeBasis.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/base/BREthereumFeeBasis.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/base/BREthereumGas.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/base/BREthereumGas.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/base/BREthereumHash.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/base/BREthereumHash.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/base/BREthereumLogic.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/base/BREthereumSignature.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/base/BREthereumSignature.h

                # BCS
                # ${PROJECT_SOURCE_DIR}/src/ethereum/bcs/BREthereumBCS.c
                # ${PROJECT_SOURCE_DIR}/src/ethereum/bcs/BREthereumBCS.h
                # ${PROJECT_SOURCE_DIR}/src/ethereum/bcs/BREthereumBCSEvent.c
                # ${PROJECT_SOURCE_DIR}/src/ethereum/bcs/BREthereumBCSPrivate.h
                # ${PROJECT_SOURCE_DIR}/src/ethereum/bcs/BREthereumBCSSync.c
                # ${PROJECT_SOURCE_DIR}/src/ethereum/bcs/BREthereumBlockChainSlice.h

                # Block Chain
                ${PROJECT_SOURCE_DIR}/src/ethereum/blockchain/BREthereumAccount.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/blockchain/BREthereumAccount.h
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
                ${PROJECT_SOURCE_DIR}/src/ethereum/blockchain/BREthereumProofOfWork.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/blockchain/BREthereumTransaction.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/blockchain/BREthereumTransaction.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/blockchain/BREthereumTransactionReceipt.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/blockchain/BREthereumTransactionReceipt.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/blockchain/BREthereumTransactionStatus.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/blockchain/BREthereumTransactionStatus.h

                # Contract
                ${PROJECT_SOURCE_DIR}/src/ethereum/contract/BREthereumContract.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/contract/BREthereumContract.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/contract/BREthereumExchange.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/contract/BREthereumExchange.h
                ${PROJECT_SOURCE_DIR}/src/ethereum/contract/BREthereumToken.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/contract/BREthereumToken.h

                # LES Msg
                # ${PROJECT_SOURCE_DIR}/src/ethereum/les/msg/BREthereumMessageDIS.c
                # ${PROJECT_SOURCE_DIR}/src/ethereum/les/msg/BREthereumMessageDIS.h
                # ${PROJECT_SOURCE_DIR}/src/ethereum/les/msg/BREthereumMessageETH.h
                # ${PROJECT_SOURCE_DIR}/src/ethereum/les/msg/BREthereumMessageLES.c
                # ${PROJECT_SOURCE_DIR}/src/ethereum/les/msg/BREthereumMessageLES.h
                # ${PROJECT_SOURCE_DIR}/src/ethereum/les/msg/BREthereumMessageP2P.c
                # ${PROJECT_SOURCE_DIR}/src/ethereum/les/msg/BREthereumMessageP2P.h
                # ${PROJECT_SOURCE_DIR}/src/ethereum/les/msg/BREthereumMessagePIP.c
                # ${PROJECT_SOURCE_DIR}/src/ethereum/les/msg/BREthereumMessagePIP.h

                # LES
                # ${PROJECT_SOURCE_DIR}/src/ethereum/les/BREthereumLES.c
                # ${PROJECT_SOURCE_DIR}/src/ethereum/les/BREthereumLES.h
                # ${PROJECT_SOURCE_DIR}/src/ethereum/les/BREthereumLESBase.h
                # ${PROJECT_SOURCE_DIR}/src/ethereum/les/BREthereumLESFrameCoder.c
                # ${PROJECT_SOURCE_DIR}/src/ethereum/les/BREthereumLESFrameCoder.h
                # ${PROJECT_SOURCE_DIR}/src/ethereum/les/BREthereumLESRandom.c
                # ${PROJECT_SOURCE_DIR}/src/ethereum/les/BREthereumLESRandom.h
                # ${PROJECT_SOURCE_DIR}/src/ethereum/les/BREthereumMessage.c
                # ${PROJECT_SOURCE_DIR}/src/ethereum/les/BREthereumMessage.h
                # ${PROJECT_SOURCE_DIR}/src/ethereum/les/BREthereumNode.c
                # ${PROJECT_SOURCE_DIR}/src/ethereum/les/BREthereumNode.h
                # ${PROJECT_SOURCE_DIR}/src/ethereum/les/BREthereumNodeEndpoint.c
                # ${PROJECT_SOURCE_DIR}/src/ethereum/les/BREthereumNodeEndpoint.h
                # ${PROJECT_SOURCE_DIR}/src/ethereum/les/BREthereumProvision.c
                # ${PROJECT_SOURCE_DIR}/src/ethereum/les/BREthereumProvision.h

                # MPT
                # ${PROJECT_SOURCE_DIR}/src/ethereum/mpt/BREthereumMPT.c
                # ${PROJECT_SOURCE_DIR}/src/ethereum/mpt/BREthereumMPT.h
		
                # Util
                ${PROJECT_SOURCE_DIR}/src/ethereum/util/BRKeccak.c
                ${PROJECT_SOURCE_DIR}/src/ethereum/util/BRKeccak.h

                )

# Ethereum Tests
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_sources (corecryptotest
                    PRIVATE
                    ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/ethereum/testBase.c
                    ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/ethereum/testBc.c
                    ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/ethereum/testContract.c
                    ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/ethereum/testEvent.c
                    ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/ethereum/testEwm.c
                    ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/ethereum/testRlp.c
                    ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/ethereum/testUtil.c
                    ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/ethereum/test.c
                    ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/include/test.h)

    target_include_directories(corecryptotest
            PRIVATE
            ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/include)
endif(CMAKE_BUILD_TYPE MATCHES Debug)

# Ripple
target_sources (corecrypto
                PRIVATE
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRipple.h
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleAccount.c
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleAccount.h
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleAddress.c
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleAddress.h
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleBase.h
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleFeeBasis.c
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleFeeBasis.h
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRipplePrivateStructs.h
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleSerialize.c
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleSerialize.h
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleSignature.c
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleSignature.h
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleTransaction.c
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleTransaction.h
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleUtils.c
                ${PROJECT_SOURCE_DIR}/src/ripple/BRRippleUtils.h)

# Ripple Tests
if (CMAKE_BUILD_TYPE MATCHES Debug)
   target_sources (corecryptotest
                   PRIVATE
                   ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/ripple/testRipple.c
                   ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/ripple/testRippleTxList1.h
                   ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/ripple/testRippleTxList2.h)
endif(CMAKE_BUILD_TYPE MATCHES Debug)

# Hedera
target_sources (corecrypto
                PRIVATE
                ${PROJECT_SOURCE_DIR}/src/hedera/BRHedera.h
                ${PROJECT_SOURCE_DIR}/src/hedera/BRHederaAccount.c
                ${PROJECT_SOURCE_DIR}/src/hedera/BRHederaAccount.h
                ${PROJECT_SOURCE_DIR}/src/hedera/BRHederaAddress.c
                ${PROJECT_SOURCE_DIR}/src/hedera/BRHederaAddress.h
                ${PROJECT_SOURCE_DIR}/src/hedera/BRHederaBase.h
                ${PROJECT_SOURCE_DIR}/src/hedera/BRHederaCrypto.c
                ${PROJECT_SOURCE_DIR}/src/hedera/BRHederaCrypto.h
                ${PROJECT_SOURCE_DIR}/src/hedera/BRHederaFeeBasis.c
                ${PROJECT_SOURCE_DIR}/src/hedera/BRHederaFeeBasis.h
                ${PROJECT_SOURCE_DIR}/src/hedera/BRHederaSerialize.c
                ${PROJECT_SOURCE_DIR}/src/hedera/BRHederaSerialize.h
                ${PROJECT_SOURCE_DIR}/src/hedera/BRHederaTransaction.c
                ${PROJECT_SOURCE_DIR}/src/hedera/BRHederaTransaction.h)

target_sources (hederaproto
	        PRIVATE
                # Proto
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
   target_sources (corecryptotest
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
        ${PROJECT_SOURCE_DIR}/src/tezos/BRTezosTransfer.h)

# Tezos Tests
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_sources (corecryptotest
            PRIVATE
            ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/tezos/testTezos.c)
endif(CMAKE_BUILD_TYPE MATCHES Debug)

# Crypto
target_sources (corecrypto
                PRIVATE
                # Public Includes
                ${PROJECT_SOURCE_DIR}/include/WKAccount.h
                ${PROJECT_SOURCE_DIR}/include/WKAddress.h
                ${PROJECT_SOURCE_DIR}/include/WKAmount.h
                ${PROJECT_SOURCE_DIR}/include/WKBase.h
                ${PROJECT_SOURCE_DIR}/include/WKCipher.h
                ${PROJECT_SOURCE_DIR}/include/WKClient.h
                ${PROJECT_SOURCE_DIR}/include/WKCoder.h
                ${PROJECT_SOURCE_DIR}/include/WKCurrency.h
                ${PROJECT_SOURCE_DIR}/include/WKExportablePaperWallet.h
                ${PROJECT_SOURCE_DIR}/include/WKFeeBasis.h
                ${PROJECT_SOURCE_DIR}/include/WKHash.h
                ${PROJECT_SOURCE_DIR}/include/WKHasher.h
                ${PROJECT_SOURCE_DIR}/include/WKKey.h
                ${PROJECT_SOURCE_DIR}/include/WKListener.h
                ${PROJECT_SOURCE_DIR}/include/WKNetwork.h
                ${PROJECT_SOURCE_DIR}/include/WKPayment.h
                ${PROJECT_SOURCE_DIR}/include/WKPeer.h
                ${PROJECT_SOURCE_DIR}/include/WKSigner.h
                ${PROJECT_SOURCE_DIR}/include/WKSync.h
                ${PROJECT_SOURCE_DIR}/include/WKSystem.h
                ${PROJECT_SOURCE_DIR}/include/WKTransfer.h
                ${PROJECT_SOURCE_DIR}/include/WKUnit.h
                ${PROJECT_SOURCE_DIR}/include/WKWallet.h
                ${PROJECT_SOURCE_DIR}/include/WKWalletManager.h
                ${PROJECT_SOURCE_DIR}/include/WKWalletSweeper.h

                # Sources
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKAccountP.h
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKAccount.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKAddressP.h
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKAddress.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKAmountP.h
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKAmount.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKBaseP.h
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKBase.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKCipher.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKClientP.h
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKClient.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKCoder.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKConfig.h
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKCurrency.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKExportablePaperWalletP.h
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKExportablePaperWallet.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKFeeBasisP.h
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKFeeBasis.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKFileService.h
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKFileService.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKHandlersP.h
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKHandlers.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/WKHandlersExport.h
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKHashP.h
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKHash.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKHasher.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKKeyP.h
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKKey.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKListenerP.h
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKListener.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKNetworkP.h
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKNetwork.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKPaymentP.h
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKPayment.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKPeer.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKSigner.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKSystemP.h
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKSystem.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKTransferP.h
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKTransfer.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKUnit.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKWalletP.h
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKWallet.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKWalletManagerP.h
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKWalletManager.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKWalletSweeperP.h
                ${PROJECT_SOURCE_DIR}/src/walletkit/WKWalletSweeper.c

                ${PROJECT_SOURCE_DIR}/src/version/WKVersion.c)

# Crypto BTC
target_sources (corecrypto
                PRIVATE
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/btc/WKAddressBTC.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/btc/WKBTC.h
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/btc/WKFeeBasisBTC.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/btc/WKNetworkBTC.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/btc/WKPaymentBTC.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/btc/WKSupportBTC.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/btc/WKTransferBTC.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/btc/WKWalletBTC.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/btc/WKWalletManagerBTC.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/btc/WKWalletManagerP2PBTC.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/btc/WKWalletManagerPersistBTC.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/btc/WKWalletSweeperBTC.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/btc/WKExportablePaperWalletBTC.c
                )

# Crypto ETH
target_sources (corecrypto
                PRIVATE
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/eth/WKAddressETH.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/eth/WKETH.h
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/eth/WKFeeBasisETH.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/eth/WKNetworkETH.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/eth/WKSupportETH.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/eth/WKTransferETH.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/eth/WKWalletETH.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/eth/WKWalletManagerETH.c
                )

# Crypto XRP
target_sources (corecrypto
                PRIVATE
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/xrp/WKAddressXRP.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/xrp/WKFeeBasisXRP.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/xrp/WKNetworkXRP.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/xrp/WKSupportXRP.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/xrp/WKTransferXRP.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/xrp/WKWalletXRP.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/xrp/WKWalletManagerXRP.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/xrp/WKXRP.h
                )

# Crypto HBAR
target_sources (corecrypto
                PRIVATE
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/hbar/WKAddressHBAR.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/hbar/WKFeeBasisHBAR.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/hbar/WKHBAR.h
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/hbar/WKNetworkHBAR.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/hbar/WKSupportHBAR.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/hbar/WKTransferHBAR.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/hbar/WKWalletHBAR.c
                ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/hbar/WKWalletManagerHBAR.c
                )

# Crypto XTZ
target_sources (corecrypto
        PRIVATE
        ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/xtz/WKAddressXTZ.c
        ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/xtz/WKFeeBasisXTZ.c
        ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/xtz/WKNetworkXTZ.c
        ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/xtz/WKSupportXTZ.c
        ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/xtz/WKTransferXTZ.c
        ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/xtz/WKWalletXTZ.c
        ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/xtz/WKWalletManagerXTZ.c
        ${PROJECT_SOURCE_DIR}/src/walletkit/handlers/xtz/WKXTZ.h
        )

# Crypto Tests
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_sources (corecryptotest
                    PRIVATE
                    ${PROJECT_SOURCE_DIR}/WalletKitCoreTests/test/crypto/testCrypto.c)
endif(CMAKE_BUILD_TYPE MATCHES Debug)


# Link in the static sqlite3 library
target_link_libraries (corecrypto
                       sqlite3
                       ed25519
                       blake2
		       hederaproto)

if (CMAKE_BUILD_TYPE MATCHES Debug)
   target_link_libraries (corecrypto
   			  corecryptotest)
endif(CMAKE_BUILD_TYPE MATCHES Debug)

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
