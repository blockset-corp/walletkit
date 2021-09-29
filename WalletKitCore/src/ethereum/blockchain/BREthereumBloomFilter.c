//
//  BREthereumBloomFilter.c
//  WalletKitCore
//
//  Created by Ed Gamble on 5/10/18.
//  Copyright © 2018-2019 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include <assert.h>
#include <string.h>
#include "BREthereumBloomFilter.h"

/* Forward Declarations */
static void
ethBloomFilterExtractLocation (unsigned int index, unsigned int *byteIndex, unsigned int *bitIndex);

static void
ethBloomFilterSetBit (BREthereumBloomFilter *filter, unsigned int index);

static void
ethBloomFilterClrBit (BREthereumBloomFilter *filter, unsigned int index);

static unsigned int
ethBloomFilterCreateIndex (uint8_t highByte, uint8_t lowByte);

//
// An Empty BloomFilter
//
static BREthereumBloomFilter empty;

extern BREthereumBloomFilter
ethBloomFilterCreateEmpty (void) {
    return empty;
}

/**
 * 'Designated Contructor' for BloomFilter
 */
extern BREthereumBloomFilter
ethBloomFilterCreateHash (const BREthereumHash hash) {
    BREthereumBloomFilter filter = empty;
    ethBloomFilterSetBit(&filter, ethBloomFilterCreateIndex(hash.bytes[0], hash.bytes[1]));
    ethBloomFilterSetBit(&filter, ethBloomFilterCreateIndex(hash.bytes[2], hash.bytes[3]));
    ethBloomFilterSetBit(&filter, ethBloomFilterCreateIndex(hash.bytes[4], hash.bytes[5]));
    return filter;
}

extern BREthereumBloomFilter
ethBloomFilterCreateData (const BRRlpData data) {
    return ethBloomFilterCreateHash(ethHashCreateFromData(data));
}

extern BREthereumBloomFilter
ethBloomFilterCreateAddress (const BREthereumAddress address) {
    BRRlpData data;
    data.bytes = (uint8_t *)  address.bytes;
    data.bytesCount = sizeof (address.bytes);
    return ethBloomFilterCreateHash(ethHashCreateFromData(data));
}

extern BREthereumBloomFilter
ethBloomFilterCreateString (const char *string) {
    BREthereumBloomFilter filter;
    if (0 == strncmp ("0x", string, 2)) string = &string[2];
    hexDecode (filter.bytes, sizeof (filter.bytes), string, strlen(string));
    return filter;
}

extern BREthereumBloomFilter
ethBloomFilterOr (const BREthereumBloomFilter filter1, const BREthereumBloomFilter filter2) {
    BREthereumBloomFilter result = empty;
    for (int i = 0; i < ETHEREUM_BLOOM_FILTER_BYTES; i++)
        result.bytes[i] = filter1.bytes[i] | filter2.bytes[i];
    return result;
}

extern void
ethBloomFilterOrInPlace (BREthereumBloomFilter filter1, const BREthereumBloomFilter filter2) {
    for (int i = 0; i < ETHEREUM_BLOOM_FILTER_BYTES; i++)
        filter1.bytes[i] |= filter2.bytes[i];
}

extern BREthereumBoolean
ethBloomFilterEqual (const BREthereumBloomFilter filter1, const BREthereumBloomFilter filter2) {
    return (0 == memcmp (filter1.bytes, filter2.bytes, ETHEREUM_BLOOM_FILTER_BYTES)
            ? ETHEREUM_BOOLEAN_TRUE
            : ETHEREUM_BOOLEAN_FALSE);
}

extern BREthereumBoolean
ethBloomFilterMatch (const BREthereumBloomFilter filter, const BREthereumBloomFilter other) {
    return ethBloomFilterEqual(filter, ethBloomFilterOr(filter, other));
}

//
// RLP Encode / Decoe
//
extern BRRlpItem
ethBloomFilterRlpEncode(BREthereumBloomFilter filter, BRRlpCoder coder) {
    return rlpEncodeBytes(coder, filter.bytes, 256);
}

extern BREthereumBloomFilter
ethBloomFilterRlpDecode (BRRlpItem item, BRRlpCoder coder) {
    BREthereumBloomFilter filter;

    BRRlpData data = rlpDecodeBytes(coder, item);
    assert (256 == data.bytesCount);

    memcpy (filter.bytes, data.bytes, 256);
    rlpDataRelease(data);
    
    return filter;
}

//
// As String
//
extern char *
ethBloomFilterAsString (BREthereumBloomFilter filter) {
    return hexEncodeCreate(NULL, filter.bytes, sizeof (filter.bytes));
}

//
// Support
//

/**
 * Extract the byte and bit position of `index` within ETHEREUM_BLOOM_FILTER_BITS.  `index`
 * is [0, ETHEREUM_BLOOM_FILTER_BITS).
 *
 */
static void
ethBloomFilterExtractLocation (unsigned int index, unsigned int *byteIndex, unsigned int *bitIndex) {
    assert (index < ETHEREUM_BLOOM_FILTER_BITS);
    assert (NULL != byteIndex && NULL != bitIndex);

    *byteIndex = (ETHEREUM_BLOOM_FILTER_BITS - 1 - index ) / 8;
    *bitIndex  = index % 8;
}

static void
ethBloomFilterSetBit (BREthereumBloomFilter *filter, unsigned int index) {
    unsigned int byteIndex;
    unsigned int bitIndex;
    ethBloomFilterExtractLocation(index, &byteIndex, &bitIndex);
    filter->bytes[byteIndex] |= (1 << bitIndex);
}

#pragma clang diagnostic push
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-function"
static void
ethBloomFilterClrBit (BREthereumBloomFilter *filter, unsigned int index) {
    unsigned int byteIndex;
    unsigned int bitIndex;
    ethBloomFilterExtractLocation(index, &byteIndex, &bitIndex);
    filter->bytes[byteIndex] &= ~(1 << bitIndex);
}
#pragma clang diagnostic pop
#pragma GCC diagnostic pop

#define ETHERUM_BLOOM_FILTER_BITS_MASK  (ETHEREUM_BLOOM_FILTER_BITS - 1)

static unsigned int
ethBloomFilterCreateIndex (uint8_t highByte, uint8_t lowByte) {
    return ((highByte << 8) | lowByte) & ETHERUM_BLOOM_FILTER_BITS_MASK;
}
