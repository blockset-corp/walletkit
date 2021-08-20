//
//  BRAvalancheAddress.c
//  WalletKitCore
//
//  Created by Amit Shah on 2021-08-04.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "BRAvalancheAddress.h"
#include "BRAvalancheBase.h"

#include "support/BRCrypto.h"
#include "support/util/BRHex.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include <stdbool.h>

//
#define AVALANCHE_ADDRESS_BYTES     (AVALANCHE_ADDRESS_BYTES_X > AVALANCHE_ADDRESS_BYTES_C ? AVALANCHE_ADDRESS_BYTES_X : AVALANCHE_ADDRESS_BYTES_C)


// MARK: Fee Address

static uint8_t feeAddressBytes [AVALANCHE_ADDRESS_BYTES] = { 0 };

static BRAvalancheAddress
avalancheAddressCreateFeeAddress(BRAvalancheChainType type) {
    BRAvalancheAddress address = { type };
    switch (type) {
        case AVALANCHE_CHAIN_TYPE_X:
            memcpy (address.u.x.bytes, feeAddressBytes, AVALANCHE_ADDRESS_BYTES_X);
            break;

        case AVALANCHE_CHAIN_TYPE_C:
            memcpy (address.u.c.bytes, feeAddressBytes, AVALANCHE_ADDRESS_BYTES_C);
            break;

        case AVALANCHE_CHAIN_TYPE_P:
            assert (false);
            break;
    }

    return address;
}

extern bool
avalancheAddressIsFeeAddress (BRAvalancheAddress address) {
    switch (address.type) {
        case AVALANCHE_CHAIN_TYPE_X:
            return 0 == memcmp (address.u.x.bytes, feeAddressBytes, AVALANCHE_ADDRESS_BYTES_X);

        case AVALANCHE_CHAIN_TYPE_C:
            return 0 == memcmp (address.u.c.bytes, feeAddressBytes, AVALANCHE_ADDRESS_BYTES_X);

        case AVALANCHE_CHAIN_TYPE_P:
            return false;
    }
}


// MARK: - Unknown Address

static uint8_t unknownAddressBytes [AVALANCHE_ADDRESS_BYTES] = { 0 };

static BRAvalancheAddress
avalancheAddressCreateUnknownAddress(BRAvalancheChainType type) {
    BRAvalancheAddress address = { type };
    switch (type) {
        case AVALANCHE_CHAIN_TYPE_X:
            memcpy (address.u.x.bytes, unknownAddressBytes, AVALANCHE_ADDRESS_BYTES_X);
            break;

        case AVALANCHE_CHAIN_TYPE_C:
            memcpy (address.u.c.bytes, unknownAddressBytes, AVALANCHE_ADDRESS_BYTES_C);
            break;

        case AVALANCHE_CHAIN_TYPE_P:
            assert (false);
            break;
    }

    return address;
}

extern bool
avalancheAddressIsUnknownAddress (BRAvalancheAddress address) {
    switch (address.type) {
        case AVALANCHE_CHAIN_TYPE_X:
            return 0 == memcmp (address.u.x.bytes, unknownAddressBytes, AVALANCHE_ADDRESS_BYTES_X);

        case AVALANCHE_CHAIN_TYPE_C:
            return 0 == memcmp (address.u.c.bytes, unknownAddressBytes, AVALANCHE_ADDRESS_BYTES_X);

        case AVALANCHE_CHAIN_TYPE_P:
            return false;
    }
}

// MARK: - Address As String

static char *
avalancheAddressAsStringX (BRAvalancheAddress address) {
    assert (AVALANCHE_CHAIN_TYPE_X == address.type);

    static const char *hrp = "avax";
    const size_t hrpLen = strlen (hrp);

    UInt256 pkh;
    size_t pkhLen = sizeof(UInt256);

    char *result = malloc (44);
    size_t resultLen = 44;

    avax_base32_encode (pkh.u8, &pkhLen, address.u.x.bytes, sizeof(address.u.x.bytes));
    avax_bech32_encode (result, &resultLen, hrp, hrpLen, pkh.u8, pkhLen);

    return result;
}

