//
//  File.h
//  
//
//  Created by Amit on 17/08/2021.
//

#ifndef BRAvaxEncode_h
#define BRAvaxEncode_h

#include <stdio.h>
#include "BRAvaxTransaction.h"
#include "support/BRArray.h"

#ifdef __cplusplus
extern "C" {
#endif

extern uint8_t * avaxPackTransferableOutput(struct TransferableOutputRecord * output, uint8_t * buffer );

extern uint8_t * avaxPackTransferableInput(struct TranferableInputRecord * input, uint8_t * buffer);

extern uint8_t * avaxPackAddress(struct AddressRecord * address, uint8_t * buffer);

extern uint8_t * avaxPackUInt32(uint32_t value,uint8_t * buffer);

extern uint8_t * avaxPackUInt64(uint64_t value,uint8_t * buffer);

extern uint8_t * avaxPackByteArray(uint8_t * bytes, size_t len,uint8_t * buffer, int include_len_prefix);


extern void avaxPackBaseTx(struct BaseTxRecord baseTx);

#ifdef __cplusplus
}
#endif

#endif /* BRAvaxEncode_h */
