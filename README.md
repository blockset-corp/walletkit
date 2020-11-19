
![WalletKit](../gh-pages/public/img/logo-tight.png)

WalletKit provides a uniform wallet interface to access numerous crypto-currencies including 
Bitcoin and Etheruem.  WalletKit is implemented in the C progamming language and includes
a number of bindings for other languages, notably Swift and Java.  

WalletKit supports the following crypto-currencies: Bitcoin, Bitcoin Cash, Bitcoin SV, Ethereum,
Ethereum ERC20 'tokens', Ripple, Hedera and Tezos.  Other crypto-currencies are added
regularly.  Adding another blockchain is accomplished by satisfying a WalletKit-defined API.

WalletKit is the basis for the BRD iOS and Android mobile applications which currently have
combined downloads of over 6,000,000.

![badge-mit][]   ![badge-languages][]   ![badge-platforms][]  

# Features

## Crypto-Currency Agnostic

## Syncing Modes

## Event-Based

## Multiple Language Bindings 

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

A `BRCryptoNetwork` ...

## Currency

A `BRCryptoCurrency` ...

## Transfer

A `BRCryptoTransfer` ...

## Wallet

A `BRCryptoWallet` ...

## WalletManager

A `BRCryptoWalletManager` ...

# Installation and Use

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

### SwiftPM

### Maven

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

# Versions

## 1.0

Version 1.0 is currently the basis for the BRD iOS and Android mobile applications

# Support

Contact [Blockset](https://blockset.com "Blockset")



[badge-languages]: https://img.shields.io/badge/languages-C%20%7C%20Swift%20%7C%20Java-orange.svg
[badge-platforms]: https://img.shields.io/badge/platforms-iOS%20%7C%20Android%20%7C%20macOS%20%7C%20Linux-lightgrey.svg
[badge-mit]: https://img.shields.io/badge/license-MIT-blue.svg
