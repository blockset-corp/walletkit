//
//  BRAvalancheTransaction.h
//  
//
//  Created by Amit on 21/06/2021.
//

#ifndef BRAvalancheTransaction_h
#define BRAvalancheTransaction_h

#include <stdio.h>
#include "BRAvalancheAddress.h"

#ifndef AVAX
#define AVAX(THING) BRAvalanche## THING
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { AVAX(SECP256K1TransferOutput) = 0x00000007,AVAX(SECP256K1OutputOwners) = 0x0000000b } AVAX(TransferOutputType);

typedef struct AVAX(TransferOutputBaseRecord) * AVAX(TransferOutputBase);

typedef struct AVAX(TransferableOutputRecord) * AVAX(TransferableOutput);

typedef struct AVAX(SECP256K1TransferInputRecord) * AVAX(SECP256K1TransferInput);

typedef struct AVAX(TransferableInputRecord) * AVAX(TransferableInput);

typedef struct AVAX(BaseTxRecord) * AVAX(BaseTx);


extern void createBaseTx();

#ifdef __cplusplus
}
#endif

#endif /* BRAvalancheTransaction_h */
