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



struct AVAX(TransferOutputBaseRecord) {
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

struct AVAX(TransferableOutputRecord) {
    uint8_t asset_id[32]; // 32 bytes
    size_t outputlen;
    BRAvalancheTransferOutputBase output;  // size(output) // can be of type SECP256K1TransferOutput,SECP256K1OutputOwners
};


struct SECP256K1TransferOutputRecord{
    struct AVAX(TransferOutputBaseRecord) base;
    uint64_t amount;
}SECP256K1TransferOutput_DEFAULT = { .base.type= BRAvalancheSECP256K1TransferOutput};

typedef struct SECP256K1TransferOutputRecord * SECP256K1TransferOutput;
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

struct AVAX(SECP256K1TransferInputRecord) {
    uint32_t type_id;                  // 04 bytes
    uint64_t amount;                   // 08 bytes
    size_t address_indiceslen;
    uint32_t * address_indices; // 04 bytes + 4 bytes * len(address_indices)
};

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
struct AVAX(Txid) {
    uint8_t bytes[32];
};

struct AVAX(AssetId) {
    uint8_t bytes[32];
};

struct AVAX(TransferableInputRecord){
    struct BRAvalancheTxid tx_id;
    uint32_t utxo_index;
    struct BRAvalancheAssetId asset_id;
    size_t inputlen;
    BRAvalancheSECP256K1TransferInput * input;
};



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
struct AVAX(BaseTxRecord) {
    uint32_t type_id;          // 04 bytes
    uint32_t network_id;       // 04 bytes
    uint8_t blockchain_id[32];     // 32 bytes
    size_t outputslen;
    BRAvalancheTransferableOutput outputs; // 04 bytes + size(outs) //TransferrableOutputs
    BRAvalancheTransferableOutput * transfers;
    
    size_t inputslen;
    BRAvalancheTransferableInput * inputs;   // 04 bytes + size(ins) //TransferrableInputs
    size_t memolen;
    uint8_t *  memo;              // 04 bytes + size(memo)
} AVAX(BaseTx_DEFAULT) = { .type_id =0x01};


extern void createBaseTx(){
   
    
   
    
    SECP256K1TransferOutput output1 = calloc(1, sizeof(struct SECP256K1TransferOutputRecord));
    *output1 = SECP256K1TransferOutput_DEFAULT;
    output1->amount = 666;
    
    
    BRAvalancheBaseTx tx = calloc(1, sizeof(struct BRAvalancheBaseTxRecord));
    *tx = BRAvalancheBaseTx_DEFAULT;
    tx->outputs = calloc(1, sizeof(struct BRAvalancheTransferableOutputRecord) );
    tx->outputs[0].outputlen =2 ;
    tx->outputs[0].output = calloc(2, sizeof(struct BRAvalancheTransferOutputBaseRecord));
    tx->outputs[0].output[0].type_id = 0xa;
    tx->outputs[0].output[0].target.amount = 666;
    tx->outputs[0].output[0].type=BRAvalancheSECP256K1TransferOutput;
    tx->outputs[0].output[1].type_id = 0xb;
    tx->outputs[0].output[1].target.amount = 808;
    tx->outputs[0].output[1].type=BRAvalancheSECP256K1OutputOwners;

    for(int i=0; i < tx->outputs[0].outputlen; i++){
        printf("%d - %d \r\n", tx->outputs[0].output[i].type, tx->outputs[0].output[i].target.amount);
        BRAvalancheTransferOutputType type = tx->outputs[0].output[i].type;
        switch(type){
            case BRAvalancheSECP256K1TransferOutput: printf("got a transfer \r\n"); break;
            case BRAvalancheSECP256K1OutputOwners: printf("owner swap\r\n");
                break;
            default: printf("!!!unknown transfer type!!!\r\n"); break;
        }
        
    }

    free(tx->outputs[0].output);
    free(tx->outputs);
    free(tx);    
}

void * createOutputs(BRAvalancheXAddress address){
    BRAvalancheTransferOutputBase to = calloc(1, sizeof(struct BRAvalancheTransferOutputBaseRecord));
    to->address = address;
    return to;

}

