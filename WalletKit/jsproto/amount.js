class Amount {
  /* private */ constructor (core) {
    this.core = core
  }

  get unit() {
  }

  get currency() {
  }

  get isNegative() {
  }

  func asDoubleIn (unit) {
  }

  func asStringIn (unit, formatter) {
  }

  // string(base, preface)

  func isCompatibleWith (amount) {
  }

  func hasCurrency (currency) {
  }

  func add (that) {
  }

  func sub (that) {
  }

  func convertTo (unit) {
  }

  get negate() {
  }

  get isZero() {
  }

  static createFromDoule (value, unit) {
  }

  static createFromInteger (value, unit) {
  }

  static createFromString (value, negative /* false */, unit) {
  }

  // +/-
  // ==/!=/</...

  get description() {
  }
