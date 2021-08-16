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
    SECP256K1TransferOutput = 0x00000007, //output
    SECP256K1OutputOwners= 0x0000000b, //output
    SECP256K1TransferInput = 0x00000005, //input
} type_id_t;

struct AddressRecord{
    char address[20];
};

struct AssetRecord{
    char asset_id[33];
    uint8_t bytes[32];
};

struct TxIdRecord{
    char id[33];
    uint8_t bytes[32];
};

struct BaseOutputRecord{
    type_id_t type_id;
    uint64_t amount;
    uint64_t locktime;
    uint32_t threshold;
    size_t addresses_len;
    struct AddressRecord * addresses;
};

struct SECP256K1TransferOutputRecord{
    struct BaseOutputRecord base;
    uint64_t amount;
};
//SECP256K1TransferOutputRecord_Default = { .base.type_id= SECP256K1TransferOutput};


struct TransferableOutputRecord{
    struct AssetRecord asset; //32 bytes
    size_t outputs_len;
    struct BaseOutputRecord ** outputs;
   
};

struct BaseInputRecord{
    type_id_t type_id;                  // 04 bytes
    uint64_t amount;                   // 08 bytes
    size_t address_indices_len;
    uint32_t * address_indices; //variable array of 04 bytes
};
//BaseInputRecord_Default = {.type_id = SECP256K1TransferInput };

struct TranferableInputRecord{
    struct TxIdRecord tx; //32 byte txid
    uint32_t utxo_index;
    struct AssetRecord asset; //32 byte
    size_t inputs_len;
    struct BaseInputRecord ** inputs;
 };

struct BaseTxRecord{
    type_id_t type_id; // 4 bytes
    network_id_t network_id; //4 bytes
    char blockchain_id[32];//no null terminating char
    struct TransferableOutputRecord ** outputs;
    size_t outputs_len;
    struct TranferableInputRecord ** inputs;
    size_t inputs_len;
    char memo [256];//no null terminating char
};


extern void avaxCreateBaseTx();

extern void serializeOutputs();

extern void serializeSECP256K1TransferOutputRecord(struct SECP256K1TransferOutputRecord * secp256k1TransferOutput );

extern void serializeBaseInputRecord(struct BaseInputRecord * secp256k1TransferInput );


extern void serializeInput(struct TranferableInputRecord * input);

extern void serializeInput(struct TranferableInputRecord * input);


extern void serializeBaseTx();

extern void signTx();


#ifdef __cplusplus
}
#endif

#endif /* BRAvaxTransaction_h */
