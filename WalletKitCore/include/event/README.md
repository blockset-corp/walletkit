
Contains low-level Transfer, Wallet, WalletManager, Network and System declarations that 
are shared between 1) the interfaces and implementations of those concepts and 
2) BRCryptoListener

BRCryptoListener is a high-level concept used by Swift/Java/other interfaces and Apps
to provide functions that will handle WalletKit events.

We want the implemenations of Transfer, Wallet, etc to 'signal their own events by 
invoking a BRCryptoListener function.

By splitting low-level declarations into an 'events/' module, we avoid circularities among
header files.

