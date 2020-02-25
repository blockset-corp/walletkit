var CoreJS = require ("./libcorecrypto.js")

var BRCryptoAccount = {
  "cryptoAccountGeneratePaperKey":  CoreJS.cwrap('cryptoAccountGeneratePaperKey', 'int', ['int-array']),
  // ...
};


/*
var ref = require('ref');
var ffi = require('ffi');

var BRCryptoAccount = ref.types.void

var uint8Ptr = ref.refType(ref.types.uint8)

const cryptoAccountDeclarations = {
  'cryptoAccountGeneratePaperKey':        [ 'string', [ /* char *wordList[] */ ]],
  'cryptoAccountValidatePaperKey':        [ '', []],
  // validatecryptoAccountValidateWordsList
  'cryptoAccountCreate':                  [ 'BRCryptoAccount', [ 'string', 'uint64', 'string' ]],

  // ...
  'create_from_serialization': [ 'BRCryptoAccount', [ uint8Ptr, ref.types.size, 'string']],
  'serialize':                 [ 'BRCryptoBoolean', [ BRCryptoAccount, uint8Ptr, ref.types.size ]],
  'validate_serialization':    [],
  'get_timestamp':             [],
  'get_file_system_identifier':[],
  'get_uids':                  [],
  'is_initialized':            [],
  'get_initialization_data':   [],
  'initialize':                []
};
*/
  
