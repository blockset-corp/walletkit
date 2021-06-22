//
//  BRAvalancheTransaction.c
//
//
//  Created by Amit on 19/06/2021.
//

#include "BRAvalancheTransaction.h"
#include <stdlib.h>

//SECP256K1TransferOutput
//type_id   : int        |                        4 bytes |
//+-----------+------------+--------------------------------+
//| amount    : long       |                        8 bytes |
//+-----------+------------+--------------------------------+
//| locktime  : long       |                        8 bytes |
//+-----------+------------+--------------------------------+
//| threshold : int        |                        4 bytes |
//+-----------+------------+--------------------------------+
//| addresses : [][20]byte |  4 + 20 * len(addresses) bytes |

//SECP256K1OutputOwners
//+-----------+------------+--------------------------------+
//| type_id   : int        |                        4 bytes |
//+-----------+------------+--------------------------------+
//| locktime  : long       |                        8 bytes |
//+-----------+------------+--------------------------------+
//| threshold : int        |                        4 bytes |
//+-----------+------------+--------------------------------+
//| addresses : [][20]byte |  4 + 20 * len(addresses) bytes |
//+-----------+------------+--------------------------------+
//                         | 20 + 20 * len(addresses) bytes |
//                         +--------------------------------+

typedef enum { SECP256K1TransferOutput = 0x3,SECP256K1OutputOwners = 0X8 } BRAvalancheTransferOutputType;

struct TransferOutputBaseRecord {
    BRAvalancheTransferOutputType type;//4 bytes
    uint32_t type_id;
    uint64_t locktime; //8 bytes
    uint32_t threshold; //4 bytes
    size_t addresslen;
    BRAvalancheXAddress address;//array of addresses
    union {
        uint64_t amount;
    }target;
} ;

typedef struct TransferOutputBaseRecord *TransferOutputBase;

struct TransferrableOutputRecord {
    uint8_t asset_id[32]; // 32 bytes
    size_t outputlen;
    TransferOutputBase output;  // size(output) // can be of type SECP256K1TransferOutput,SECP256K1OutputOwners
};

typedef struct TransferrableOutputRecord * TransferrableOutput;

//
//+-------------------------+-------------------------------------+
//| type_id         : int   |                             4 bytes |
//+-----------------+-------+-------------------------------------+
//| amount          : long  |                             8 bytes |
//+-----------------+-------+-------------------------------------+
//| address_indices : []int |  4 + 4 * len(address_indices) bytes |
//+-----------------+-------+-------------------------------------+
//                          | 16 + 4 * len(address_indices) bytes |
//                          +-------------------------------------+

struct SECP256K1TransferInputRecord {
    uint32_t type_id;                  // 04 bytes
    uint64_t amount;                   // 08 bytes
    size_t address_indiceslen;
    uint32_t * address_indices; // 04 bytes + 4 bytes * len(address_indices)
};

typedef struct SECP256K1TransferInputRecord * SECP256K1TransferInput;
//
//+------------+----------+------------------------+
//| tx_id      : [32]byte |               32 bytes |
//+------------+----------+------------------------+
//| utxo_index : int      |               04 bytes |
//+------------+----------+------------------------+
//| asset_id   : [32]byte |               32 bytes |
//+------------+----------+------------------------+
//| input      : Input    |      size(input) bytes |
//+------------+----------+------------------------+
//                        | 68 + size(input) bytes |
//                        +------------------------+
struct BRAvalancheTxId {
    uint8_t bytes[32];
};

struct BRAvalancheAssetId {
    uint8_t bytes[32];
};

struct TransferrableInputRecord{
    struct BRAvalancheTxId tx_id;
    uint32_t utxo_index;
    struct BRAvalancheAssetId asset_id;
    size_t inputlen;
    SECP256K1TransferInput * input;
};

typedef struct TransferrableInputRecord * TransferrableInput;


////+---------------+----------------------+-----------------------------------------+
//| type_id       : int                  |                                 4 bytes |
//+---------------+----------------------+-----------------------------------------+
//| network_id    : int                  |                                 4 bytes |
//+---------------+----------------------+-----------------------------------------+
//| blockchain_id : [32]byte             |                                32 bytes |
//+---------------+----------------------+-----------------------------------------+
//| outputs       : []TransferableOutput |                 4 + size(outputs) bytes |
//+---------------+----------------------+-----------------------------------------+
//| inputs        : []TransferableInput  |                  4 + size(inputs) bytes |
//+---------------+----------------------+-----------------------------------------+
//| memo          : [256]byte            |                    4 + size(memo) bytes |
//+---------------+----------------------+-----------------------------------------+
//                          | 52 + size(outputs) + size(inputs) + size(memo) bytes |
//                          +------------------------------------------------------+


struct BaseTxRecord {
    uint32_t type_id;          // 04 bytes
    uint32_t network_id;       // 04 bytes
    uint8_t blockchain_id[32];     // 32 bytes
    size_t outputslen;
    TransferrableOutput outputs; // 04 bytes + size(outs) //TransferrableOutputs
    TransferrableOutput * transfers;
    
    size_t inputslen;
    TransferrableInput * inputs;   // 04 bytes + size(ins) //TransferrableInputs
    size_t memolen;
    uint8_t *  memo;              // 04 bytes + size(memo)
} BaseTx_DEFAULT = {.type_id=277};

typedef struct BaseTx_DEFAULT BaseTxRecord;

typedef struct BaseTxRecord * BaseTx;

struct point{
    int x;
    int y;
};
typedef struct point * pt;

extern void createBaseTx(){
    TransferOutputBase transfer = calloc(1, sizeof(struct TransferrableOutputRecord));
    transfer->type_id=SECP256K1TransferOutput;
    transfer->target.amount = 1001;
    
    TransferOutputBase transfer2 = calloc(1, sizeof(struct TransferrableOutputRecord));
    transfer2->type_id=SECP256K1OutputOwners;
    
    BaseTx tx = calloc(1, sizeof(struct BaseTxRecord));
    tx->outputs = calloc(1, sizeof(struct TransferrableOutputRecord) );
    tx->outputs[0].outputlen =2 ;
    tx->outputs[0].output = calloc(2, sizeof(struct TransferOutputBaseRecord));
    tx->outputs[0].output[0].type_id = 0xa;
    tx->outputs[0].output[0].target.amount = 666;
    tx->outputs[0].output[0].type=SECP256K1TransferOutput;
    tx->outputs[0].output[1].type_id = 0xb;
    tx->outputs[0].output[1].target.amount = 808;
    tx->outputs[0].output[1].type=SECP256K1OutputOwners;

    for(int i=0; i < tx->outputs[0].outputlen; i++){
        printf("%d - %d \r\n", tx->outputs[0].output[i].type_id, tx->outputs[0].output[i].target.amount);
        
    }
    
    free(tx->outputs[0].output);
    free(tx->outputs);
    free(tx);
    free(transfer);
    free(transfer2);
    

    
}

