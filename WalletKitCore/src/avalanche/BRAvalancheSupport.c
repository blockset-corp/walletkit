//
//  BRAvalancheSupport.h
//  WalletKitCore
//
//  Created by Amit Shah on 2021-08-04.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "BRAvalancheSupport.h"
#include "BRAvalancheAddress.h"

#include "support/BRInt.h"
#include "support/BRCrypto.h"
#include "support/BRBase58.h"

#include <ctype.h>

// MARK: - Hash

extern BRAvalancheHash
avalancheHashFromString(const char *input) {
    size_t hashSize = BRAvalancheCB58CheckDecode (NULL, 0, input);
    assert (AVALANCHE_HASH_BYTES == hashSize);

    BRAvalancheHash hash;
    BRAvalancheCB58CheckDecode (hash.bytes, AVALANCHE_HASH_BYTES, input);

    return hash;
}

extern char *
avalancheHashToString (BRAvalancheHash hash) {

    //    "sign the transaction and issue it to the Avalanche network. If successful it will return a
    //    CB58 serialized string for the TxID."
    //
    // issueTx - returns a CB58 serialized string for the TxID

    size_t stringSize = BRAvalancheCB58CheckEncode (NULL, 0, hash.bytes, sizeof(hash.bytes));
    char  *string     = malloc (stringSize);

    BRAvalancheCB58CheckEncode (string, stringSize, hash.bytes, sizeof(hash.bytes));

    return string;
}

// MARK: - CB58

// CB58 uses the final 4 bytes of SHA256(msg) as the checksum. Base58Check uses the first 4 bytes
// of SHA256(SHA256(msg)) as the checksum. Because "I am Different, Special!"

extern size_t
BRAvalancheCB58CheckEncode(char *str, size_t strLen,
                           const uint8_t *data, size_t dataLen) {
    assert(data != NULL || dataLen == 0);

    // Return
    size_t len = 0;

    // We'll tack on 4 bytes, from a digest, to `dataLen`
    size_t bufLen = dataLen + 4;

    // Get some temporary storage
    uint8_t _buf[0x1000];
    uint8_t *buf = (bufLen <= 0x1000) ? _buf : malloc(bufLen);
    assert(buf != NULL);

    if (data || dataLen == 0) {
        UInt256 md;
        BRSHA256(md.u8, data, dataLen);

        memcpy (&buf[0],        data,      dataLen);            // `data` into `buf`
        memcpy (&buf[dataLen], &md.u8[28], 4);                  // last 4 bytes of `md` into `buf`

        // Base58 encode the full buffer
        len = BRBase58Encode(str, strLen, buf, bufLen);
    }

    // Clean up
    mem_clean(buf, bufLen);
    if (buf != _buf) free(buf);

    // Return the required length, more or less (see BRBase58Encode())
    return len;
}

extern char *
BRAvalancheCB58CheckEncodeCreate (const uint8_t *data, size_t dataLen) {
    size_t strLength = BRAvalancheCB58CheckEncode (NULL, 0, data, dataLen);
    char  *str       = malloc (strLength + 1);

    BRAvalancheCB58CheckEncode (str, strLength, data, dataLen);

    return str;
}

extern size_t // This is BRBase58CheckDecode but w/ only BRSHA256
BRAvalancheCB58CheckDecode(uint8_t *data, size_t dataLen,
                           const char *str) {
    assert(str != NULL);

    size_t len;
    size_t bufLen = (str) ? strlen(str) : 0;

    uint8_t _buf[0x1000], *buf = (bufLen <= 0x1000) ? _buf : malloc(bufLen);
    assert(buf != NULL);

    len = BRBase58Decode(buf, bufLen, str);

    if (len >= 4) {
        len -= 4;

        // Compute the digest of the decoded `buf` but skip the 4 checksum bytes at the end
        UInt256 md;
        BRSHA256(md.u8, buf, len);

        // Verify the checksum
        if (0 != memcmp(&buf[len], &md.u8[28], 4)) len = 0;

        // Fill in the result
        if (data && len <= dataLen) memcpy (data, buf, len);
    }
    else len = 0;

    // Cleanup
    mem_clean(buf, bufLen);
    if (buf != _buf) free(buf);

    // Return the required length
    return (! data || len <= dataLen) ? len : 0;
}

