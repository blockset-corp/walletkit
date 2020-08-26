class System {
  constructor (listener, account, onMainnet, path, query, queue) {
  }
  
  get account() {
  }

  get path() {
  }

  get onMainnet() {
  }

  get query() {
  }

  // queue

  get networks() {
  }

  // networkIsReachable

  get managers() {
  }

  // internal callbackCoordinator

  // internal networkAdd (network)
  // internal networkByUids (uids)
  // internal managerByCore (core)

  // internal managerAdd (manager)

  createManager (network, mode, addressScheme, currencies) {
  }

  // wipe filesystem

  get wallets() {
  }

  // subscribe
  // subscribeAnnounceTransaction

  updateNetworkFees (onCompletion) {
  }

  disconnectAll() {
  }

  connectAll() {
  }

  get networkReachable() {
  }

  set networkReachable(reachable) {
  }

  configure (currencyModels) {
  }


  // Static System Table
  // queue
  // index
  // mapping
  // lookup
  // remove
  // extend
  // extract
  // destroy

  // wipe
  // wipeAll
}

// SystemEvent

// SystemListener
// SystemCallbackCoordinator

// cryptoListener
//   managerEventCallback
//   walletEventCallback
//   transferEventCallback

// cryptoClient
//   getBalance
//   getBlockNumber
//   getTransactions
//   getTransfers
//   submitTransfer
//   ... ETH
