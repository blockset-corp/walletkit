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
#include "BRAvalancheUtil.h"




//generate x-chain address from seed
// algorithm: Bech32Encode("avax",Base32Encode(ripemd160(sha256(publicKey))))
extern BRAvalancheXAddress
avalancheAddressCreateFromKey (const uint8_t * pubKey, size_t pubKeyLen) {
    BRAvalancheXAddress address;// = calloc(1, sizeof(struct BRAvalancheXAddress));
    uint8_t pkh[32];
    uint8_t rmd160[20];
    static const char *hrp = "avax";
    char addr[44];
    const uint8_t *hrpLen = 4;
    const uint8_t  *pkhLen = 32;
    size_t outputLen = 44 ;// must be > pkhLen + hrpSiz + 7
    
    BRSHA256(pkh, pubKey, pubKeyLen);
    BRRMD160(rmd160, pkh, 32);
    avax_base32_encode(pkh, &pkhLen, rmd160, sizeof(rmd160));
    avax_bech32_encode(&addr[0], &outputLen, hrp, hrpLen, pkh, sizeof(pkh));
    
    memcpy(address.bytes, addr, sizeof(address.bytes));
    
    uint8_t recovered[20];
    size_t rec_len;
    avax_addr_bech32_decode(recovered, &rec_len, "avax", (char * )&addr[0] );
    printf("expected:");
    for(int i=0; i < 20; i++){
        printf("%02x", rmd160[i]);
    }
    printf(" actual:");
    for(int i=0; i < rec_len; i++){
        printf("%02x", recovered[i]);
    }
    printf("\r\n");
   
   
    return address;
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


int avax_addr_bech32_decode(uint8_t *addr_data, size_t *addr_len, const char *hrp, const char *addr_str) {
    uint8_t data[38];
    char hrp_actual[8];//we only expect the hrp = avax\0
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