extern uint8_t *
BRAvalancheCB58CheckDecodeCreate(const char *str, size_t *dataLen) {
    size_t   dataLength = BRAvalancheCB58CheckDecode (NULL, 0, str);
    uint8_t *data       = malloc (dataLength);
    BRAvalancheCB58CheckDecode (data, dataLength, str);

    if (NULL != dataLen) *dataLen = dataLength;
    return data;
}


// MARK: - Support

//TODO: move everything below here to vendored utils
//https://github.com/ava-labs/ledger-app-avalanche/blob/a340702ec427b15e7e4ed31c47cc5e2fb170ebdf/src/bech32encode.c#L28
//thanks to ledger for your hard work xo
static uint32_t
bech32_polymod_step(uint32_t pre) {
    uint8_t b = pre >> 25;
    return (((pre & 0x1FFFFFF) << 5) ^
            (-((b >> 0) & 1) & 0x3b6a57b2UL) ^
            (-((b >> 1) & 1) & 0x26508e6dUL) ^
            (-((b >> 2) & 1) & 0x1ea119faUL) ^
            (-((b >> 3) & 1) & 0x3d4233ddUL) ^
            (-((b >> 4) & 1) & 0x2a1462b3UL));
}

extern bool
avax_base32_encode(uint8_t *const out, size_t *out_len,
                   const uint8_t *const in, const size_t in_len) {
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
                return false;
            out[out_idx++] = (val >> bits) & maxv;
        }
    }
    if (pad) {
        if (bits) {
            if (out_idx >= out_len_max)
                return false;
            out[out_idx++] = (val << (outbits - bits)) & maxv;
        }
    } else if (((val << (outbits - bits)) & maxv) || bits >= inbits) {
        return false;
    }
    // Set out index
    *out_len = out_idx;
    return true;
}

static const char *charset_bech32 = "qpzry9x8gf2tvdw0s3jn54khce6mua7l";

