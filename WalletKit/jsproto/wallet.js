class Wallet {
  /* private */ constructor (core) {
    this.core = core
  }

  get manager() {
  }

  get system() {
  }

  get unit() {
  }

  get currency() {
  }

  get name() {
  }

  get balance() {
  }

  get balanceMaximum() {
  }

  get balanceMinimum() {
  }

  get state() {
  }

  get target() {
  }

  targetForScheme (scheme) {
  }

  hasAddress (address) {
  }

  // attributes

  get transfers() {
  }

  transferByHash (hash) {
  }

  // internal transferByCore
  //          transerByCoreOrCreate

  createTransfer (target, amount, estimatedFeeBasis, attributes) {
  }

  // createTransfer ...

  estimateLimitMaximum (target, fee, onCompletion) {
  }

  estimateLimitMinimum (target, fee, onCompletion) {
  }

  // LimitEstimationError

  estimateFee (target, amount, networkFee, onCompletion) {
  }

  // estimateFee ...

  // FeeEstimationError
  
}

const WalletState = {
  created:  0, // CRYPTO_WALLET_STATE_CREATED
  deleted:  1
}

// WalletEvent
