//
//  File.h
//  
//
//  Created by Amit on 17/06/2021.
//

#ifndef BRAvalancheUtil_h
#define BRAvalancheUtil_h

#include <stdio.h>
#include "WKBase.h"
#ifdef __cplusplus
extern "C" {
#endif

//extern uint8_t* avaxEncodeInt32(uint32_t input);

#define to_uint64(buffer,n) ((uint64_t)buffer[n+7] << 56 | (uint64_t)buffer[n+6] << 48 | (uint64_t)buffer[n+5] << 40  | (uint64_t)buffer[n+4] << 32 | (uint64_t) buffer[n+3] << 24 | (uint64_t)buffer[n+2] << 16 | (uint64_t)buffer[n+1] << 8  | (uint64_t)buffer[n])

int avax_bech32_encode(char *const output, size_t *const out_len,
                  const char *const hrp, const size_t hrp_len,
                       const uint8_t *const data, const size_t data_len);

int avax_base32_encode(
    uint8_t *const out, size_t *out_len,
                       const uint8_t *const in, const size_t in_len);

int avax_bech32_decode(char *hrp, uint8_t *data, size_t *data_len, const char *input);



#ifdef __cplusplus
}
#endif
#endif /* File_h */