static const int8_t charset_rev[128] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 15, -1, 10, 17,
    21, 20, 26, 30, 7,  5,  -1, -1, -1, -1, -1, -1, -1, 29, -1, 24, 13, 25, 9,  8,  23, -1, 18, 22, 31, 27,
    19, -1, 1,  0,  3,  16, 11, 28, 12, 14, 6,  4,  2,  -1, -1, -1, -1, -1, -1, 29, -1, 24, 13, 25, 9,  8,
    23, -1, 18, 22, 31, 27, 19, -1, 1,  0,  3,  16, 11, 28, 12, 14, 6,  4,  2,  -1, -1, -1, -1, -1};

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
extern bool
avax_bech32_encode(char *const output, size_t *const out_len,
                   const char *const hrp, const size_t hrp_len,
                   const uint8_t *const data, const size_t data_len) {
    uint32_t chk = 1;
    size_t out_off = 0;
    const size_t out_len_max = *out_len;
    // hrp '1' data checksum
    const size_t final_out_len = hrp_len + data_len + 7;
    if (final_out_len > 108)
        return false;
    // Note we want <=, to account for the null at the end of the string
    // i.e. equivalent to out_len_max < final_out_len + 1
    if (output == NULL || out_len_max <= final_out_len) return false;
    if (hrp    == NULL || hrp_len  <= 0) return false;
    if (data   == NULL || data_len <= 0) return false;

    for (size_t i = 0; i < hrp_len; ++i) {
        char ch = hrp[i];
        if (!(33 <= ch && ch <= 126))
            return false;
        chk = bech32_polymod_step(chk) ^ ((uint32_t) (ch >> 5));
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
            return false;
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

//https://github.com/iotaledger/iota.c/blob/6b94ad03c915139d8292513d7849cb5e1a20df2f/tests/core/test_utils_bech32.c
extern bool
avax_bech32_decode(char *hrp, uint8_t *data, size_t *data_len, const char *input) {
    uint32_t chk = 1;
    size_t i;
    size_t input_len = strlen(input);
    size_t hrp_len;
    int have_lower = 0, have_upper = 0;
    if (input_len < 8 || input_len > 108) {
        return false;
    }
    *data_len = 0;
    while (*data_len < input_len && input[(input_len - 1) - *data_len] != '1') {
        ++(*data_len);
    }
    if (1 + *data_len >= input_len || *data_len < 6) {
        return false;
    }
    hrp_len = input_len - (1 + *data_len);
    *(data_len) -= 6;
    for (i = 0; i < hrp_len; ++i) {
        int ch = input[i];

        if (!isgraph(ch)) return false;
        have_lower = islower(ch);
        have_upper = isupper(ch);
        if (have_upper) ch = tolower(ch);

        hrp[i] = ch;
        chk = bech32_polymod_step(chk) ^ ((uint32_t) (ch >> 5));
    }
    hrp[i] = 0;
    chk = bech32_polymod_step(chk);
    for (i = 0; i < hrp_len; ++i) {
        chk = bech32_polymod_step(chk) ^ (input[i] & 0x1f);
    }
    ++i;
    while (i < input_len) {
        int v = (input[i] & 0x80) ? -1 : charset_rev[(int)input[i]];
        have_lower = islower(input[i]);
        have_upper = isupper(input[i]);
        if (v == -1) return false;

        chk = bech32_polymod_step(chk) ^ ((uint32_t) v);
        if (i + 6 < input_len) {
            data[i - (1 + hrp_len)] = v;
        }
        ++i;
    }

    return ((!have_lower || !have_upper) && (1 == chk));
}


static bool
avax_convert_bits(uint8_t *out, size_t *outlen, int outbits,
                  const uint8_t *in, size_t inlen, int inbits,
                  int pad) {
    uint32_t val = 0;
    int bits = 0;
    uint32_t maxv = (((uint32_t)1) << outbits) - 1;
    while (inlen--) {
        val = (val << inbits) | *(in++);
        bits += inbits;
        while (bits >= outbits) {
            bits -= outbits;
            out[(*outlen)++] = (val >> bits) & maxv;
        }
    }
    if (pad) {
        if (bits) {
            out[(*outlen)++] = (val << (outbits - bits)) & maxv;
        }
    } else if (((val << (outbits - bits)) & maxv) || bits >= inbits) {
        return false;
    }
    return true;
}

#define THE_NUMBER_IS_FORTY_THREE   (43)

extern bool
avax_addr_bech32_decode(uint8_t *addr_data, size_t *addr_len, const char *hrp, const char *addr_str) {
    uint8_t data[THE_NUMBER_IS_FORTY_THREE - (strlen(hrp)+1)]; // total - (hrp + 1) =
    char hrp_actual[strlen(hrp)+1];//we only expect the hrp = avax\0
    size_t data_len = 0;

    if (!avax_bech32_decode(hrp_actual, data, &data_len, addr_str)) {
        return false;
    }

    if (data_len == 0 || data_len > 64) {
        return false;
    }

    //  if (strncmp(hrp, hrp_actual, 84) != 0) {
    //    return 0;
    //  }

    *addr_len = 0;
    if (!avax_convert_bits(addr_data, addr_len, 8, data, data_len, 5, 0)) {
        return false;
    }

    return true;
}


#if 0
int uint32_t_to_bytes(uint32_t data, uint8_t * output){
    output[0] = (uint8_t)(data >>  0);
    output[1] = (uint8_t)(data >>  8);
    output[2] = (uint8_t)(data >> 16);
    output[3] = (uint8_t)(data >> 24);
    return 1;
};

int uint64_t_to_bytes(uint64_t data, uint8_t * output){
    output[0] = (uint8_t)(data >>  0);
    output[1] = (uint8_t)(data >>  8);
    output[2] = (uint8_t)(data >> 16);
    output[3] = (uint8_t)(data >> 24);
    output[4] = (uint8_t)(data >> 32);
    output[5] = (uint8_t)(data >> 40);
    output[6] = (uint8_t)(data >> 48);
    output[7] = (uint8_t)(data >> 56);
    return 1;
};
#endif
