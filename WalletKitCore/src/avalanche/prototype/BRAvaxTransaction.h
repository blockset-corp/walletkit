//
//  Header.h
//  
//
//  Created by Amit on 28/07/2021.
//

#ifndef BRAvaxTransaction_h
#define BRAvaxTransaction_h

#include "support/BRBase58.h"
#include "support/util/BRHex.h"
#include "support/BRKey.h"
#include "support/BRSet.h"
#include "support/BRArray.h"
#include "BRAvalancheAddress.h"
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


//uint32_t
typedef enum {
  NETWORK_ID_MAINNET  = 1,
  NETWORK_ID_FUJI     = 5,
  NETWORK_ID_LOCAL    = 12345,
} network_id_t;

//unit32_t
typedef enum {
    SECP256K1TransferInput = 5, //input
} input_type;

typedef enum{
    SECP256K1TransferOutput = 7, //output
    SECP256K1OutputOwners= 11, //output
} output_type;

typedef enum{
    BaseTx = 0x0000,
}tx_type;


struct AddressRecord{
    uint8_t rmd160[20];
};

struct BRAssetRecord{
    char base58[50]; //null terminated asset id
    UInt256 id;
};

struct TxIdRecord{
    char base58[50]; //Base58 encoded string
    UInt256 id;
};


struct BRAvaxUtxoRecord{
    struct TxIdRecord tx;
    struct BRAssetRecord asset;
    uint32_t output_index;
    uint64_t amount;
    uint64_t locktime;
    BRArrayOf(struct AddressRecord) addresses;
};

struct SECP256K1TransferOutputRecord{
    uint64_t amount;
    uint64_t locktime;
    uint32_t threshold;
    BRArrayOf(struct AddressRecord) addresses;
};

//=====Unsupported Ouptput Types
struct SECP256K1MintOutputRecord{
    uint64_t locktime;
    uint32_t threshold;
    size_t addresses_len; //must be set to 0 if address is null
    BRAvalancheXAddress ** addresses;
};

struct NFTTransferOutputRecord{
    uint64_t locktime;
    uint64_t threshold;
    uint32_t group_id;
    size_t payload_len;
    uint8_t ** payload; //variable lenghh array
    size_t addresses_len; //must be set to 0 if address is null
    BRAvalancheXAddress ** addresses;
};
//=====End

struct TransferableOutputRecord{
    output_type type_id;
    struct BRAssetRecord asset; //32 bytes
    //void * output; //Ugly as we have to cast by type_id and manage memory seperately potentially :(
    union{
            struct SECP256K1TransferOutputRecord secp256k1;
            struct SECP256K1MintOutputRecord mint;
    }output;
};

struct SECP256K1TransferInputRecord{
    uint64_t amount;                   // 08 bytes
    size_t address_indices_len;
    uint32_t * address_indices; //variable array of 04 bytes
};

struct TransferableInputRecord{
    input_type type_id;
    struct TxIdRecord tx; //32 byte txid
    uint32_t utxo_index;
    struct BRAssetRecord asset; //32 byte
    union {
        struct SECP256K1TransferInputRecord secp256k1;
    } input;
 };

struct BaseTxRecord{
    uint16_t codec;
    tx_type type_id; // 4 bytes
    network_id_t network_id; //4 bytes
    char blockchain_id[32];//no null terminating char
    BRArrayOf( struct TransferableOutputRecord) outputs;
    BRArrayOf(struct TransferableInputRecord) inputs;
    char memo [256];//no null terminating char
};


extern void avaxCreateBaseTx();

extern struct BaseTxRecord
avaxTransactionCreate(const char* sourceAddress,
                      const char* targetAddress,
                      const char* changeAddress,
                      const char* assetId,
                      uint64_t amount,
                      BRArrayOf(struct BRAvaxUtxoRecord ) utxos,
                      const char * memo,
                      network_id_t network_id,
                      const char * cb58chainId);

extern void releaseTransaction(struct BaseTxRecord * tx);

extern void avaxSignBytes(BRKey * key, uint8_t * bytes, size_t len, uint8_t * sig65);

extern UInt256 avaxAssetDecodeAssetId(struct BRAssetRecord asset);

extern UInt256 avaxTxidDecodeBase58(struct TxIdRecord tx);

#ifdef __cplusplus
}
#endif

#endif /* BRAvaxTransaction_h */
