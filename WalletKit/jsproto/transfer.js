class Transfer {
  /* private */ constructore (core) {
    this.core = core
  }

  get wallet() {
  }

  get manager() {
  }

  get system() {
  }

  get unit() {
  }

  get unitForFee() {
  }

  get source() {
  }

  get target() {
  }

  get amount() {
  }

  get amountDirected() {
  }

  get estimatedFeeBasis() {
  }

  get confirmedFeeBasis() {
  }

  get fee() {
  }

  get confirmation() {
  }

  get confirmations() {
  }
  
  confirmationsAt (blockHeight) {
  }

  get hash() {
  }

  get state() {
  }

  get direction() {
  }

  // attributes

  // identical
  // equality
}

const TransferDirection = {
  sent:      0, // CRYPTO_TRANSFER_SENT
  received:  1,
  recovered: 2
}

class TransferFeeBasis {
  // ...
}

class TransferConfirmation {
  constructor (blockNumber, transactionIndex, timestamp, fee) {
    // ...
  }
  
  get blockNumber() {
  }

  get transactionIndex() {
  }

  get timestamp() {
  }

  get fee() {
  }
}

class TransferHash {
  /* private */ constructor (core) {
    this.core = core
  }

  // ...
}

// TransferSubmitError

// TransferAttribute

// TransferState

// TransferEvent

class TransferOutput {
  // target Address
  // amount
}
