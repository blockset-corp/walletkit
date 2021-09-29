
![WalletKit](../gh-pages/public/img/logo-tight.png)

WalletKit provides a uniform wallet interface to access numerous crypto-currencies including 
Bitcoin and Ethereum.  WalletKit is implemented in the C progamming language and includes
a number of bindings for other languages, notably Swift and Java.  

WalletKit supports the following crypto-currencies: Bitcoin, Bitcoin Cash, Bitcoin SV, Ethereum,
Ethereum ERC20 'tokens', Ripple, Hedera and Tezos.  Other crypto-currencies are added
regularly.  Adding another blockchain is accomplished by satisfying a WalletKit-defined API.

WalletKit is the basis for the BRD iOS and Android mobile applications which currently have
combined downloads of over 6,000,000.

![badge-mit][]   ![badge-languages][]   ![badge-platforms][]  

# Features

## Cryptocurrency Agnostic

WalletKit treats the assets held in a wallet without regard to the crytpocurrency.  Thus Bitcoin,
Ethereuem and other wallets have a balance and a set of transfers contributing to that balance.
The wallet's balance will be in a currency defined for the cryptocurrency.

## Syncing Modes

WalletKit allows one to specify how a wallet's transfers are identified.  Some cryptocurrencies,
notably Bitcoin and related currencies, define peer-to-peer interfaces that can be used to 
identify transactions for a User's wallet.  (For Bitcoin, this peer-to-peer interface is known as
SPV - 'Simple Payment Verification').  WalletKit implements peer-to-peer interfaces for some
cyptocurrencies.  In addition, WalletKit defines an interface that can be used to identify 
transactions based on HTTP requests made to endpoints holding cryptocurrency blocks and 
transactions.  WalletKit provides a default implementatin of this 'client interface' using Blockset.

## Event-Based

WalletKit defines

## Multiple Language Bindings 

WalletKit is implemented in C and uses minimal OS resources.  Thus WalletKit will run on many
platforms.  With a basis in C, multiple other languages can invoke the C interfaces.

### C

WalletKit defines a C interface, with associated C implementation, that runs on both MacOS
and on Linux platforms.  The C interface can be accessed through the 'foriegn function interface'
offered by other languages, such as by Swift and Java.

The C interface is located in `.../WalletKit/WalletKitCore/include`.

### Swift

WalletKit includes a Swift framework, called `WalletKit` layered on the C code.

### Java

WalletKit includes a Java library ...

### Others

WalletKit is being extended with additional language bindings for Kotlin, Python, node.js and 
web assembly.

## Demo Mobile Applications

WalletKit includes iOS and Android demo applications.  These applications illustrate 
basic usage of WalletKit to connect to numerous blockchains and to manage numerous wallets
holding assets based on blockchain transactions.

The iOS demo application is accessed using Xcode; the Andriod demo application is accessed
using Android Studio.

# Concepts

WalletKit is crypto-currency agnostic; as such, WalletKit defines a number of concepts that
apply across disparte blockchains but also can be extended with blockchain specific data and
behaviors.

## Network

A `WKNetwork` represents a cryptocurrency blockchain.

## Currency

A `WKCurrency` represents an asset on a particular network that can be held by a User
or transfers between Users.

## Account

A `WKAccount`  holds the User's public information for an associated BIP-39 'paperKey'.  The
public information includes everything needed to interact with any of WalletKit's supported
blockchains.

`WalletKit` never holds a User's private information.  An account is created initially with a
BIP-39 'paperKey' and then, thereafter, the account is recreated from an account-specific
serializaiton of the account's public information.


## Transfer

A `WKTransfer` represents the exchange of an asset between two Users.  A transfer
abstracts over blockchain's transactions - a transaction potentially produces multiple Transfers.

## Wallet

A `WKWallet` holds the asset represented by a currency.  A wallet maintains a balance
and a time-ordered list of transfers.

## WalletManager

A `WKWalletManager` manages one or more wallets associated with a specific network.
For example, the Ethereum blockchain defines ERC20 tokens, a type of asset.  The Ethereum
wallet manager can manage a wallet holding Ether as well as any number of ERC20-based 
assets.

## System

