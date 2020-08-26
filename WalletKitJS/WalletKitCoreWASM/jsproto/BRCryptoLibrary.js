var ref = require('ref');
var ffi = require('ffi');

var Core = ffi.Library('libCore', {
  ...cryptoCurrencyDeclarations,
  ...cryptoUnitDeclarations,
  ...cryptoAccountDeclarations,
})
