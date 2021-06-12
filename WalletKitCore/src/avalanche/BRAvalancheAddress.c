//
//  BRAvalancheAddress.c
//  
//
//  Created by Amit on 10/06/2021.
//

#include "BRAvalancheAddress.h"
#include "support/BRKey.h"
#include "support/BRInt.h"
#include "support/BRCrypto.h"
#include "support/BRBech32.h"

struct BRAvalancheAddressRecord {
    uint8_t bytes[AVAX_X_ADDRESS_BYTES];
    //todo ethereum address
};

//generate x-chain address from seed
// Bech32Encode("avax",Base32Encode(ripemd160(sha256(publicKey))))
extern BRAvalancheAddress
avalancheAddressCreateFromKey (const uint8_t * pubKey, size_t pubKeyLen) {
    BRAvalancheAddress address = calloc(1, sizeof(struct BRAvalancheAddressRecord));
    uint8_t pkh[32];
    uint8_t rmd160[20];
    
    BRSHA256(pkh, pubKey, pubKeyLen);
    BRRMD160(rmd160, pkh, 32);
    //========= right up to here
    //uint8_t data[22] = { 0, 20 };
    //memcpy(&data[2], rmd160, 20);
    //BRBech32Encode(addr, AVAX_HRP , data);
    static const char *hrp = "avax";
    char addr[91];
    size_t * size = 50;
    uint8_t * hrpSize = 4;
    uint8_t * dataLen = 32;
    avax_base32_encode(pkh, &dataLen, rmd160, sizeof(rmd160));
    avax_bech32_encode(&addr[0], &size, hrp, hrpSize, pkh, sizeof(pkh));
    //size_t BRBech32Encode(char *addr91, const char *hrp, const uint8_t data[]);
    
    //uint8_t pkh[BLAKE20_BYTES];
    //blake2b(pkh, sizeof(pkh), NULL, 0, pubKey, pubKeyLen);

    //memcpy(address->bytes, TZ1_PREFIX, sizeof(TZ1_PREFIX));
    //memcpy(address->bytes + sizeof(TZ1_PREFIX), pkh, sizeof(pkh));
    return address;
}

//https://github.com/ava-labs/ledger-app-avalanche/blob/a340702ec427b15e7e4ed31c47cc5e2fb170ebdf/src/bech32encode.c#L28
//thanks to ledger for your hard work xo
uint32_t bech32_polymod_step(uint32_t pre) {
    uint8_t b = pre >> 25;
    return ((pre & 0x1FFFFFF) << 5) ^ (-((b >> 0) & 1) & 0x3b6a57b2UL) ^ (-((b >> 1) & 1) & 0x26508e6dUL) ^
           (-((b >> 2) & 1) & 0x1ea119faUL) ^ (-((b >> 3) & 1) & 0x3d4233ddUL) ^ (-((b >> 4) & 1) & 0x2a1462b3UL);
}

static const char *charset_bech32 = "qpzry9x8gf2tvdw0s3jn54khce6mua7l";

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

int avax_bech32_encode(char *const output, size_t *const out_len,
                  const char *const hrp, const size_t hrp_len,
                  const uint8_t *const data, const size_t data_len) {
    uint32_t chk = 1;
    size_t out_off = 0;
    const size_t out_len_max = *out_len;
    // hrp '1' data checksum
    const size_t final_out_len = hrp_len + data_len + 7;
    if (final_out_len > 108)
        return 0;
    // Note we want <=, to account for the null at the end of the string
    // i.e. equivalent to out_len_max < final_out_len + 1
    if (output == NULL || out_len_max <= final_out_len)
        return 0;
    if (hrp == NULL || hrp_len <= 0)
        return 0;
    if (data == NULL || data_len <= 0)
        return 0;
    for (size_t i = 0; i < hrp_len; ++i) {
        char ch = hrp[i];
        if (!(33 <= ch && ch <= 126))
            return 0;
        chk = bech32_polymod_step(chk) ^ (ch >> 5);
    }
    chk = bech32_polymod_step(chk);
    for (size_t i = 0; i < hrp_len; ++i) {
        char ch = hrp[i];
        chk = bech32_polymod_step(chk) ^ (ch & 0x1f);
        output[out_off++] = ch;
    }
    output[out_off++] = '1';
    for (size_t i = 0; i < data_len; ++i) {
        if (data[i] >> 5)
            return 0;
        chk = bech32_polymod_step(chk) ^ data[i];
        output[out_off++] = charset_bech32[data[i]];
    }
    for (size_t i = 0; i < 6; ++i) {
        chk = bech32_polymod_step(chk);
    }
    chk ^= 1;
    for (size_t i = 0; i < 6; ++i) {
        output[out_off++] = charset_bech32[(chk >> ((5 - i) * 5)) & 0x1f];
    }
    output[out_off] = 0;
    *out_len = out_off;
    return (out_off == final_out_len);
}

int avax_base32_encode(
    uint8_t *const out, size_t *out_len,
    const uint8_t *const in, const size_t in_len)
{
    const int outbits = 5;
    const int inbits = 8;
    const int pad = 1;
    uint32_t val = 0;
    int bits = 0;
    size_t out_idx = 0;
    const size_t out_len_max = *out_len;
    const uint32_t maxv = (((uint32_t)1) << outbits) - 1;
    for (size_t inx_idx = 0; inx_idx < in_len; ++inx_idx) {
        val = (val << inbits) | in[inx_idx];
        bits += inbits;
        while (bits >= outbits) {
            bits -= outbits;
            if (out_idx >= out_len_max)
                return 0;
            out[out_idx++] = (val >> bits) & maxv;
        }
    }
    if (pad) {
        if (bits) {
            if (out_idx >= out_len_max)
                return 0;
            out[out_idx++] = (val << (outbits - bits)) & maxv;
        }
    } else if (((val << (outbits - bits)) & maxv) || bits >= inbits) {
        return 0;
    }
    // Set out index
    *out_len = out_idx;
    return 1;
}


