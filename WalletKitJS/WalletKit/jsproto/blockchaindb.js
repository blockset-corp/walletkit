class BlockchainDB {

  constructor (/* session */ bdbBaseURL, /* func */ apiBaseURL /* func */) {
  }
  
  get bdbBaseURL() {
  }

  get apiBaseURL() {
  }

  // session
  // queue

  // QueryError

  // Model
  //   Blockchain
  //   CurrencyDenomination
  //   Currency
  //   Transfer
  //   Transaction
  //   Block
  //   Subscription

  getBlockchains (mainnet, onCompletion) {
  }

  getBlockchain (blockchainID, onCompletion) {
  }

  getCurrencies (blockchainID, onCompletion) {
  }

  getCurrency (currencyID, onCompletion) {
  }

  // subscriptions

  getTransfers (blockchainID, address,
		begBlockNumber, endBlockNumber, maxPageSize,
		onCompletion) {
  }

  getTransfer (transferID, onCompletion) {
  }

  getTransactions (blockchainID,
		   address,
		   begBlockNumber,
		   endBlockNumber,
		   includeRaw,
		   includeProof,
		   maxPageSize,
		   onCompletion) {
  }

  getTransaction (transactionID,
		  includeRaw,
		  includeProof,
		  onCompletion) {
  }

  createTransaction (blockchainID,
		     hash, /* HexString */
		     transaction, /* Data */
		     onCompletion) {
  }

  // getBlocks
  // getBlock

  // Ethereum

  // JSON
  // sendRequest
  // decorateRequest
  // makeRequest
  // bdb
  //   handleResult
  //   makeRequest

  // ...
}
