class Address {
  /* private */ constructor (core) {
    this.core = core
  }

  get description () {
  }

  static createFromString (string, network) {
  }

  // equality
}

// AddressScheme
case AddressScheme = {
  btcLegacy:     0, // CRYPTO_ADDRESS_SCHEME_BTC_LEGACY
  btcSegwit:     1,
  ethDefault:    2,
  genDefault:    3
}
