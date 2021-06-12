//
//  BRAvalancheAddress.h
//  
//
//  Created by Amit on 10/06/2021.
//

#ifndef BRAvalancheAddress_h
#define BRAvalancheAddress_h

#include <stdio.h>

#define AVAX_X_ADDRESS_BYTES (23)

#define AVAX_C_ADDRESS_BYTES (23)

#define AVAX_HRP ("avax")

#ifdef __cplusplus
extern "C" {
#endif
typedef struct BRAvalancheAddressRecord *BRAvalancheAddress;

extern BRAvalancheAddress
avalancheAddressCreateFromKey (const uint8_t * pubKey, size_t pubKeyLen);

/** Encode a Bech32 string
 *
 *  Out:
 *      output:  Pointer to a buffer of size strlen(hrp) + data_len + 8 that
 *               will be updated to contain the null-terminated Bech32 string.
 *  In/Out:
 *      out_len: Length of output buffer so no overflows occur.
 *  In:
 *      hrp :     Pointer to the human readable part.
 *      hrp_len:  length of the human readable part
 *      data :    Pointer to an array of 5-bit values.
 *      data_len: Length of the data array.
 *  Returns 0 on failure, or strlen(output) if successful
 */
int avax_bech32_encode(char *output, size_t *const out_len,
                  const char *hrp, const size_t hrp_len,
                  const uint8_t *data, size_t data_len);


/** Encode a Bech32 string
 *
 *  Out:
 *      output:  Pointer to a buffer of size strlen(hrp) + data_len + 8 that
 *               will be updated to contain the null-terminated Bech32 string.
 *  In/Out:
 *      out_len: Length of output buffer so no overflows occur.
 *  In:
 *      hrp :     Pointer to the human readable part.
 *      hrp_len:  length of the human readable part
 *      data :    Pointer to an array of 5-bit values.
 *      data_len: Length of the data array.
 *  Returns 0 on failure, or strlen(output) if successful
 */

int avax_base32_encode(
    uint8_t *const out, size_t *const out_len,
    const uint8_t *const in, const size_t inlen
    );
#ifdef __cplusplus
}
#endif

#endif /* BRAvalancheAddress_h */