static char *
avalancheAddressAsStringC (BRAvalancheAddress address) {
    assert (AVALANCHE_CHAIN_TYPE_C == address.type);
    return hexEncodeCreate (NULL, address.u.c.bytes, AVALANCHE_ADDRESS_BYTES_C);
}

extern char *
avalancheAddressAsString (BRAvalancheAddress address) {
    if (avalancheAddressIsFeeAddress (address)) {
        return strdup ("__fee__");
    } else if (avalancheAddressIsUnknownAddress (address)) {
        return strdup ("unknown");
    } else {
        switch (address.type) {
            case AVALANCHE_CHAIN_TYPE_X:
                return avalancheAddressAsStringX (address);
            case AVALANCHE_CHAIN_TYPE_C:
                return avalancheAddressAsStringC (address);
            case AVALANCHE_CHAIN_TYPE_P:
                assert (false);
                return NULL;
        }
    }
}

// MARK: - Create From Key

static BRAvalancheAddress
avalancheAddressCreateFromKeyX (const uint8_t * pubKey, size_t pubKeyLen) {
    assert (33 == pubKeyLen);

    BRAvalancheAddress address = { AVALANCHE_CHAIN_TYPE_X };

    // Take the SHA256 of the `pubKey`
    UInt256 pubKeySha256;
    BRSHA256 (pubKeySha256.u8, pubKey, pubKeyLen);

    // The X address is the 20 bytes of the RMD160
    assert (20 == AVALANCHE_ADDRESS_BYTES_X);
    BRRMD160 (address.u.x.bytes, pubKeySha256.u8, sizeof (UInt256));

    return address;
}

static BRAvalancheAddress
avalancheAddressCreateFromKeyC (const uint8_t * pubKey, size_t pubKeyLen) {
    assert (65 == pubKeyLen);
    assert (32 >= AVALANCHE_ADDRESS_BYTES_C);

    BRAvalancheAddress address = { AVALANCHE_CHAIN_TYPE_C };

    UInt256 hash;
    BRKeccak256(hash.u8, &pubKey[1], pubKeyLen - 1);

    memcpy (address.u.c.bytes, &hash.u8[32 - AVALANCHE_ADDRESS_BYTES_C], AVALANCHE_ADDRESS_BYTES_C);

    return address;
}

extern BRAvalancheAddress
avalancheAddressCreateFromKey (const uint8_t * pubKey, size_t pubKeyLen, BRAvalancheChainType type) {
    switch (type) {
        case AVALANCHE_CHAIN_TYPE_X:
            return avalancheAddressCreateFromKeyX (pubKey, pubKeyLen);

        case AVALANCHE_CHAIN_TYPE_C:
            return avalancheAddressCreateFromKeyC (pubKey, pubKeyLen);

        case AVALANCHE_CHAIN_TYPE_P:
            assert (false);
            return ((BRAvalancheAddress) { (BRAvalancheChainType) -1 });
    }
}

// MARK: - Create From Bytes

#if 0
static BRAvalancheAddress
avalancheAddressCreateFromBytes (uint8_t * bytes, size_t length) {
    assert(bytes);
    assert(length == AVALANCHE_ADDRESS_BYTES);

    ASSERT_UNIMPLEMENTED;
    BRAvalancheAddress address = calloc(1, sizeof(struct BRAvalancheAddressRecord));
    memcpy(address->bytes, bytes, length);
    return address;
}
#endif

// MARK: - String to Address

static BRAvalancheAddress
avalancheAddressStringToAddressX (const char *input) {
    BRAvalancheAddress address = { AVALANCHE_CHAIN_TYPE_X };

    size_t addressBytesCount = AVALANCHE_ADDRESS_BYTES_X;
    avax_addr_bech32_decode(address.u.x.bytes, &addressBytesCount, "avax", input);
    
    return address;
}

