//
//  BRAvalancheUtil.c
//  
//
//  Created by Amit on 17/06/2021.
//

#include "BRAvalancheUtil.h"
#include <arpa/inet.h>
#include "support/BRInt.h"

static WKData pack_byte(uint8_t byte){
    return wkDataCopy(byte, 1);
}
static WKData pack_int16(uint16_t int16){
    uint8_t bytes[2];
    bytes[0] = (int16 >> 8) & 0xFF;
    bytes[1] = int16 & 0xFF;
    return wkDataCopy(bytes,sizeof(bytes));
}

static WKData pack_int32(uint32_t int32){
    uint8_t bytes[4];
    bytes[0] = (int32 >> 24) & 0xFF;
    bytes[1] = (int32 >> 16) & 0xFF;
    bytes[2] = (int32 >> 8) & 0xFF;
    bytes[3] = int32 & 0xFF;
    return wkDataCopy(bytes,sizeof(bytes));
}

static WKData pack_int64(uint64_t int32){
    uint8_t bytes[8];
    bytes[0] = (int32 >> 56) & 0xFF;
    bytes[1] = (int32 >> 48) & 0xFF;
    bytes[2] = (int32 >> 40) & 0xFF;
    bytes[3] = (int32 >> 32) & 0xFF;
    bytes[4] = (int32 >> 24) & 0xFF;
    bytes[5] = (int32 >> 16) & 0xFF;
    bytes[6] = (int32 >> 8) & 0xFF;
    bytes[7] = int32 & 0xFF;
    return wkDataCopy(bytes,sizeof(bytes));
}

static WKData pack_ipv4(char * ipaddr){
    uint8_t bytes[18];
    //set first 12 bytes to zero
    memset(bytes,0, 12);
    struct sockaddr_in sa;
    char str[INET_ADDRSTRLEN];

    // store this IP address in sa:
    inet_pton(AF_INET, ipaddr, &(sa.sin_addr));
    memcpy((void*) &sa.sin_addr, &bytes[12], sizeof(sa.sin_addr));
    memcpy((void*)&sa.sin_port, &bytes[17], sizeof(sa.sin_port));
    return wkDataCopy(bytes, 18);
}

static WKData pack_string(char * str, size_t strlen){
    return wkDataCopy((uint8_t *) str,strlen );
}

static WKData BKAvaxEncodeLength(UInt256 len){
    return wkDataCopy(len.u8, 16);
}
static WKData BKAvaxStringToBuffer(char * str){
    
    return wkDataCopy((uint8_t*)str, 10);
}



//RECALL for base58 see if this is still a thing:https://github.com/ava-labs/avalanche-docs-deprecated/issues/50




//https://github.com/ava-labs/avalanchejs/blob/629a4b7f97b66bdf7f20d1f147484274e514ab75/src/utils/bintools.ts

//TODO:codec must implement following set of primitives:
/*
 https://github.com/ava-labs/avalanchejs/blob/1a2866a6aa85de0eb27a8f212658a5a64e0263ad/src/utils/serialization.ts#L150
 if(type === "BN") {
     let str:string = (v as BN).toString("hex");
     if(args.length == 1 && typeof args[0] === "number"){
        return Buffer.from(str.padStart(args[0] * 2, '0'), 'hex');
     }
     return Buffer.from(str, 'hex');
 } else if(type === "Buffer") {
     return v;
 } else if(type === "bech32") {
     return this.bintools.stringToAddress(v);
 } else if(type === "nodeID") {
     return NodeIDStringToBuffer(v);
 } else if(type === "privateKey") {
     return privateKeyStringToBuffer(v);
 } else if(type === "cb58") {
     return this.bintools.cb58Decode(v);
 } else if(type === "base58") {
     return this.bintools.b58ToBuffer(v);
 } else if(type === "base64") {
     return Buffer.from(v as string, "base64");
 } else if(type === "hex") {
     if((v as string).startsWith("0x")){
         v = (v as string).slice(2);
     }
     return Buffer.from(v as string, "hex");
 } else if(type === "decimalString") {
     let str:string = new BN(v as string, 10).toString("hex");
     if(args.length == 1 && typeof args[0] === "number"){
         return Buffer.from(str.padStart(args[0] * 2, '0'), 'hex');
     }
     return Buffer.from(str, 'hex');
 } else if(type === "number") {
     let str:string = new BN(v, 10).toString("hex");
     if(args.length == 1 && typeof args[0] === "number"){
         return Buffer.from(str.padStart(args[0] * 2, '0'), 'hex');
     }
     return Buffer.from(str, 'hex');
 } else if(type === "utf8") {
     if(args.length == 1 && typeof args[0] === "number"){
         let b:Buffer = Buffer.alloc(args[0]);
         b.write(v)
         return b;
     }
     return Buffer.from(v, 'utf8');
 }
 */







//TODO: move everything below here to vendored utils
//https://github.com/ava-labs/ledger-app-avalanche/blob/a340702ec427b15e7e4ed31c47cc5e2fb170ebdf/src/bech32encode.c#L28
//thanks to ledger for your hard work xo
uint32_t bech32_polymod_step(uint32_t pre) {
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


int avax_bech32_decode(char *hrp, uint8_t *data, size_t *data_len, const char *input) {
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
      data[i - (1 + hrp_len)] = v;
    }
    ++i;
  }
  if (have_lower && have_upper) {
    return 0;
  }
  return chk == 1;
}
