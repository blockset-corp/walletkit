//
//  File.c
//  
//
//  Created by Carl Cherry on 2021-05-19.
//

#include "BRStellarAddress.h"
#include "BRStellarPrivateStructs.h"
#include "utils/base32.h"
#include "utils/crc16.h"

#define STELLAR_ADDRESS_STRING_SIZE   (56)

static uint8_t unknownAddressBytes[20] = {
    0x42, 0x52, 0x44, //BRD
    0x5F, 0x5F, // __
    'u', 'n', 'k', 'n', 'o', 'w', 'n', // unknown
    0x5F, 0x5F, // __
    0x42, 0x52, 0x44, // BRD
    0x00, 0x00, 0x00 // padding
};

static uint8_t feeAddressBytes[20] = {
    0x42, 0x52, 0x44, //BRD
    0x5F, 0x5F, // __
    'f', 'e', 'e', // fee
    0x5F, 0x5F, // __
    0x42, 0x52, 0x44, // BRD
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // padding
};

extern void
stellarAddressFree(BRStellarAddress address)
{
    if (address) free(address);
}

extern BRStellarAddress
stellarAddressCreate(BRKey * publicKey)
{
    BRStellarAddress address = calloc(1, sizeof(struct BRStellarAddressRecord));
    memcpy(address->bytes, publicKey->pubKey, STELLAR_ADDRESS_BYTES);
    return address;
}

extern BRStellarAddress stellarAddressClone (BRStellarAddress address)
{
    if (address) {
        BRStellarAddress clone = calloc(1, sizeof(struct BRStellarAddressRecord));
        //*clone = *address;
        memcpy(clone->bytes, address->bytes, sizeof(address->bytes));
        return clone;
    }
    return NULL;
}

BRStellarAddress stellarAddressCreateUnknownAddress()
{
    BRStellarAddress address = calloc(1, sizeof(struct BRStellarAddressRecord));
    memcpy(address->bytes, unknownAddressBytes, sizeof(unknownAddressBytes));
    return address;
}

BRStellarAddress stellarAddressCreateFeeAddress()
{
    BRStellarAddress address = calloc(1, sizeof(struct BRStellarAddressRecord));
    memcpy(address->bytes, feeAddressBytes, sizeof(feeAddressBytes));
    return address;
}

BRStellarAddress stellarAddressStringToAddress(const char* input)
{
    BRStellarAddress address = calloc(1, sizeof(struct BRStellarAddressRecord));

    uint8_t rawBytes[36];
    base32_decode((const unsigned char*)input, rawBytes);
    memcpy(address->bytes, &rawBytes[1], 32);
    return address;
}

extern char *
stellarAddressAsString(BRStellarAddress address)
{
    assert(address);

    // The public key is 32 bytes, we need 3 additional bytes
    uint8_t rawBytes[35];
    rawBytes[0] = 0x30; // account prefix
    memcpy(&rawBytes[1], address->bytes, 32); // public key bytes

    // Create a CRC16-XMODEM checksum and append to bytes
    uint16_t checksum = crc16((const char*)rawBytes, 33);
    rawBytes[33] = (checksum & 0x00FF);
    rawBytes[34] = (checksum & 0xFF00) >> 8;

    // Base32 the result
    char * addressAsString = (char*)calloc(1, STELLAR_ADDRESS_STRING_SIZE + 1);
    base32_encode(rawBytes, 35, (unsigned char*)addressAsString);
    return addressAsString;
}

extern BRStellarAddress
stellarAddressCreateFromString(const char * stellarAddressString, bool strict)
{
    // Handle an 'invalid' string argument.
    if (stellarAddressString == NULL || strlen(stellarAddressString) == 0) {
        return (strict
                ? NULL
                : stellarAddressCreateUnknownAddress ());
    }

    //  If strict, only accept 'r...' addresses
    else if (strict) {
        return stellarAddressStringToAddress (stellarAddressString);
    }

    // Handle an 'unknown' address
    else if (strcmp(stellarAddressString, "unknown") == 0) {
        return stellarAddressCreateUnknownAddress ();
    }

    // Handle a '__fee__' address
    else if (strcmp(stellarAddressString, "__fee__") == 0) {
        return stellarAddressCreateFeeAddress ();
    }

    // Handle an 'r...' address (in a non-strict mode).
    else {
        // Work backwards from this ripple address (string) to what is
        // known as the acount ID (20 bytes)
        return stellarAddressStringToAddress (stellarAddressString);
    }
}

extern int // 1 if equal
stellarAddressEqual (BRStellarAddress a1, BRStellarAddress a2) {
    return 0 == memcmp (a1->bytes, a2->bytes, STELLAR_ADDRESS_BYTES);
}

extern size_t
stellarAddressHashValue (BRStellarAddress address) {
    return *((size_t*) address->bytes);
}

extern size_t
stellarAddressGetRawSize (BRStellarAddress address)
{
    return STELLAR_ADDRESS_BYTES;
}

extern void stellarAddressGetRawBytes (BRStellarAddress address, uint8_t *buffer, size_t bufferSize)
{
    assert(buffer);
    assert(bufferSize >= STELLAR_ADDRESS_BYTES);
    memcpy(buffer, address->bytes, STELLAR_ADDRESS_BYTES);
}