A `WKSystem` provides the primary interface to WalletKit.  A system manages one or more
Networks and one or more Wallet Manaers.

A `WKSystem` is configured with a  `WKListener` and a `WKClient`.  The listener implements a
defined interface with functions that receive events; the events announce WalletKit state
changes - such as a blockchain height has been updated and a wallet has a new transfer.  The
client also implements a defined interface that WalletKit uses to gather blockchain data from
and external, generally HTTP, source.  A default implementation of `WKClient` is provided that 
uses Blockset for blockchain data.

A `WKSystem` is created with a `WKAccount` and thus one system exists for each User.

A `WKSystem` is created with a `WKDatbaseClient` which is used to store the User's public
blockchain information - specifically transfers associated with the User's various wallets.  Two
types of database clients are provided for default use - one that stores no User information
which thus requires re-discovering transaction each time a system is created and one that
stores User information in a file-system based SQLite database.

# Installation

WalletKit is delivered as a Git repository for development within Xcode and Android Studio.

## Access

### Git

Clone WalletKit with 
```
git clone --recurse-submodules git@github.com:blockset-corp/walletkit.git WalletKit
```
If you've cloned WalletKit but without the `--recursive-submodules` flag then perform:
```
(cd .../WalletKit; git submodule update --init --recursive)
```
Otherwise a compilation error will arise when a secp256k1 header file is not found.

### SwiftPM

...

### Maven

...

## Setup

WalletKit must be configured to support the demo applications and the units tests.  Setup is
required whether one chooses Xcode or Android Studio for development.  Setup entails 
ensuring that a file, typically called `WalletKitTestsConfig.json`, exists and can be built
into the applications and unit tests.  That file has the following structure:

```
{
  "blockset": {
    "baseURL": "https://api.blockset.com",
    "token":   "<blockset access token>"
  },

  "accounts": [
    {
      "identifier": "ginger",
      "paperKey":   "ginger settle marine tissue robot crane night number ramp coast roast critic",
      "timestamp":  "2018-01-01",
      "network":    "testnet",
      "content":    "BTC, BCH, ETH"
    },

    {
      "identifier": "tape",
      "paperKey":   "tape argue fetch truck cattle quiz wide equal inform rabbit ranch educate",
      "timestamp":  "2018-04-02",
      "network":    "testnet",
      "content":    "(NO) BTC"
    },
    
    ...
    }
```

The structure is as follows:

