import * as WalletKit from '../dist/WalletKit/index.js'

console.log('WalletKit: '); console.log(WalletKit);
//console.log('WalletKit.Core: '); console.log(WalletKit.Core);
console.log('WalletKit.Currency: '); console.log(WalletKit.Currency);
 
//WalletKit.Core['onRuntimeInitialized'] = function () {
 var btc = new WalletKit.Currency (1); //'btc');
  console.log(btc)

  var btc_satoshi = new WalletKit.Unit ('satoshi');
  console.log(btc_satoshi)

  var btc_too = WalletKit.Currency.create ('btc uids', 'Bitcoin', 'btc', 'btc type', undefined);
  console.log(btc_too.uids);
  console.log(btc_too.code);
  console.log(btc_too.name);

  console.log(btc)
//}
