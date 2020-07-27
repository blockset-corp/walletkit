class WalletManager {
  /* private */ constructor (core) {
    this.core = core
  }

  get system() {
  }

  get account() {
  }

  get network() {
  }

  // primaryWallet currency
  get currency() {
  }

  get name() {
  }

  get baseUnit() {
  }

  get defaultUnit() {
  }

  get isActive() {
  }
  
  get query() {
  }

  get unit() {
  }

  get mode() {
  }

  get path() {
  }

  get state() {
  }

  get height() {
  }

  get primaryWallet() {
  }

  registerWalletFor (currency) {
  }

  get wallets() {
  }

  // walletByCore
  // walletByCoreOrCreate


  get defaultNetworkFee() {
  }

  get addressScheme() {
  }

  set addressScheme(scheme) {
  }

  connect() {
  }

  connectUsingPeer (peer) {
  }

  disconnect() {
  }

  stop() {
  }

  sync() {
  }

  syncToDepth (depth) {
  }

  signTransfer (transfer, paperKey) {
  }

  submitTranfer (transfer, paperKey) {
  }

  get networkReachable() {
  }

  set networkReachable(reachable) {
  }

  // sweep

  get description() {
  }

  
}

const WalletManagerMode = {
  api_only:             0,  // CRYPTO_SYNC_MODE_API_ONLY
  api_with_p2p_submit:  1,
  p2p_with_api_send:    2,
  p2p_only:             3

  // serialize
}

// WalletManagerState

// WalletManagerSyncDepth

// WalletMangerSyncStoppedReason

// WalletManagerDisconnectReason

// WalletManagerEvent

// WalletSweeper
// WalletSweeperError