static BRAvalancheAddress
avalancheAddressStringToAddressC (const char *input) {
    assert (40 == strlen(input));

    BRAvalancheAddress address = { AVALANCHE_CHAIN_TYPE_C };
    hexDecode (address.u.c.bytes, AVALANCHE_ADDRESS_BYTES_C, input, strlen(input));
    return address;
}


static BRAvalancheAddress
avalancheAddressStringToAddress(const char *input, BRAvalancheChainType type) {
    switch (type) {
        case AVALANCHE_CHAIN_TYPE_X:
            return avalancheAddressStringToAddressX (input);

        case AVALANCHE_CHAIN_TYPE_C:
            return avalancheAddressStringToAddressC (input);

        case AVALANCHE_CHAIN_TYPE_P:
            assert (false);
            return ((BRAvalancheAddress) { (BRAvalancheChainType) -1 });
    }
}

extern BRAvalancheAddress
avalancheAddressCreateFromString(const char * addressString, bool strict, BRAvalancheChainType type) {
    
    if (addressString == NULL || strlen(addressString) == 0) {
        assert (!strict);
        return avalancheAddressCreateUnknownAddress (type);
    }
    else if (strict) {
        return avalancheAddressStringToAddress (addressString, type);
    }
    else if (strcmp(addressString, "unknown") == 0) {
        return avalancheAddressCreateUnknownAddress (type);
    }
    else if (strcmp(addressString, "__fee__") == 0) {
        return avalancheAddressCreateFeeAddress (type);
    }
    else {
        return avalancheAddressStringToAddress (addressString, type);
    }
}

extern bool
avalancheAddressEqual (BRAvalancheAddress a1, BRAvalancheAddress a2) {
    return (a1.type == a2.type &&
            (AVALANCHE_CHAIN_TYPE_X == a1.type
             ? 0 == memcmp (a1.u.x.bytes, a2.u.x.bytes, AVALANCHE_ADDRESS_BYTES_X)
             : 0 == memcmp (a1.u.c.bytes, a2.u.c.bytes, AVALANCHE_ADDRESS_BYTES_C)));
}

extern size_t
avalancheAddressHashValue (BRAvalancheAddress address) {
    return (AVALANCHE_CHAIN_TYPE_X == address.type
            ? *((size_t*) address.u.x.bytes)
            : *((size_t*) address.u.c.bytes));
}

#if 0
extern BRAvalancheAddress
avalancheAddressClone (BRAvalancheAddress address) {
    if (address) {
        BRAvalancheAddress clone = calloc(1, sizeof(struct BRAvalancheAddressRecord));
        *clone = *address;
        return clone;
    }
    return NULL;
}

extern size_t
avalancheAddressGetRawSize (BRAvalancheAddress address) {
    return AVALANCHE_ADDRESS_BYTES;
}

extern void avalancheAddressGetRawBytes (BRAvalancheAddress address, uint8_t *buffer, size_t bufferSize) {
    assert(buffer);
    assert(bufferSize >= AVALANCHE_ADDRESS_BYTES);
    memcpy(buffer, address->bytes, AVALANCHE_ADDRESS_BYTES);
}
#endif



// MARK: - Support



