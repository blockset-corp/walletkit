class Network {
  /* private */ constructor (core) {
    this.core = core
  }

  get /* internal */ uids() {
  }

  get name() {
  }

  get isMainnet() {
  }

  get type() {
  }

  get height() {
  }

  get fees() {
  }

  get minimumFee() {
  }

  get confirmationsUntilFinal() {
  }

  // createPeer

  get defaultAddressScheme() {
  }

  get supportedAddressSchemes() {
  }

  func supportsAddressScheme (scheme) {
  }

  get defaultMode() {
  }

  get supportedModes() {
  }

  get supportsMode (mode) {
  }

  // Native 
  get currency() {
  }

  // All
  get currencies() {
  }

  currencyByCode (core) {
  }

  currencyByIssuer (issuer) {
  }

  hasCurrency (currency) {
  }

  baseUnitFor (currency) {
  }

  defaultUnitFor (currency) {
  }

  unitsFor (currency) {
  }

  hasUnitFor (currency, unit) {
  }

  // internal addCurrency (currency, baseUnit, defaultUnit)
  // internal addUnitFor (currency, unit)

  static findBuiltin (uids) {
  }

  static installBuiltins () {
  }

  get description() {
  }

  // equality
  // hashable
}

const NetworkType = {
  btc:    0, // CRYPTO_NETWORK_TYPE_BTC
  bch:    1,
  eth:    2,
  xrp:    3,
  hbar:   4
}

// NetworkEvent

class NetworkFee {
  // ...
}

class NetworkPeer {
  // ...
}