* The `"blockset"` dictionary detemines how the `FastSync` capability is accomplisted with 
a connection to [Blockset](https://blockset.com "Blockset").  Use of `FastSync` requires a 
"blockset client access token", which can be obtained from
[Blockset Docs, Client Authentication](https://docs.blockset.com/getting-started/authentication#authentication-walkthrough "Blockset Docs, Client Authentication").

* The `"accounts"` array defines 'paperKeys' to use when creating an 'account' for connecting 
to various blockchains.  You can add your own 'paperKeys', including ones for mainnet.  It is a 
good idea to get the `"timestamp"` correct so that searching for transactions (aka 'syncing') 
has a reasonable starting time.

To specify a `WalletKitTestsConfig.json` file do the following:

* Copy the file `.../WalletKit/templates/WalletKitTestsConfig_example.json` into 
a location in your file system, generally outside of the WalletKit cloned directory.  For example,
`${HOME}/.walletKitTestsConfig.json` is a common location.  If you edit the file and add 
your own paperKeys then you will want to ensure that those changes are kept independent of 
the WalletKit directory.

* Copy the file `.../WalletKit/templates/WalletKitTestsConfig.env` to
`.../WalletKit/WalletKitTestsConfig.env` and then edit that file by changing the
assignment of `WALLET_KIT_TESTS_CONFIG_FILE` to point to your config file.

With the `.env` file specified, the WalletKit build process will copy the `.json` file into the
iOS and Android mobile apps as a resource and into locations accessible by the unit tests.

## Building

### Swift Package Manager

The WalletKit Swift framework can be built with `swift build`; the unit tests can be run with
`swift test`.  This will work on MacOS and on Linux operating systems.  The 
`swift-tools-version` must be 5.3 or greater (see 
`.../WalletKit/WalletKitSwift/Package.swift`)

### Gradle

The WalletKit Java library can be built with ...

### Xcode

WalletKit can be started in Xcode using `open .../WalletKit/WalletKitSwift/WalletKitDemo/WalletKitDemo.xcworkspace`.
This defines a workspace that allows one to access the Swift Demo App, the `WalletKit` Swift
code and the `WalletKitCore` C code.

### Android Studio

WalletKit can be started in Android Studio using ...

### Command Line

The `cmake` file 'WalletKitCore/CMakeLists.txt' can be used to build all of the C code into  
`libWalletKitCore.so` which can then be linked into an exectuable.

# Framework Use

`WalletKit` is delivered as a framework and is designed to be embedded in another 
application.  The `WalletKit` interfaces are implmented in each of `C`, `Swift` and `Java`.  All
the interfaces are analogous to one other with differences owing to idiomatic usage for that
language.

## Concepts

The following describes the `C` interfaces, upon which the `Swift` and `Java` interfaces are built.

### WKSystem

Use of the framework starts with the instantiation of `WKSystem`.  The top-level 
interfaces is:

```
extern WKSystem
wkSystemCreate (WKClient client,
                WKListener listener,
                WKAccount account,
                const char *path,
                WKBoolean onMainnet);
```
Given a system, one invokes `wkSystemStart()` to begin receiving events on `listener` and
then `wkSystemConnect()` to begin processing blockchain transactions on `client`.  If 
internet  connection is lost one calls `wkSystemDisconnect()` and then one connects again
when internet connectivity is restorted.

Beyond this, there are various `WKSystem` accessor functions.

The embedding applications primary interaction with a system comes from the `WKListener`.  
As listener callbacks (aka events) occur, the application can create wallet managers of interest,
update the UI with the blockchains current height, display wallet balances and transfers.

### WKListener

A `WKListener` is created with:
```
extern WKListener
wkListenerCreate (WKListenerContext context,
                  WKListenerSystemCallback systemCallback,
                  WKListenerNetworkCallback networkCallback,
                  WKListenerWalletManagerCallback managerCallback,
                  WKListenerWalletCallback walletCallback,
                  WKListenerTransferCallback transferCallback);
```
where the `context` allows the application to reestablish its own state within each listener
callback.  The callbacks are functions; `WKListenerSystemCallback` is:
```
typedef void (*WKListenerSystemCallback) (WKListenerContext context,
                                          WKSystem system,
                                          WKSystemEvent event);
```
where the `WKSysemEvent` types are:
```
typedef enum {
    WK_SYSTEM_EVENT_CREATED,
    WK_SYSTEM_EVENT_CHANGED,
    WK_SYSTEM_EVENT_DELETED,

    WK_SYSTEM_EVENT_NETWORK_ADDED,
    WK_SYSTEM_EVENT_NETWORK_CHANGED,
    WK_SYSTEM_EVENT_NETWORK_DELETED,

    WK_SYSTEM_EVENT_MANAGER_ADDED,
    WK_SYSTEM_EVENT_MANAGER_CHANGED,
    WK_SYSTEM_EVENT_MANAGER_DELETED,

    WK_SYSTEM_EVENT_DISCOVERED_NETWORKS,
} WKSystemEventType;
```
These events detail the system changes in state.  The listener takes actions appropriate for the
application.  Notably, on `NETWORK_ADDED` or  `DISCOVERED_NETWORKS` the application can
create a `WKWalletManager` for each network of interest.  One might be interested in BTC and
ETH network types, but not BCH for example.

As the networks are announced and wallet managers of interest are created, then their 
associated callbacks occur, including adding wallets to wallet manager and adding transfers to
wallets.  The `WKListener` instance handles each event with an update to application state,
typically the UI.

### WKClient

A `WKClient` is created as a value-type instance of:
```
typedef struct {
    WKClientContext context;
    WKClientGetBlockNumberCallback  funcGetBlockNumber;
    WKClientGetTransactionsCallback funcGetTransactions;
    WKClientGetTransfersCallback funcGetTransfers;
    WKClientSubmitTransactionCallback funcSubmitTransaction;
    WKClientEstimateTransactionFeeCallback funcEstimateTransactionFee;
} WKClient;
```
where the `context` allows the application to reestablish its own state within each client
callback.  The callbacks are functions; `WKClientGetTransactionsCallback` is:
```
typedef void
(*WKClientGetTransactionsCallback) (WKClientContext context,
                                    OwnershipGiven WKWalletManager manager,
                                    OwnershipGiven WKClientCallbackState callbackState,
                                    OwnershipKept const char **addresses,
                                    size_t addressCount,
                                    WKBlockNumber begBlockNumber,
                                    WKBlockNumber endBlockNumber);
```
This callback queries the manager's network for transactions in blocks `[begBlockNumber,
endBlockNumber)` that involve any of `addressCount` addresses in the array `addresses`.
Once all the transactions have been identified, the application invokes
```
extern void
wkClientAnnounceTransactions (OwnershipKept WKWalletManager cwm,
                              OwnershipGiven WKClientCallbackState callbackState,
                              WKBoolean success,
                              WKClientTransactionBundle *bundles,
                              size_t bundlesCount);
```
with one bundle created for each transaction using:
```
extern WKClientTransactionBundle
wkClientTransactionBundleCreate (WKTransferStateType status,
                                 OwnershipKept uint8_t *transaction,
                                 size_t transactionLength,
                                 WKTimestamp timestamp,
                                 WKBlockNumber blockHeight);
```
Other callbacks have similar forms: a) perform a query to get the objects of interest, b) create
a specified 'bundle' for each object, c) and then 'announce' all the bundles.

A default `WKClient`, based on `Blockset` is implemented in the Swift and Java code - where
convenient HTTP related functions are accessible for iOS, Android and Linux platforms.

### WKAccount

A `WKAccount` is created in one of two ways.  For the very first time, an account is created with:
```
extern WKAccount
wkAccountCreate (const char *paperKey, WKTimestamp timestamp, const char *uids);
```
where `paperKey` is a BIP-39 sequence of 12 words, `timestamp` is the Unix time at which the
`paperKey` was initially created (or the time of the User's first transaction on any blockchain), 
and `uids` is a globally unique identifier for the account.  The `uids` is not used internally by
`WalletKit`.

Once an account is created and to avoid repeated use of the sensitive, private `paperKey`, one
creates a serialization of the account with `wkAccountSerialize()` and then subsequently 
re-creates the account with:
```
extern WKAccount
wkAccountCreateFromSerialization (const uint8_t *bytes, size_t bytesCount, const char *uids);
```

The BIP-39 `paperKey` is thus only used in two cases: 1) to create the account initially and 2)
to sign transactions.

### WKDatabaseClient

TBD

## Example

WalletKit is designed to be include in Swift, Java or C executables.  An example is provided for 
each.

The high-level process is to create a `System` by providing:
    - Client - implements functions designed to query for a User's transactions.  The Swift and
    Java implementations provide a default client that uses Blockset
    - Listener - implements functions, as callbacks, that announce changes to WalletKit state such
    as the addition of a `Transfer` to a `Wallet`.
    - Account - provides the public key information needed to derive a User's addresses.  The 
    private key is only needed when signing transactions for submission to a blockchain.
    
Given a `System` one then starts and connects it. 

Importantly, one of the early events provided by a `System`, which gets announced through the
`Listener` interface, is the `WK_SYSTEM_EVENT_NETWORK_ADDED` event.  All networks, aka
blockchains such as BTC, BCH, DOGE, ETH, XRP, etc, supported by WalletKit get announced.
If your application is interested in the announced network, your application must call 
`wkSystemCreateWalletManager()`.  The created wallet manager is responsible for managing
the currencies on that network.

### Swift

### Java

### C

# Versions

## 0.10.0

Version 0.10.0 is currently the basis for the BRD iOS and Android mobile applications.  Note
that `walletkit-0.10.x` has evolved from `core-9.x.x`

# Support

Contact [Blockset](https://blockset.com "Blockset")

[badge-languages]: https://img.shields.io/badge/languages-C%20%7C%20Swift%20%7C%20Java-orange.svg
[badge-platforms]: https://img.shields.io/badge/platforms-iOS%20%7C%20Android%20%7C%20macOS%20%7C%20Linux-lightgrey.svg
[badge-mit]: https://img.shields.io/badge/license-MIT-blue.svg