//TODO: move everything below here to vendored utils
//https://github.com/ava-labs/ledger-app-avalanche/blob/a340702ec427b15e7e4ed31c47cc5e2fb170ebdf/src/bech32encode.c#L28
//thanks to ledger for your hard work xo
static uint32_t
bech32_polymod_step(uint32_t pre) {
    uint8_t b = pre >> 25;
    return ((pre & 0x1FFFFFF) << 5) ^ (-((b >> 0) & 1) & 0x3b6a57b2UL) ^ (-((b >> 1) & 1) & 0x26508e6dUL) ^
    (-((b >> 2) & 1) & 0x1ea119faUL) ^ (-((b >> 3) & 1) & 0x3d4233ddUL) ^ (-((b >> 4) & 1) & 0x2a1462b3UL);
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
    if (output == NULL || out_len_max <= final_out_len)
        return false;
    if (hrp == NULL || hrp_len <= 0)
        return false;
    if (data == NULL || data_len <= 0)
        return false;
    for (size_t i = 0; i < hrp_len; ++i) {
        char ch = hrp[i];
        if (!(33 <= ch && ch <= 126))
            return false;
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

//https://github.com/iotaledger/iota.c/blob/6b94ad03c915139d8292513d7849cb5e1a20df2f/tests/core/test_utils_bech32.c
extern int
avax_bech32_decode(char *hrp, uint8_t *data, size_t *data_len, const char *input) {
    uint32_t chk = 1;
    size_t i;
    size_t input_len = strlen(input);
    size_t hrp_len;
    int have_lower = 0, have_upper = 0;
    if (input_len < 8 || input_len > 108) {
        return 0;
    }
    *data_len = 0;
    while (*data_len < input_len && input[(input_len - 1) - *data_len] != '1') {
        ++(*data_len);
    }
    if (1 + *data_len >= input_len || *data_len < 6) {
        return 0;
    }
    hrp_len = input_len - (1 + *data_len);
    *(data_len) -= 6;
    for (i = 0; i < hrp_len; ++i) {
        int ch = input[i];
        if (ch < 33 || ch > 126) {
            return 0;
        }
        if (ch >= 'a' && ch <= 'z') {
            have_lower = 1;
        } else if (ch >= 'A' && ch <= 'Z') {
            have_upper = 1;
            ch = (ch - 'A') + 'a';
        }
        hrp[i] = ch;
        chk = bech32_polymod_step(chk) ^ (ch >> 5);
    }
    hrp[i] = 0;
    chk = bech32_polymod_step(chk);
    for (i = 0; i < hrp_len; ++i) {
        chk = bech32_polymod_step(chk) ^ (input[i] & 0x1f);
    }
    ++i;
    while (i < input_len) {
        int v = (input[i] & 0x80) ? -1 : charset_rev[(int)input[i]];
        if (input[i] >= 'a' && input[i] <= 'z') have_lower = 1;
        if (input[i] >= 'A' && input[i] <= 'Z') have_upper = 1;
        if (v == -1) {
            return 0;
        }
        chk = bech32_polymod_step(chk) ^ v;
        if (i + 6 < input_len) {
            printf("data[%i - (1 + %i)] = %i\r\n",i,hrp_len,v);
            data[i - (1 + hrp_len)] = v;
        }
        ++i;
    }
    if (have_lower && have_upper) {
        return 0;
    }
    return chk == 1;
}

static int avax_convert_bits(uint8_t *out, size_t *outlen, int outbits, const uint8_t *in, size_t inlen, int inbits,
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
        return 0;
    }
    return 1;
}


extern int
avax_addr_bech32_decode(uint8_t *addr_data, size_t *addr_len, const char *hrp, const char *addr_str) {
    uint8_t data[AVALANCHE_ADDRESS_BYTES_X - (strlen(hrp)+1)]; // total - (hrp + 1) =
    char hrp_actual[strlen(hrp)+1];//we only expect the hrp = avax\0
    size_t data_len = 0;

    if (!avax_bech32_decode(hrp_actual, data, &data_len, addr_str)) {
        return 0;
    }

    if (data_len == 0 || data_len > 64) {
        return 0;
    }

    //  if (strncmp(hrp, hrp_actual, 84) != 0) {
    //    return 0;
    //  }

    *addr_len = 0;
    if (!avax_convert_bits(addr_data, addr_len, 8, data, data_len, 5, 0)) {
        return 0;
    }

    return 1;
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
